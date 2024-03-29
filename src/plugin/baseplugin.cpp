#define BUILD_DUMMY_PLUGIN
#include "baseplugin.h"
#include "synth/synthcontext.h"
#include <iostream>
#include <sstream>
#include <vector>

std::string ClefPluginBase::libclefCopyright()
{
  return "\n\n"
    "Derived from libclef, copyright (C) 2020-2024 Adam Higerd\n"
    "under the terms of the MIT license.";
}

TagMap TagsM3UMixin::readTags(ClefContext* clef, const std::string& filename)
{
  std::string m3uPath = TagsM3U::relativeTo(filename);
  try {
    auto m3uStream(clef->openFile(m3uPath));
    if (*m3uStream) {
      TagsM3U m3u(*m3uStream);
      TagMap tagMap = m3u.allTags(filename);
      if (!tagMap.at("title").empty()) {
        return tagMap;
      }
    }
  } catch (...) {
    // no entry for filename in !tags.m3u or title tag does not exist
  }
  return TagMap();
}

std::vector<std::string> TagsM3UMixin::getSubsongs(ClefContext* clef, const std::string& filename, std::istream& file)
{
  std::string m3uPath = TagsM3U::relativeTo(filename);
  int slashPos = filename.find_last_of(PATH_CHARS);
  int qPos = filename.find('?');
  std::string baseName = filename.substr(slashPos + 1, qPos - slashPos - 1);
  std::vector<std::string> result;
  try {
    auto m3uStream(clef->openFile(m3uPath));
    if (*m3uStream) {
      TagsM3U m3u(*m3uStream);
      std::vector<int> trackIds = m3u.findTracksByPrefix(baseName);
      if (trackIds.size() < 2) {
        return result;
      }
      int slashPos = m3uPath.find_last_of(PATH_CHARS);
      std::string relative;
      if (slashPos != std::string::npos) {
        relative = m3uPath.substr(0, slashPos + 1);
      }
      for (int trackId : trackIds) {
        result.push_back(relative + m3u.trackName(trackId));
      }
    }
  } catch (...) {
    // no entry for filename in !tags.m3u or title tag does not exist
    std::cerr << "error" << std::endl;
  }
  return result;
}

ClefPluginBase::ClefPluginBase(ClefContext* clef) : clef(clef), ctx(nullptr)
{
  // initializers only
}

std::string toLower(const std::string& str)
{
  std::string result(str);
  for (char& ch : result) {
    if (ch >= 'A' && ch <= 'Z') {
      ch += ('a' - 'A');
    }
  }
  return str;
}

bool ClefPluginBase::matchExtension(const std::string& filename) const
{
  int dotPos = filename.rfind('.');
  if (dotPos == std::string::npos) {
    return false;
  }
  std::string ext = toLower(filename.substr(dotPos + 1));
  for (const auto& iter : extensions()) {
    if (ext == toLower(iter.first)) {
      return true;
    }
  }
  return false;
}

TagMap ClefPluginBase::getTags(const std::string& filename, std::istream& file) const
{
  try {
    auto pos = file.tellg();
    TagMap tagMap = getTagsBase(filename, file);
    try {
      if (!tagMap.count("length_seconds_fp")) {
        std::ostringstream ss;
        file.clear();
        file.seekg(pos);
        double len = length(filename, file);
        if (len > 0) {
          ss << len;
          tagMap["length_seconds_fp"] = ss.str();
        }
      }
    } catch (...) {
      // ignore error getting length instead of breaking all of the tags
    }
    if (!tagMap.count("display_title") && tagMap.count("title")) {
      std::string displayTitle = tagMap["title"];
      std::string artist = tagMap.count("artist") ? tagMap.at("artist") : std::string();
      if (!artist.empty()) {
        displayTitle = artist + " - " + displayTitle;
      }
      tagMap["display_title"] = displayTitle;
    }
    return tagMap;
  } catch (...) {
    return TagMap();
  }
}

int ClefPluginBase::fillBuffer(uint8_t* buffer, int len)
{
  if (!ctx) {
    return 0;
  }
  return ctx->fillBuffer(buffer, len);
}

int ClefPluginBase::channels() const
{
  if (!ctx) {
    return 0;
  }
  return ctx->outputChannels;
}

int ClefPluginBase::sampleRate() const
{
  if (!ctx) {
    return 0;
  }
  return ctx->sampleRate;
}

bool ClefPluginBase::play(const std::string& filename, std::istream& file)
{
  try {
    file.seekg(0);
    ctx = prepare(filename, file);
    if (!ctx || ctx->channels.size() == 0) {
      unload();
      return false;
    }
    return true;
  } catch (std::exception& e) {
    std::cerr << "Exception in play(): " << e.what() << std::endl;
    return false;
  } catch (...) {
    std::cerr << "Unknown exception in play()" << std::endl;
    return false;
  }
}

double ClefPluginBase::currentTime() const
{
  if (ctx) {
    return ctx->currentTime();
  }
  return 0;
}

void ClefPluginBase::seek(double time)
{
  if (ctx) {
    ctx->seek(time);
  }
}

void ClefPluginBase::unload()
{
  release();
  ctx = nullptr;
}

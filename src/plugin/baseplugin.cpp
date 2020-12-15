#define BUILD_DUMMY_PLUGIN
#include "baseplugin.h"
#include "synth/synthcontext.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

std::string S2WPluginBase::seq2wavCopyright()
{
  return "\n\n"
    "Derived from seq2wav copyright (C) 2020 Adam Higerd\n"
    "under the terms of the MIT license.";
}

TagMap TagsM3UMixin::readTags(const OpenFn& openFile, const std::string& filename)
{
  std::string m3uPath = TagsM3U::relativeTo(filename);
  try {
    auto m3uStream(openFile(m3uPath));
    TagsM3U m3u(*m3uStream);
    TagMap tagMap = m3u.allTags(filename);
    if (!tagMap.at("title").empty()) {
      return tagMap;
    }
  } catch (...) {
    // no entry for filename in !tags.m3u or title tag does not exist
  }
  return TagMap();
}

S2WPluginBase::S2WPluginBase() : ctx(nullptr)
{
  openFile = [](const std::string& filename) {
    return std::unique_ptr<std::istream>(new std::ifstream(filename.c_str(), std::ios::in | std::ios::binary));
  };
}

bool S2WPluginBase::matchExtension(const std::string& filename) const
{
  int dotPos = filename.rfind('.');
  if (dotPos == std::string::npos) {
    return false;
  }
  std::string ext = filename.substr(dotPos + 1);
  for (const auto& iter : extensions()) {
    if (ext == iter.first) {
      return true;
    }
  }
  return false;
}

TagMap S2WPluginBase::getTags(const std::string& filename, std::istream& file) const
{
  try {
    auto pos = file.tellg();
    TagMap tagMap = getTagsBase(filename, file);
    if (!tagMap.count("length_seconds_fp")) {
      std::ostringstream ss;
      file.seekg(pos);
      double len = length(filename, file);
      if (len > 0) {
        ss << len;
        tagMap["length_seconds_fp"] = ss.str();
      }
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

void S2WPluginBase::setOpener(const OpenFn& opener)
{
  openFile = opener;
}

int S2WPluginBase::fillBuffer(uint8_t* buffer, int len)
{
  if (!ctx) {
    return 0;
  }
  return ctx->fillBuffer(buffer, len);
}

int S2WPluginBase::channels() const
{
  if (!ctx) {
    return 0;
  }
  return ctx->outputChannels;
}

int S2WPluginBase::sampleRate() const
{
  if (!ctx) {
    return 0;
  }
  return ctx->sampleRate;
}

bool S2WPluginBase::play(const std::string& filename, std::istream& file)
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

double S2WPluginBase::currentTime() const
{
  if (ctx) {
    return ctx->currentTime();
  }
  return 0;
}

void S2WPluginBase::seek(double time)
{
  if (ctx) {
    ctx->seek(time);
  }
}

void S2WPluginBase::unload()
{
  release();
  ctx = nullptr;
}

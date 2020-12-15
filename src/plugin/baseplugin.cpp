#define BUILD_DUMMY_PLUGIN
#include "baseplugin.h"
#include "synth/synthcontext.h"
#include <iostream>
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
  // initializers only
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
  return tagMap;
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

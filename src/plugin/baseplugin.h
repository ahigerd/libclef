#ifndef CLEF_BASEPLUGIN_H
#define CLEF_BASEPLUGIN_H

#include <string>
#include <utility>
#include <cstdint>
#include <memory>
#include <functional>
#include <type_traits>
#include "utility.h"
#include "tagmap.h"
#include "clefcontext.h"
#include "synth/synthcontext.h"
class ClefPluginInfo;

using ConstPairList = const std::vector<std::pair<std::string, std::string>>;

#ifdef BUILD_CLAP
# define CLEF_PLUGIN_BANK_EXT static ConstPairList bankExtensions;
#else
# define CLEF_PLUGIN_BANK_EXT
#endif

#define CLEF_PLUGIN_STATIC_FIELDS \
  static const std::string version, pluginName, pluginShortName, about, author, url; \
  static ConstPairList extensions; CLEF_PLUGIN_BANK_EXT

struct TagsM3UMixin {
  static TagMap readTags(ClefContext* clef, const std::string& filename);
  static inline TagMap readTags(ClefContext* clef, const std::string& filename, std::istream& /* unused */) { return readTags(clef, filename); }
  static std::vector<std::string> getSubsongs(ClefContext* clef, const std::string& filename, std::istream& file);
};

#ifdef BUILD_DUMMY_PLUGIN
struct DummyPluginInfo : public TagsM3UMixin {
  CLEF_PLUGIN_STATIC_FIELDS
  static bool isPlayable(ClefContext* clef, const std::string& filename, std::istream& file, bool ignoreExtension = false) {
    return false;
  }
  static double length(ClefContext* clef, const std::string& filename, std::istream& file) {
    return 0;
  }
  static double sampleRate(ClefContext* clef, const std::string& filename, std::istream& file) {
    return 44100;
  }
  SynthContext* prepare(ClefContext* clef, const std::string& filename, std::istream& file) {
    // Implementations should retain appropriate pointers
    return nullptr;
  }
  void release() {
    // Implementations should release any retained pointers
  }
};

const std::string DummyPluginInfo::version = "0.0.1";
const std::string DummyPluginInfo::pluginName = "dummyClef";
ConstPairList DummyPluginInfo::extensions = { { "dummy", "Dummy files (*.dummy)" } };
const std::string DummyPluginInfo::about =
  "Dummy plugin copyright (C) 2020 Adam Higerd\n"
  "Distributed under the MIT license.";
#endif

class ClefPluginBase {
public:
  static std::string libclefCopyright();

  inline ClefContext* context() const { return clef; }

  bool matchExtension(const std::string& filename) const;
  TagMap getTags(const std::string& filename, std::istream& file) const;
  virtual std::vector<std::string> getSubsongs(const std::string& filename, std::istream& file) const = 0;

  int fillBuffer(uint8_t* buffer, int len);

  int channels() const;
  int sampleRate() const; // of playing track
  bool play(const std::string& filename, std::istream& file);
  double currentTime() const;
  void seek(double time);
  void unload();

  virtual const std::string& version() const = 0;
  virtual const std::string& pluginShortName() const = 0;
  virtual const std::string& pluginName() const = 0;
  virtual const ConstPairList& extensions() const = 0;
  virtual const std::string& about() const = 0;
  virtual bool isPlayable(const std::string& filename, std::istream& file, bool ignoreExtension = false) const = 0;
  virtual double length(const std::string& filename, std::istream& file) const = 0;
  virtual int sampleRate(const std::string& filename, std::istream& file) const = 0; // of unloaded track

  virtual SynthContext* prepare(const std::string& filename, std::istream& file) = 0;
  virtual void release() = 0;

protected:
  ClefPluginBase(ClefContext* clef);
  virtual TagMap getTagsBase(const std::string& filename, std::istream& file) const = 0;

  ClefContext* clef;
  SynthContext* ctx;
};

template <typename Info, typename U = int> struct GetSubsongs {
static std::vector<std::string> getSubsongsImpl(ClefContext* clef, const std::string& filename, std::istream& file) {
  (void)clef;
  (void)filename;
  (void)file;
  return std::vector<std::string>();
}
};

template <typename Info> struct GetSubsongs<Info, decltype((void) Info::getSubsongs, 0)> {
static std::vector<std::string> getSubsongsImpl(ClefContext* clef, const std::string& filename, std::istream& file) {
  file.seekg(0);
  return Info::getSubsongs(clef, filename, file);
}
};

template <typename PluginInfo>
class ClefPlugin : public ClefPluginBase, public PluginInfo {
public:
  using Info = PluginInfo;

  ClefPlugin(ClefContext* ctx) : ClefPluginBase(ctx) {}

  const std::string& version() const { return Info::version; }
  const std::string& pluginShortName() const { return Info::pluginShortName; }
  const std::string& pluginName() const { return Info::pluginName; }
  const ConstPairList& extensions() const { return Info::extensions; }
  const std::string& about() const {
    static std::string message = Info::about + libclefCopyright();
    return message;
  }
  virtual bool isPlayable(const std::string& filename, std::istream& file, bool ignoreExtension = false) const {
    try {
      if (!ignoreExtension && !matchExtension(filename)) {
        return false;
      }
      file.clear();
      file.seekg(0);
      return Info::isPlayable(clef, filename, file);
    } catch (...) {
      return false;
    }
  }
  double length(const std::string& filename, std::istream& file) const { file.seekg(0); return Info::length(clef, filename, file); }
  int sampleRate(const std::string& filename, std::istream& file) const { file.seekg(0); return Info::sampleRate(clef, filename, file); }
  inline int sampleRate() const { return ClefPluginBase::sampleRate(); }

  SynthContext* prepare(const std::string& filename, std::istream& file) { file.seekg(0); return Info::prepare(clef, filename, file); }
  void release() { Info::release(); }

  std::vector<std::string> getSubsongs(const std::string& filename, std::istream& file) const { return GetSubsongs<Info>::getSubsongsImpl(clef, filename, file); }

protected:
  TagMap getTagsBase(const std::string& filename, std::istream& file) const { file.seekg(0); return Info::readTags(clef, filename, file); }
};

#if defined(BUILD_AUDACIOUS)
#include "plugin/audaciousplugin.h"
#elif defined(BUILD_WINAMP)
#include "plugin/winampplugin.h"
#elif defined(BUILD_FOOBAR)
#include "plugin/foobarplugin.h"
#elif defined(BUILD_CLAP)
#include "plugin/clapplugin.h"
#else
namespace Clef {
  ClefPluginBase* makePlugin(ClefContext* ctx);
}

#define CLEF_PLUGIN(PluginInfo) namespace Clef { ClefPluginBase* makePlugin(ClefContext* ctx) { \
  return new ClefPlugin<PluginInfo>(ctx); \
}}
#endif

#endif

#ifndef S2W_BASEPLUGIN_H
#define S2W_BASEPLUGIN_H

#include <string>
#include <utility>
#include <cstdint>
#include <memory>
#include <functional>
#include "utility.h"
#include "tagmap.h"
#include "synth/synthcontext.h"

using ConstPairList = const std::vector<std::pair<std::string, std::string>>;

#define S2WPLUGIN_STATIC_FIELDS \
  static const std::string version, pluginName, pluginShortName, about; \
  static ConstPairList extensions;

struct TagsM3UMixin {
  static TagMap readTags(const OpenFn& openFile, const std::string& filename);
  static inline TagMap readTags(const OpenFn& openFile, const std::string& filename, std::istream& /* unused */) { return readTags(openFile, filename); }
};

#ifdef BUILD_DUMMY_PLUGIN
struct DummyPluginInfo : public TagsM3UMixin {
  S2WPLUGIN_STATIC_FIELDS
  static bool isPlayable(std::istream& file) {
    return false;
  }
  static double length(const OpenFn& openFile, const std::string& filename, std::istream& file) {
    return 0;
  }
  static double sampleRate(const OpenFn& openFile, const std::string& filename, std::istream& file) {
    return 44100;
  }
  SynthContext* prepare(const OpenFn& openFile, const std::string& filename, std::istream& file) {
    // Implementations should retain appropriate pointers
    return nullptr;
  }
  void release() {
    // Implementations should release any retained pointers
  }
};

const std::string DummyPluginInfo::version = "0.0.1";
const std::string DummyPluginInfo::pluginName = "seq2wav";
ConstPairList DummyPluginInfo::extensions = { { "dummy", "Dummy files (*.dummy)" } };
const std::string DummyPluginInfo::about =
  "Dummy plugin copyright (C) 2020 Adam Higerd\n"
  "Distributed under the MIT license.";
#endif

class S2WPluginBase {
public:
  static std::string seq2wavCopyright();

  bool matchExtension(const std::string& filename) const;
  TagMap getTags(const std::string& filename, std::istream& file) const;

  void setOpener(const OpenFn& opener);
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
  virtual bool isPlayable(const std::string& filename, std::istream& file) const = 0;
  virtual double length(const std::string& filename, std::istream& file) const = 0;
  virtual int sampleRate(const std::string& filename, std::istream& file) const = 0; // of unloaded track

protected:
  S2WPluginBase();
  virtual TagMap getTagsBase(const std::string& filename, std::istream& file) const = 0;
  virtual SynthContext* prepare(const std::string& filename, std::istream& file) = 0;
  virtual void release() = 0;

  OpenFn openFile;
  SynthContext* ctx;
};

template <typename PluginInfo>
class S2WPlugin : public S2WPluginBase, public PluginInfo {
public:
  using Info = PluginInfo;

  const std::string& version() const { return Info::version; }
  const std::string& pluginShortName() const { return Info::pluginShortName; }
  const std::string& pluginName() const { return Info::pluginName; }
  const ConstPairList& extensions() const { return Info::extensions; }
  const std::string& about() const {
    static std::string message = Info::about + seq2wavCopyright();
    return message;
  }
  bool isPlayable(const std::string& filename, std::istream& file) const {
    try {
      if (!matchExtension(filename)) {
        return false;
      }
      file.seekg(0);
      return Info::isPlayable(file);
    } catch (...) {
      return false;
    }
  }
  double length(const std::string& filename, std::istream& file) const { file.seekg(0); return Info::length(openFile, filename, file); }
  int sampleRate(const std::string& filename, std::istream& file) const { file.seekg(0); return Info::sampleRate(openFile, filename, file); }
  inline int sampleRate() const { return S2WPluginBase::sampleRate(); }

protected:
  TagMap getTagsBase(const std::string& filename, std::istream& file) const { file.seekg(0); return Info::readTags(openFile, filename, file); }
  SynthContext* prepare(const std::string& filename, std::istream& file) { file.seekg(0); return Info::prepare(openFile, filename, file); }
  void release() { Info::release(); }
};

#define DEFINE_SEQ2WAV_PLUGIN(PluginInfo) static S2WPlugin<PluginInfo> staticPlugin;

#if defined(BUILD_AUDACIOUS)
#include "plugin/audaciousplugin.h"
#elif defined(BUILD_WINAMP)
#include "plugin/winampplugin.h"
#elif defined(BUILD_FOOBAR)
#include "plugin/foobarplugin.h"
#else
#define SEQ2WAV_PLUGIN(PluginInfo) DEFINE_SEQ2WAV_PLUGIN(PluginInfo)
#endif

#endif

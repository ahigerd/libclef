#define WANT_VFS_STDIO_COMPAT
#include <libaudcore/plugin.h>
#include <libaudcore/i18n.h>
#include <libaudcore/audstrings.h>
#include "synth/synthcontext.h"

#ifndef PLUGIN_NAME
#define PLUGIN_NAME seq2wav
#endif

class Seq2WavPlugin : public InputPlugin {
public:
  static const char about[];
  static const char* const exts[];
  static const char* const mimes[];

  static constexpr PluginInfo info = {
    N_(PLUGIN_NAME " Plugin"),
    PLUGIN_NAME, // TODO: ???
    about
  };

  constexpr Seq2WavPlugin();

  bool is_our_file(const char* filename, VFSFile& file);
  bool read_tag(const char* filename, VFSFile& file, Tuple& tuple, Index<char>* image);
  bool play(const char* filename, VFSFile& file);

};

EXPORT Seq2WavPlugin aud_plugin_instance;

constexpr Seq2WavPlugin::Seq2WavPlugin()
: InputPlugin(info, InputInfo().with_priority(8).with_exts(exts).with_mimes(mimes))
{
  // initializers only
}

bool Seq2WavPlugin::is_our_file(const char* filename, VFSFile& file)
{
  // TODO: implementations may want to perform a magic-number check
  std::string fn(filename);
  const char* const* _ext = exts;
  while (*_ext) {
    std::string ext(*_ext);
    if (fn.substr(fn.size() - ext.size(), ext.size()) == ext) {
      return true;
    }
    ++_ext;
  }
  return false;
}

bool Seq2WavPlugin::read_tag(const char* filename, VFSFile& file, Tuple& tuple, Index<char>* image)
{
  // TODO: implementations should provide whatever native tag data is available
  // TODO: seq2wav should provide !tags.m3u support
  return true;
}

bool Seq2WavPlugin::play(const char* filename, VFSFile& file)
{
  // TODO: Create a SynthContext ready to play
  SynthContext ctx(0);
  if (ctx.channels.size() == 0) {
    return false;
  }

  String error;
  // XXX: Ubuntu's audacious-dev doesn't install libaudcore/audio.h
  open_audio(/* FMT_S16_LE */ 3, ctx.sampleRate, ctx.outputChannels);

  uint8_t buffer[1024];
  while (!check_stop()) {
    // TODO: seek support
    size_t written = ctx.fillBuffer(buffer, sizeof(buffer));
    if (!written) {
      break;
    }
    write_audio(buffer, written);
  }

  return true;
}

// TODO: implementations should fill in appropriate values.
// The arrays should end in nullptr.
const char* const Seq2WavPlugin::exts[] = { nullptr };
const char* const Seq2WavPlugin::mimes[] = { nullptr };
const char Seq2WavPlugin::about[] = N_(
    "seq2wav copyright (c) 2020 Adam Higerd.\n"
    "Distributed under the MIT license."
);

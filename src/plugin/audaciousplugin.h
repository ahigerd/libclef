#ifndef CLEF_AUDACITYPLUGIN_H
#define CLEF_AUDACITYPLUGIN_H

#define WANT_VFS_STDIO_COMPAT
#include <iomanip>
#include "plugin/baseplugin.h"
#include <libaudcore/plugin.h>
#include <libaudcore/i18n.h>
#include <libaudcore/audstrings.h>
#include <libaudcore/audio.h>
#include "codec/sampledata.h"
#include "synth/synthcontext.h"
#include "tagmap.h"
#include <iostream>
#include <sstream>
#include <memory>
#include <cstdlib>

class vfsfile_istream : public std::istream {
  class vfsfile_streambuf : public std::basic_streambuf<char> {
  public:
    vfsfile_streambuf(const char* filename) : source(new VFSFile(filename, "rb")), owned(source) {}
    vfsfile_streambuf(VFSFile* source) : source(source), owned(nullptr) {}

  protected:
    std::streamsize xsgetn(char* s, std::streamsize count) {
      return source->fread(s, 1, count);
    }

    traits_type::int_type underflow() {
      if (source && *source) {
        uint8_t result[1];
        int ok = source->fread(reinterpret_cast<char*>(result), 1, 1);
        if (ok) {
          if (source->fseek(-1, VFS_SEEK_CUR)) {
            return traits_type::eof();
          }
          return result[0];
        }
      } else {
      }
      return traits_type::eof();
    }

    traits_type::int_type uflow() {
      if (source && *source) {
        uint8_t result[1];
        int ok = source->fread(reinterpret_cast<char*>(result), 1, 1);
        if (ok) {
          return result[0];
        }
      }
      return traits_type::eof();
    }

    traits_type::pos_type seekoff(traits_type::off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in) {
      if (!source || !*source) {
        return traits_type::off_type(-1);
      }
      if (off != 0) {
        int err = 1;
        if (dir == std::ios_base::beg) {
          err = source->fseek(off, VFS_SEEK_SET);
        } else if (dir == std::ios_base::end) {
          err = source->fseek(off, VFS_SEEK_END);
        } else {
          err = source->fseek(off, VFS_SEEK_CUR);
        }
        if (err) {
          return traits_type::off_type(-1);
        }
      }
      return source->ftell();
    }

    traits_type::pos_type seekpos(traits_type::pos_type pos, std::ios_base::openmode which = std::ios_base::in) {
      if (!source || !*source) {
        return traits_type::off_type(-1);
      }
      int err = source->fseek(pos, VFS_SEEK_SET);
      if (err) {
        return traits_type::off_type(-1);
      }
      return source->ftell();
    }

  private:
    VFSFile* source;
    std::unique_ptr<VFSFile> owned;
  };

public:
  vfsfile_istream(const char* source) : std::istream(new vfsfile_streambuf(source)) {}
  vfsfile_istream(VFSFile* source) : std::istream(new vfsfile_streambuf(source)) {}
  ~vfsfile_istream() { delete rdbuf(nullptr); }
};

template <typename ClefPluginInfo>
class ClefAudPlugin : public InputPlugin {
  struct ExtExpand {
    std::vector<const char*> exts;
    operator const char* const *() const { return exts.data(); }

    ExtExpand() : exts(ClefPluginInfo::extensions.size() + 1, nullptr) {
      for (int i = 0; i < ClefPluginInfo::extensions.size(); i++) {
        exts[i] = ClefPluginInfo::extensions[i].first.c_str();
      }
    }
  };

  ClefContext clef;
  ClefPlugin<ClefPluginInfo> plugin;
  static ExtExpand extensions;

public:
  ClefAudPlugin();

  bool is_our_file(const char* filename, VFSFile& file) {
    vfsfile_istream vs(&file);
    return plugin.isPlayable(filename, vs);
  }

  bool read_tag(const char* filename, VFSFile& file, Tuple& tuple, Index<char>* image) {
    vfsfile_istream vs(&file);
    TagMap tagMap(plugin.getTags(filename, vs));
    if (tagMap.count("length_seconds_fp")) {
      std::istringstream ss(tagMap.at("length_seconds_fp"));
      double len = 0;
      ss >> len;
      if (len) {
        tuple.set_int(Tuple::Length, len * 1000);
      }
    }
    std::string title = tagMap.count("title") ? tagMap.at("title") : std::string();
    tuple.set_int(Tuple::Bitrate, 48 * 2 * 4); // actually kilobitrate: 48kHz * 2 channels * 4 bits/sample
    tuple.set_str(Tuple::Codec, "Sequenced");
    tuple.set_str(Tuple::Quality, "Stereo, 48 kHz");
    if (title.empty()) {
      tuple.set_str(Tuple::Title, filename);
      return true;
    }
    tuple.set_str(Tuple::Title, title.c_str());
    if (tagMap.count("album")) {
      tuple.set_str(Tuple::Album, tagMap.at("album").c_str());
    }
    if (tagMap.count("albumartist")) {
      tuple.set_str(Tuple::AlbumArtist, tagMap.at("albumartist").c_str());
    } else if (tagMap.count("album artist")) {
      tuple.set_str(Tuple::AlbumArtist, tagMap.at("album artist").c_str());
    }
    if (tagMap.count("artist")) {
      tuple.set_str(Tuple::Artist, tagMap.at("artist").c_str());
    }
    if (tagMap.count("genre")) {
      tuple.set_str(Tuple::Genre, tagMap.at("genre").c_str());
    }
    if (tagMap.count("comment")) {
      tuple.set_str(Tuple::Comment, tagMap.at("comment").c_str());
    }
    if (tagMap.count("year")) {
      tuple.set_int(Tuple::Year, std::atoi(tagMap.at("year").c_str()));
    }
    if (tagMap.count("date")) {
      tuple.set_str(Tuple::Date, tagMap.at("date").c_str());
    }
    if (tagMap.count("track")) {
      int trackno = std::atoi(tagMap.at("track").c_str());
      if (trackno) {
        tuple.set_int(Tuple::Track, trackno);
      }
    }
    return true;
  }

  bool play(const char* filename, VFSFile& file);
};

static std::unique_ptr<std::istream> clefAudaciousOpenFile(const std::string& filename)
{
  return std::unique_ptr<std::istream>(new vfsfile_istream(filename.c_str()));
}

template <typename ClefPluginInfo>
ClefAudPlugin<ClefPluginInfo>::ClefAudPlugin()
: InputPlugin(
    PluginInfo{ N_(plugin.pluginName().c_str()), plugin.pluginName().c_str(), plugin.about().c_str() },
    InputInfo().with_priority(8).with_exts(extensions)
  ), plugin(&clef)
{
}

template <typename ClefPluginInfo>
bool ClefAudPlugin<ClefPluginInfo>::play(const char* filename, VFSFile& file)
{
  vfsfile_istream vs(&file);
  bool ok = plugin.play(filename, vs);
  if (ok) {
    open_audio(FMT_S16_LE, plugin.sampleRate(), plugin.channels());

    uint8_t buffer[10240];
    double seekPos;
    size_t written;
    while (!check_stop()) {
      if ((seekPos = check_seek()) >= 0) {
        plugin.seek(seekPos / 1000.0);
      }
      if (!(written = plugin.fillBuffer(buffer, sizeof(buffer)))) {
        break;
      }
      write_audio(buffer, written);
    }
  }
  return ok;
}

#define CLEF_PLUGIN(ClefPluginInfo) \
  template<> ClefAudPlugin<ClefPluginInfo>::ExtExpand ClefAudPlugin<ClefPluginInfo>::extensions = ClefAudPlugin<ClefPluginInfo>::ExtExpand(); \
  EXPORT ClefAudPlugin<ClefPluginInfo> aud_plugin_instance;
#endif

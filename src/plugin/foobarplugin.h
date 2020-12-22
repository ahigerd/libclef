#ifndef S2W_FOOBARPLUGIN_H
#define S2W_FOOBARPLUGIN_H

#include "foobar2000/SDK/foobar2000.h"
#include "s2wconfig.h"
#include "plugin/baseplugin.h"
#include "codec/sampledata.h"
#include "synth/synthcontext.h"
#include "tagmap.h"
#include "utility.h"
#include <iostream>
#include <memory>
#include <sstream>

static std::unordered_map<std::string, std::string> tagKeys = {
  { "length_seconds_fp", "" },
  { "display_title", "" },
  { "year", "date" },
  { "track", "tracknumber" },
  { "albumartist", "album artist" },
};

class foofile_istream : public std::istream {
  class foofile_streambuf : public std::basic_streambuf<char> {
  public:
    foofile_streambuf(service_ptr_t<file> p_filehint, const char* p_path, t_input_open_reason p_reason, abort_callback& p_abort)
    : m_file(p_filehint), m_abort(&p_abort) {
      if (filesystem::g_exists(p_path, p_abort)) {
        input_open_file_helper(m_file, p_path, p_reason, p_abort);
      } else {
        m_file.release();
      }
    }

    foofile_streambuf(service_ptr_t<file> p_file, abort_callback& p_abort)
    : m_file(p_file), m_abort(&p_abort) {
      // initializers only
    }

    service_ptr_t<file> m_file;
    abort_callback* m_abort;

  protected:
    std::streamsize xsgetn(char* data, std::streamsize length) {
      if (!m_file.get_ptr()) {
        return 0;
      }
      return m_file->read(data, length, *m_abort);
    }

    traits_type::int_type underflow() {
      if (m_file.get_ptr()) {
        char result[1];
        int ok = m_file->read(result, 1, *m_abort);
        if (ok) {
          m_file->seek_ex(-1, file::seek_from_current, *m_abort);
          return result[0];
        }
      }
      return traits_type::eof();
    }

    traits_type::int_type uflow() {
      if (m_file.get_ptr()) {
        char result[1];
        int ok = m_file->read(result, 1, *m_abort);
        if (ok) {
          return result[0];
        }
      }
      return traits_type::eof();
    }

    traits_type::pos_type seekoff(traits_type::off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in) {
      if (!m_file.get_ptr()) {
        return traits_type::eof();
      }
      if (off != 0) {
        if (dir == std::ios_base::beg) {
          m_file->seek_ex(off, file::seek_from_beginning, *m_abort);
        } else if (dir == std::ios_base::end) {
          m_file->seek_ex(off, file::seek_from_eof, *m_abort);
        } else {
          m_file->seek_ex(off, file::seek_from_current, *m_abort);
        }
      }
      return m_file->get_position(*m_abort);
    }

    traits_type::pos_type seekpos(traits_type::pos_type pos, std::ios_base::openmode which = std::ios_base::in) {
      if (!m_file.get_ptr()) {
        return traits_type::eof();
      }
      m_file->seek(pos, *m_abort);
      return m_file->get_position(*m_abort);
    }
  };

  void prepare() {
    if (!fooFile().get_ptr()) {
      clear(std::ios::badbit);
    } else {
      seekg(0);
    }
  }

public:
  foofile_istream(service_ptr_t<file> p_filehint, const char* p_path, t_input_open_reason p_reason, abort_callback& p_abort)
    : std::istream(new foofile_streambuf(p_filehint, p_path, p_reason, p_abort)) { prepare(); }
  foofile_istream(service_ptr_t<file> p_file, abort_callback& p_abort)
    : std::istream(new foofile_streambuf(p_file, p_abort)) { prepare(); }
  foofile_istream(const std::string& filename, abort_callback& p_abort)
    : std::istream(new foofile_streambuf(nullptr, filename.c_str(), input_open_info_read, p_abort)) { prepare(); }
  ~foofile_istream() { delete rdbuf(nullptr); }

  service_ptr_t<file>& fooFile() { return static_cast<foofile_streambuf*>(rdbuf())->m_file; }
};

static abort_callback_impl dummyAbort;

template <typename S2WPluginInfo>
class input_seq2wav : public input_stubs {
  using PluginType = S2WPlugin<S2WPluginInfo>;
public:
  static PluginType plugin;

  input_seq2wav() {
    plugin.setOpener([](const std::string& filename) {
      auto stream = std::unique_ptr<std::istream>(new foofile_istream(filename, dummyAbort));
      stream->seekg(0);
      return stream;
    });
  }

	void open(service_ptr_t<file> p_filehint, const char* p_path, t_input_open_reason p_reason, abort_callback& p_abort) {
    foofile_istream file(p_filehint, p_path, p_reason, p_abort);
    if (!file) {
      throw std::exception("File does not exist");
    }
    if (!plugin.isPlayable(p_path, file)) {
      throw std::exception("File is not playable");
    }
    m_filename = p_path;
    m_file = file.fooFile();
    fileInfo.set_length(plugin.length(p_path, file));
    TagMap tagMap = plugin.getTags(p_path, file);
    for (const auto& iter : tagMap) {
      if (iter.first == "year" && tagMap.count("date") && !tagMap.at("date").empty()) {
        continue;
      }
      std::string key = tagKeys.count(iter.first) ? tagKeys.at(iter.first) : iter.first;
      if (key.empty()) {
        continue;
      }
      fileInfo.meta_add(key.c_str(), iter.second.c_str());
    }
    fileInfo.info_set_int("samplerate", 48000);
    fileInfo.info_set_int("channels", 2);
    fileInfo.info_set_int("bitspersample", 16);
    fileInfo.info_set_bitrate(4 * 2 * 48); // actually kilobitrate
  }

	void get_info(file_info& p_info, abort_callback& p_abort) {
    p_info.copy(fileInfo);
  }
	void retag(const file_info& p_info, abort_callback& p_abort) { throw exception_io_unsupported_format(); }
	t_filestats get_file_stats(abort_callback& p_abort) {
    return m_file->get_stats(p_abort);
  }

	void decode_initialize(unsigned p_flags, abort_callback& p_abort) {
    SampleData::purge();
    p_abort.check();
    foofile_istream stream(m_file, p_abort);
    bool ok = plugin.play(m_filename, stream);
    p_abort.check();
    if (!ok) {
      plugin.unload();
      throw exception_io_data("unable to load file");
    }
  }
	bool decode_run(audio_chunk& p_chunk, abort_callback& p_abort) {
    p_abort.check();
    size_t written = plugin.fillBuffer(buffer, sizeof(buffer));
    if (!written) {
      plugin.unload();
      return false;
    }
		p_chunk.set_data_fixedpoint(reinterpret_cast<char*>(buffer), written, 48000, 2, 16, audio_chunk::g_guess_channel_config(2));
    return true;
  }
	void decode_seek(double p_seconds, abort_callback& p_abort) {
    plugin.seek(p_seconds);
  }
	bool decode_can_seek() { return true; }
	void decode_on_idle(abort_callback& p_abort) {}

	static bool g_is_our_content_type(const char * p_content_type) { return false; }
	static bool g_is_our_path(const char * p_path,const char * p_extension) { return plugin.matchExtension(p_path); }
	static GUID g_get_guid() {
    GUID guid = { 0xf85ca9fe, 0x228c, 0x4f26, { 0xa1, 0xf4, 0xd0, 0x2b, 0x59, 0x7e, 0xa9, 0x12 } };
    int pos = 0;
    // really simplistic hash
    for (char ch : plugin.pluginShortName()) {
      guid.Data4[pos] ^= unsigned char(ch);
      pos = (pos + 1) % 8;
      guid.Data4[pos] ^= unsigned char(ch << 4);
    }
    return guid;
  }
	static const char * g_get_name() { return plugin.pluginName().c_str(); }
	//! See: input_entry::get_preferences_guid().
	// static GUID g_get_preferences_guid();

  uint8_t buffer[1024];
  service_ptr_t<file> m_file;
  file_info_impl fileInfo;
  std::string m_filename;
};

bool s2wFoobarMeta(const S2WPluginBase& plugin)
{
  DECLARE_COMPONENT_VERSION_COPY(plugin.pluginName().c_str(), plugin.version().c_str(), plugin.about().c_str());

  static std::vector<std::unique_ptr<input_file_type_impl>> filetype_instances;
  static std::vector<std::unique_ptr<service_factory_single_ref_t<input_file_type_impl>>> filetype_services;
  static std::vector<std::string> stringBuffer;
  for (const auto& ext : plugin.extensions()) {
    stringBuffer.emplace_back("*." + ext.first);
    auto ift = new input_file_type_impl(ext.second.c_str(), stringBuffer.back().c_str(), true);
    filetype_instances.emplace_back(ift);
    filetype_services.emplace_back(new service_factory_single_ref_t<input_file_type_impl>(*ift));
  }

  return true;
}

#define SEQ2WAV_PLUGIN(S2WPluginInfo) \
  using S2W = input_seq2wav<S2WPluginInfo>; \
  template<> S2WPlugin<S2WPluginInfo> S2W::plugin = S2WPlugin<S2WPluginInfo>(); \
  DECLARE_COMPONENT_VERSION_COPY(S2W::plugin.pluginName().c_str(), S2W::plugin.version().c_str(), S2W::plugin.about().c_str()); \
  static bool metaOK = s2wFoobarMeta(S2W::plugin); \
  static input_singletrack_factory_t<input_seq2wav<S2WPluginInfo>> g_input_seq2wav_factory; \
  static std::string pluginFilename = "foo_input_" + S2WPluginInfo::pluginShortName + ".dll"; \
  VALIDATE_COMPONENT_FILENAME(pluginFilename.c_str());

#endif

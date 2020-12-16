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
  { "track", "tracknumber" },
};

class foofile_istream : public std::istream {
  class foofile_streambuf : public std::basic_streambuf<char> {
  public:
    foofile_streambuf(service_ptr_t<file> p_filehint, const char* p_path, t_input_open_reason p_reason, abort_callback& p_abort)
    : m_file(p_filehint), m_abort(&p_abort) {
      input_open_file_helper(m_file, p_path, p_reason, p_abort);
    }

    foofile_streambuf(service_ptr_t<file> p_file, abort_callback& p_abort)
    : m_file(p_file), m_abort(&p_abort) {
      // initializers only
    }

    service_ptr_t<file> m_file;
    abort_callback* m_abort;

  protected:
    std::streamsize xsgetn(char* data, std::streamsize length) {
      return m_file->read(data, length, *m_abort);
    }

    traits_type::int_type underflow() {
      return traits_type::eof();
    }
  };

public:
  foofile_istream(service_ptr_t<file> p_filehint, const char* p_path, t_input_open_reason p_reason, abort_callback& p_abort)
    : std::istream(new foofile_streambuf(p_filehint, p_path, p_reason, p_abort)) {}
  foofile_istream(service_ptr_t<file> p_file, abort_callback& p_abort)
    : std::istream(new foofile_streambuf(p_file, p_abort)) {}
  foofile_istream(const std::string& filename, abort_callback& p_abort)
    : std::istream(new foofile_streambuf(nullptr, filename.c_str(), input_open_info_read, p_abort)) {}
  ~foofile_istream() { delete rdbuf(nullptr); }

  service_ptr_t<file>& fooFile() { return static_cast<foofile_streambuf*>(rdbuf())->m_file; }
};

static abort_callback_impl dummyAbort;

template <typename S2WPluginInfo>
class input_seq2wav : public input_stubs {
  using PluginType = S2WPlugin<S2WPluginInfo>;
  static PluginType plugin;
public:
  input_seq2wav() {
    plugin.setOpener([](const std::string& filename) {
      return std::unique_ptr<std::istream>(new foofile_istream(filename, dummyAbort));
    });
  }

	void open(service_ptr_t<file> p_filehint, const char* p_path, t_input_open_reason p_reason, abort_callback& p_abort) {
    foofile_istream file(p_filehint, p_path, p_reason, p_abort);
    if (!plugin.isPlayable(p_path, file)) {
      throw std::exception("File is not playable");
    }
    m_filename = p_path;
    m_file = file.fooFile();
    fileInfo.set_length(plugin.length(p_path, file));
    TagMap tagMap = plugin.getTags(p_path, file);
    for (const auto& iter : tagMap) {
      std::string key = tagKeys.count(iter.first) ? tagKeys.at(iter.first) : iter.first;
      if (key.empty()) {
        continue;
      }
      fileInfo.meta_add(key.c_str(), iter.second.c_str());
    }
    fileInfo.info_set_int("samplerate", 48000);
    fileInfo.info_set_int("channels", 2);
    fileInfo.info_set_int("bitspersample", 16);
    fileInfo.info_set("encoding", "lossless");
    fileInfo.info_set_bitrate(16 * 2 * 48); // actually kilobitrate
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
      throw exception_io_data("unable to load ifs");
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

#define SEQ2WAV_PLUGIN(S2WPluginInfo) \
  template<> S2WPlugin<S2WPluginInfo> input_seq2wav<S2WPluginInfo>::plugin = S2WPlugin<S2WPluginInfo>(); \
  static input_singletrack_factory_t<input_seq2wav<S2WPluginInfo>> g_input_seq2wav_factory;

#endif

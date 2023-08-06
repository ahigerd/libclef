#ifndef S2W_CLAPPLUGIN_H
#define S2W_CLAPPLUGIN_H
#ifdef BUILD_CLAP

#include "plugin/baseplugin.h"
#include "plugin/realtimetrack.h"
#include <clap/clap.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <cstring>
namespace pfd {
  class open_file;
  class message;
}

class S2WClapPluginBase
{
public:
  clap_plugin_t plugin;
  const clap_host_t* host;
  S2WContext* ctx;
  S2WPluginBase* s2wPlugin;
  SynthContext* synth;
  RealTimeTrack seq;

  S2WClapPluginBase(const clap_host_t* host);
  ~S2WClapPluginBase();

  virtual bool init();
  virtual void destroy();
  virtual bool activate(double sampleRate, uint32_t minFrames, uint32_t maxFrames);
  virtual void deactivate();
  virtual bool startProcessing();
  virtual void stopProcessing();
  virtual void reset();
  clap_process_status process(const clap_process_t* p);
  const void* getExtension(const char* id);
  virtual void onMainThread();

  virtual uint32_t notePortCount(bool isInput) const;
  virtual bool getNotePort(uint32_t index, bool isInput, clap_note_port_info_t& port) const;

  virtual uint32_t audioPortCount(bool isInput) const;
  virtual bool getAudioPort(uint32_t index, bool isInput, clap_audio_port_info_t& port) const;

  virtual uint32_t paramCount() const;
  virtual bool paramInfo(uint32_t index, clap_param_info_t& info) const;
  virtual bool paramValue(clap_id id, double& value) const;
  virtual bool paramValueText(clap_id id, double value, char* text, uint32_t size) const;
  virtual bool paramTextValue(clap_id id, const char* text, double& value) const;
  virtual void flushParams(const clap_input_events_t* inEvents, const clap_output_events_t* outEvents);

  virtual bool saveState(const clap_ostream_t* stream);
  virtual bool loadState(const clap_istream_t* stream);

protected:
  void requestParamSync(bool rescanInfo = false);

  void buildContext(const std::string& filename, uint64_t instID = 0xFFFFFFFFFFFFFFFFULL);
  virtual SynthContext* createContext(const std::string& filename, std::istream& file) = 0;
  virtual BaseNoteEvent* createNoteEvent(const clap_event_note_t* event);
  virtual void prepareChannel(Channel* channel) {}

  virtual void getParamRange(uint32_t id, double& minValue, double& maxValue, double& defaultValue) const;

  virtual void dispatchEvent(const clap_event_header_t* event);
  virtual void noteEvent(const clap_event_note_t* event);
  virtual void expressionEvent(const clap_event_note_expression_t* event);
  virtual void paramValueEvent(const clap_event_param_value_t* event);
  virtual void paramModEvent(const clap_event_param_mod_t* event);
  virtual void transportEvent(const clap_event_transport_t* event);
  virtual void midiEvent(const clap_event_midi_t* event);
  virtual void sysexEvent(const clap_event_midi_sysex_t* event);
  virtual void midi2Event(const clap_event_midi2_t* event);
  virtual IInstrument* selectInstrumentByIndex(uint32_t index, bool force = false, bool rescan = true);
  virtual IInstrument* selectInstrumentByID(uint64_t id, bool force = false, bool rescan = true);
  inline IInstrument* currentInstrument() const { return instrument; }
  inline uint32_t currentInstrumentID() const { return currentInstID; }

  clap_plugin_note_ports_t notePorts;
  clap_plugin_audio_ports_t audioPorts;
  clap_plugin_params_t paramsExtension;
  clap_plugin_state_t stateExtension;

  double eventTimestamp(const clap_event_header_t* event) const;

  template <typename EVENT>
  inline double eventTimestamp(const EVENT* event) const
  { return eventTimestamp(&event->header); }

  mutable std::mutex synthMutex;

private:
  std::unordered_map<uint32_t, uint32_t> nextPlaybackIDs;
  uint64_t currentInstID;
  IInstrument* instrument;
  std::string filePath;
  std::vector<uint32_t> paramOrder;
  std::unordered_set<uint32_t> chanParams;
  std::unordered_set<uint32_t> noteParams;
  const clap_host_params_t* hostParams;
  std::thread::id mainThreadID;
  std::atomic<bool> mustRescanInfo, queueRescanValues, mustRestart;
  pfd::open_file* openFileDialog;
  pfd::message* messageDialog;
};

template <typename S2WPluginInfo>
class S2WClapPlugin : public S2WClapPluginBase
{
public:
  using PluginBase = S2WClapPlugin<S2WPluginInfo>;
  using PluginInfo = S2WPluginInfo;

  S2WClapPlugin(const clap_host_t* host) : S2WClapPluginBase(host)
  {
    s2wPlugin = new S2WPlugin<PluginInfo>(ctx);
  }

  SynthContext* createContext(const std::string& filename, std::istream& file)
  {
    return s2wPlugin->prepare(filename, file);
  }
};

template <class S2WPluginInfo>
struct S2WPluginFactory
{
  static const clap_plugin_descriptor_t* descriptor()
  {
    static std::string id("s2w." + S2WPluginInfo::pluginShortName);

    static const char* features[] = {
      CLAP_PLUGIN_FEATURE_INSTRUMENT,
      CLAP_PLUGIN_FEATURE_SYNTHESIZER,
      CLAP_PLUGIN_FEATURE_STEREO,
      nullptr,
    };

    static const clap_plugin_descriptor_t desc = {
      .clap_version = CLAP_VERSION_INIT,
      .id = id.c_str(),
      .name = S2WPluginInfo::pluginName.c_str(),
      .vendor = S2WPluginInfo::author.c_str(),
      .url = S2WPluginInfo::url.c_str(),
      .manual_url = S2WPluginInfo::url.c_str(),
      .support_url = S2WPluginInfo::url.c_str(),
      .version = S2WPluginInfo::version.c_str(),
      .description = S2WPluginInfo::about.c_str(),
      .features = features,
    };

    return &desc;
  }

  static constexpr clap_plugin_factory_t pluginFactory = {
    .get_plugin_count = [](const clap_plugin_factory* factory) -> uint32_t { return 1; },
    .get_plugin_descriptor = [](const clap_plugin_factory* factory, uint32_t index) -> const clap_plugin_descriptor_t* {
      return index == 0 ? descriptor() : nullptr;
    },
    .create_plugin = [](const clap_plugin_factory* factory, const clap_host_t* host, const char* pluginID) -> const clap_plugin_t* {
      if (!clap_version_is_compatible(host->clap_version) || std::strcmp(pluginID, descriptor()->id)) {
        return nullptr;
      }
      auto* plugin = new typename S2WPluginInfo::ClapPlugin(host);
      plugin->plugin.desc = descriptor();
      return &plugin->plugin;
    },
  };
};

#define SEQ2WAV_PLUGIN(S2WPluginInfo) \
  extern "C" const clap_plugin_entry_t clap_entry = { \
    .clap_version = CLAP_VERSION_INIT, \
    .init = [](const char *path) -> bool { return true; }, \
    .deinit = []() {}, \
    .get_factory = [] (const char *factoryID) -> const void* { \
      return strcmp(factoryID, CLAP_PLUGIN_FACTORY_ID) ? nullptr : &S2WPluginFactory<S2WPluginInfo>::pluginFactory; \
    }, \
  };

#else

using clap_host_t = void;

class S2WClapPluginBase
{
public:
  S2WClapPluginBase(const void*) {}
};

template <typename S2WPluginInfo>
class S2WClapPlugin : public S2WClapPluginBase
{
public:
  S2WClapPlugin(const void*) {}
};

#endif
#endif

#ifdef BUILD_CLAP
#include "plugin/clapplugin.h"
#include "plugin/portable-file-dialogs.h"
#include "synth/channel.h"
#include "synth/iinstrument.h"
#include <fstream>
#include <thread>
#include <sstream>
#include <limits.h>

#define C_P const clap_plugin_t* plugin
#define WRAP_METHOD(RET, method, sig, ...) []sig -> RET { return reinterpret_cast<S2WClapPluginBase*>(plugin->plugin_data)->method(__VA_ARGS__); }
#define WRAP_METHOD_VOID(method, sig, ...) []sig { reinterpret_cast<S2WClapPluginBase*>(plugin->plugin_data)->method(__VA_ARGS__); }
S2WClapPluginBase::S2WClapPluginBase(const clap_host_t* host)
: host(host), ctx(new S2WContext(true)), synth(nullptr), currentInstID(0), instrument(nullptr), hostParams(nullptr), mustRescanInfo(true),
  mustRestart(false), openFileDialog(nullptr)
{
  mainThreadID = std::this_thread::get_id();

  plugin.plugin_data = this;
  plugin.init = WRAP_METHOD(bool, init, (C_P));
  plugin.destroy = WRAP_METHOD_VOID(destroy, (C_P));
  plugin.activate = WRAP_METHOD(bool, activate, (C_P, double sr, uint32_t min, uint32_t max), sr, min, max);
  plugin.deactivate = WRAP_METHOD_VOID(deactivate, (C_P));
  plugin.start_processing = WRAP_METHOD(bool, startProcessing, (C_P));
  plugin.stop_processing = WRAP_METHOD_VOID(stopProcessing, (C_P));
  plugin.reset = WRAP_METHOD_VOID(reset, (C_P));
  plugin.process = WRAP_METHOD(clap_process_status, process, (C_P, const clap_process_t* p), p);
  plugin.get_extension = WRAP_METHOD(const void*, getExtension, (C_P, const char* id), id);
  plugin.on_main_thread = WRAP_METHOD_VOID(onMainThread, (C_P));

  notePorts.count = WRAP_METHOD(uint32_t, notePortCount, (C_P, bool isInput), isInput);
  notePorts.get = WRAP_METHOD(bool, getNotePort, (C_P, uint32_t index, bool isInput, clap_note_port_info_t* port), index, isInput, *port);

  audioPorts.count = WRAP_METHOD(uint32_t, audioPortCount, (C_P, bool isInput), isInput);
  audioPorts.get = WRAP_METHOD(bool, getAudioPort, (C_P, uint32_t index, bool isInput, clap_audio_port_info_t* port), index, isInput, *port);

  paramsExtension.count = WRAP_METHOD(uint32_t, paramCount, (C_P));
  paramsExtension.get_info = WRAP_METHOD(bool, paramInfo, (C_P, uint32_t id, clap_param_info_t* info), id, *info);
  paramsExtension.get_value = WRAP_METHOD(bool, paramValue, (C_P, clap_id id, double* value), id, *value);
  paramsExtension.value_to_text = WRAP_METHOD(bool, paramValueText, (C_P, clap_id id, double value, char* text, uint32_t size), id, value, text, size);
  paramsExtension.text_to_value = WRAP_METHOD(bool, paramTextValue, (C_P, clap_id id, const char* text, double* value), id, text, *value);
  paramsExtension.flush = WRAP_METHOD_VOID(flushParams, (C_P, const clap_input_events_t* inEvents, const clap_output_events_t* outEvents), inEvents, outEvents);

  stateExtension.save = WRAP_METHOD(bool, saveState, (C_P, const clap_ostream_t* stream), stream);
  stateExtension.load = WRAP_METHOD(bool, loadState, (C_P, const clap_istream_t* stream), stream);
}

S2WClapPluginBase::~S2WClapPluginBase()
{
  if (synth) {
    delete synth;
  }
  if (openFileDialog) {
    delete openFileDialog;
  }
}

bool S2WClapPluginBase::init()
{
  hostParams = reinterpret_cast<const clap_host_params_t*>(host->get_extension(host, CLAP_EXT_PARAMS));
  synth = new SynthContext(ctx, 48000, 2);
  paramOrder = { 'FNAM', 'inst' };
  return true;
}

void S2WClapPluginBase::destroy()
{
  if (synth) {
    delete synth;
    synth = nullptr;
  }
  delete this;
}

bool S2WClapPluginBase::activate(double sampleRate, uint32_t minFrames, uint32_t maxFrames)
{
  std::lock_guard lock(synthMutex);
  std::cerr << "activate" << std::endl;
  if (!filePath.empty()) {
    if (synth) {
      delete synth;
    }
    std::ifstream file(filePath);
    synth = createContext(ctx, filePath, file);
    synth->addChannel(&seq);
    selectInstrumentByIndex(0, true, false);
    seq.addEvent(new ChannelEvent('inst', uint64_t(currentInstID)));
  }
  synth->setSampleRate(sampleRate);
  return true;
}

void S2WClapPluginBase::deactivate()
{
  std::cerr << "deactivate" << std::endl;
  seq.reset();
}

bool S2WClapPluginBase::startProcessing()
{
  return true;
}

void S2WClapPluginBase::stopProcessing()
{
}

void S2WClapPluginBase::reset()
{
  if (!synth) {
    return;
  }
  double sampleRate = synth->sampleRate;
  deactivate();
  activate(sampleRate, 0, 0);
}

clap_process_status S2WClapPluginBase::process(const clap_process_t* process)
{
  if (!synth) {
    return CLAP_PROCESS_CONTINUE;
  }

  if (openFileDialog && openFileDialog->ready(0)) {
    auto fn = openFileDialog->result();
    delete openFileDialog;
    openFileDialog = nullptr;
    if (fn.size()) {
      std::cerr << "selected " << fn[0] << std::endl;
      filePath = fn[0];
      activate(synth->sampleRate, 0, 0);
      requestParamSync(true);
      return CLAP_PROCESS_CONTINUE;
    } else {
      std::cerr << "canceled" << std::endl;
      requestParamSync(false);
    }
  }

  try {
    uint32_t numEvents = process->in_events->size(process->in_events);

    for (uint32_t i = 0; i < numEvents; i++) {
      const clap_event_header_t *event = process->in_events->get(process->in_events, i);
      dispatchEvent(event);
    }

    float* buffers[2] = {
      process->audio_outputs[0].data32[0],
      process->audio_outputs[0].data32[1],
    };
    {
      std::lock_guard lock(synthMutex);
      synth->fillBuffers(buffers, process->frames_count);
    }

    if (queueRescanValues) {
      host->request_callback(host);
      queueRescanValues = false;
    }

    return CLAP_PROCESS_CONTINUE;
  } catch (std::exception& e) {
    return CLAP_PROCESS_ERROR;
  }
}

double S2WClapPluginBase::eventTimestamp(const clap_event_header_t* event) const
{
  if (synth) {
    //return (steadyTime + event->time) * synth->sampleTime;
    return synth->currentTime() + (event->time * synth->sampleTime);
  }
  return -1;
}

void S2WClapPluginBase::dispatchEvent(const clap_event_header_t* event)
{
  if (event->space_id != CLAP_CORE_EVENT_SPACE_ID) {
    return;
  }
  switch (event->type) {
  case CLAP_EVENT_NOTE_ON:
  case CLAP_EVENT_NOTE_OFF:
  case CLAP_EVENT_NOTE_CHOKE:
    noteEvent(reinterpret_cast<const clap_event_note_t*>(event));
    return;
  case CLAP_EVENT_NOTE_EXPRESSION:
    expressionEvent(reinterpret_cast<const clap_event_note_expression_t*>(event));
    return;
  case CLAP_EVENT_PARAM_VALUE:
    paramValueEvent(reinterpret_cast<const clap_event_param_value_t*>(event));
    return;
  case CLAP_EVENT_PARAM_MOD:
    paramModEvent(reinterpret_cast<const clap_event_param_mod_t*>(event));
    return;
  case CLAP_EVENT_TRANSPORT:
    transportEvent(reinterpret_cast<const clap_event_transport_t*>(event));
    return;
  case CLAP_EVENT_MIDI:
    midiEvent(reinterpret_cast<const clap_event_midi_t*>(event));
    return;
  case CLAP_EVENT_MIDI_SYSEX:
    sysexEvent(reinterpret_cast<const clap_event_midi_sysex_t*>(event));
    return;
  case CLAP_EVENT_MIDI2:
    midi2Event(reinterpret_cast<const clap_event_midi2_t*>(event));
    return;
  }
}

const void* S2WClapPluginBase::getExtension(const char* id)
{
  if (!strcmp(id, CLAP_EXT_NOTE_PORTS)) {
    return &notePorts;
  } else if (!strcmp(id, CLAP_EXT_AUDIO_PORTS)) {
    return &audioPorts;
  } else if (!strcmp(id, CLAP_EXT_PARAMS)) {
    return &paramsExtension;
  } else if (!strcmp(id, CLAP_EXT_STATE)) {
    return &stateExtension;
  }
  return nullptr;
}

void S2WClapPluginBase::onMainThread()
{
  if (mustRestart) {
    host->request_restart(host);
  } else if (mustRescanInfo) {
    mustRescanInfo = false;
    hostParams->rescan(host, CLAP_PARAM_RESCAN_ALL | CLAP_PARAM_RESCAN_INFO | CLAP_PARAM_RESCAN_VALUES | CLAP_PARAM_RESCAN_TEXT);
    queueRescanValues = true;
  } else {
    hostParams->rescan(host, CLAP_PARAM_RESCAN_VALUES | CLAP_PARAM_RESCAN_TEXT);
  }
}

BaseNoteEvent* S2WClapPluginBase::createNoteEvent(const clap_event_note_t* event)
{
  InstrumentNoteEvent* note = new InstrumentNoteEvent();
  note->pitch = event->key;
  note->volume = event->velocity;
  note->timestamp = eventTimestamp(event);
  return note;
}

void S2WClapPluginBase::noteEvent(const clap_event_note_t* event)
{
  if (event->header.type == CLAP_EVENT_NOTE_ON) {
    BaseNoteEvent* note = createNoteEvent(event);
    if (event->note_id >= 0) {
      note->playbackID = event->note_id;
    } else {
      note->playbackID = uint64_t(++nextPlaybackIDs[event->key]) | (uint64_t(event->key) << 32);
    }
    seq.addEvent(note);
  } else {
    if (event->note_id >= 0) {
      KillEvent* kill = new KillEvent(event->note_id, event->header.type == CLAP_EVENT_NOTE_CHOKE);
      kill->timestamp = eventTimestamp(event);
      seq.addEvent(kill);
    } else {
      const auto& chan = synth->channels[0];
      // TODO: A note that's shorter than one process interval will fail this lookup.
      // There's also a risk that this isn't threadsafe if the main thread can ever
      // mutate the notes structure.
      for (const auto& iter : chan->notes) {
        if (event->key < 0 || (iter.first >> 32) == event->key) {
          KillEvent* kill = new KillEvent(iter.first, event->header.type == CLAP_EVENT_NOTE_CHOKE);
          kill->timestamp = eventTimestamp(event);
          seq.addEvent(kill);
        }
      }
    }
  }
}

void S2WClapPluginBase::expressionEvent(const clap_event_note_expression_t* event) {}

void S2WClapPluginBase::paramValueEvent(const clap_event_param_value_t* event)
{
  if (event->param_id == 'FNAM') {
    if (event->value < 0.5) {
      return;
    }
    if (openFileDialog) {
      requestParamSync(false);
      return;
    }
    openFileDialog = new pfd::open_file("Select file", filePath);
  } else if (event->param_id == 'inst') {
    std::lock_guard lock(synthMutex);
    IInstrument* found = selectInstrumentByIndex(uint64_t(event->value));
    if (found) {
      ChannelEvent* ch = new ChannelEvent('inst', uint64_t(currentInstID));
      ch->timestamp = eventTimestamp(event);
      seq.addEvent(ch);
    }
  } else if (event->note_id > 0) {
    ModulatorEvent* mod = new ModulatorEvent(event->note_id, event->param_id, event->value);
    mod->timestamp = eventTimestamp(event);
    seq.addEvent(mod);
  } else if (event->key >= 0) {
    const auto& chan = synth->channels[0];
    // TODO: Modulating a note that started during this process interval will fail this lookup.
    for (const auto& iter : chan->notes) {
      if ((iter.first >> 32) == event->key) {
        ModulatorEvent* mod = new ModulatorEvent(iter.first, event->param_id, event->value);
        mod->timestamp = eventTimestamp(event);
        seq.addEvent(mod);
      }
    }
  } else {
    ChannelEvent* ch = new ChannelEvent(event->param_id, event->value);
    ch->timestamp = eventTimestamp(event);
    seq.addEvent(ch);
    //requestParamSync(false);
  }
}

void S2WClapPluginBase::paramModEvent(const clap_event_param_mod_t* event)
{
}

void S2WClapPluginBase::transportEvent(const clap_event_transport_t* event) {}
void S2WClapPluginBase::midiEvent(const clap_event_midi_t* event) {}
void S2WClapPluginBase::sysexEvent(const clap_event_midi_sysex_t* event) {}
void S2WClapPluginBase::midi2Event(const clap_event_midi2_t* event) {}

uint32_t S2WClapPluginBase::notePortCount(bool isInput) const
{
  if (isInput) {
    return 1;
  }
  return 0;
}

bool S2WClapPluginBase::getNotePort(uint32_t index, bool isInput, clap_note_port_info_t& port) const
{
  if (!isInput || index != 0) {
    return false;
  }
  port.id = 0;
  port.supported_dialects = CLAP_NOTE_DIALECT_CLAP;
  port.preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
  snprintf(port.name, sizeof(port.name), "%s", "Note Port");
  return true;
}

uint32_t S2WClapPluginBase::audioPortCount(bool isInput) const
{
  if (isInput) {
    return 0;
  }
  return 1;
}

bool S2WClapPluginBase::getAudioPort(uint32_t index, bool isInput, clap_audio_port_info_t& port) const
{
  if (isInput || index != 0) {
    return false;
  }
  port.id = 0;
  port.channel_count = 2;
  port.flags = CLAP_AUDIO_PORT_IS_MAIN;
  port.port_type = CLAP_PORT_STEREO;
  port.in_place_pair = CLAP_INVALID_ID;
  snprintf(port.name, sizeof(port.name), "%s", "Audio Output");
  return true;
}

uint32_t S2WClapPluginBase::paramCount() const
{
  return paramOrder.size();
}

bool S2WClapPluginBase::paramInfo(uint32_t index, clap_param_info_t& info) const
{
  if (index >= paramOrder.size()) {
    std::cerr << "unknown param index " << index << std::endl;
    return false;
  }
  std::memset(&info, '\0', sizeof(clap_param_info_t));
  uint32_t id = paramOrder[index];
  info.id = id;
  if (id == 'FNAM') {
    info.flags |= CLAP_PARAM_IS_AUTOMATABLE;
    info.flags |= CLAP_PARAM_IS_STEPPED;
    info.min_value = 0;
    info.max_value = 1;
    info.default_value = 0;
    std::string name = "Load file...";
    std::strncpy(info.name, name.c_str(), sizeof(info.name));
    return true;
  }

  if (id == 'inst' || chanParams.count(id)) {
    info.flags |= CLAP_PARAM_IS_AUTOMATABLE;
  }
  if (noteParams.count(id)) {
    info.flags |= CLAP_PARAM_IS_AUTOMATABLE_PER_NOTE_ID;
  }
  if (!info.flags) {
    std::cerr << "unknown param " << fourccToString(id) << std::endl;
    return false;
  }
  if (id == 'inst') {
    info.flags |= CLAP_PARAM_IS_STEPPED;
    info.min_value = 0;
    std::lock_guard lock(synthMutex);
    info.max_value = synth->numInstruments() - 1;
  } else {
    // TODO: ranges
    info.min_value = 0.0f;
    info.max_value = 1.0f;
    info.default_value = 0.5f;
  }
  auto name = fourccToString(id);
  std::strncpy(info.name, name.c_str(), 5);
  return true;
}

bool S2WClapPluginBase::paramValue(clap_id id, double& value) const
{
  std::lock_guard lock(synthMutex);
  if (id == 'FNAM') {
    value = 0;
    return true;
  } else if (id == 'inst') {
    int numInsts = synth->numInstruments();
    for (int i = 0; i < numInsts; i++) {
      if (synth->instrumentID(i) == currentInstID) {
        value = i;
        return true;
      }
    }
    std::cerr << "could not find instrument " << currentInstID << " among " << numInsts << std::endl;
    return false;
  }
  if (!synth->channels.size()) {
    return false;
  }
  auto param = synth->channels[0]->param(uint32_t(id));
  if (!param) {
    return false;
  }
  value = param->valueAt(synth->currentTime());
  return true;
}

bool S2WClapPluginBase::paramValueText(clap_id id, double value, char* text, uint32_t size) const
{
  std::string strValue;
  if (id == 'FNAM') {
    if (value < 0.5) {
      if (filePath.empty()) {
        strValue = "(No File Loaded)";
      } else {
        strValue = filePath;
      }
    } else {
      strValue = "(Load New...)";
    }
  } else if (id == 'inst') {
    std::lock_guard lock(synthMutex);
    strValue = std::to_string(synth->instrumentID(uint64_t(value)));
  } else {
    strValue = std::to_string(value);
  }
  std::strncpy(text, strValue.data(), size);
  return true;
}

bool S2WClapPluginBase::paramTextValue(clap_id id, const char* text, double& value) const
{
  if (id == 'FNAM') {
    return false;
  } else if (id == 'inst') {
    std::lock_guard lock(synthMutex);
    uint64_t instrumentID = 0;
#if ULONG_MAX == UINT64_MAX
    int success = std::sscanf(text, " %lu", &instrumentID);
#else
    int success = std::sscanf(text, " %llu", &instrumentID);
#endif
    if (!success) {
      return false;
    }
    int numInsts = synth->numInstruments();
    for (int i = 0; i < numInsts; i++) {
      if (synth->instrumentID(i) == instrumentID) {
        value = i;
        return true;
      }
    }
    return false;
  }
  int success = std::sscanf(text, " %lf", &value);
  return success;
}

void S2WClapPluginBase::flushParams(const clap_input_events_t* inEvents, const clap_output_events_t* outEvents)
{
}

IInstrument* S2WClapPluginBase::selectInstrumentByIndex(uint32_t index, bool force, bool rescan)
{
  if (index >= synth->numInstruments()) {
    std::cerr << "instrument index out of range: " << index << std::endl;
    return nullptr;
  }
  return selectInstrumentByID(synth->instrumentID(index), force, rescan);
}

IInstrument* S2WClapPluginBase::selectInstrumentByID(uint64_t instrumentID, bool force, bool rescan)
{
  if (!force && instrumentID == currentInstID) {
    if (rescan) {
      requestParamSync(false);
    }
    return instrument;
  }
  IInstrument* found = synth->getInstrument(instrumentID);
  if (!found) {
    std::cerr << "instrument not found: " << instrumentID << std::endl;
    return nullptr;
  }

  instrument = found;
  chanParams.clear();
  noteParams.clear();
  chanParams.insert('inst');
  paramOrder = { 'FNAM', 'inst' };

  for (uint32_t param : instrument->supportedChannelParams()) {
    if (!chanParams.count(param)) {
      paramOrder.push_back(param);
      chanParams.insert(param);
    }
  }

  for (uint32_t param : instrument->supportedChannelParams()) {
    if (!noteParams.count(param)) {
      if (!chanParams.count(param)) {
        paramOrder.push_back(param);
      }
      noteParams.insert(param);
    }
  }

  currentInstID = instrumentID;
  if (rescan) {
    requestParamSync(true);
  }
  return found;
}

void S2WClapPluginBase::requestParamSync(bool rescanInfo)
{
  if (rescanInfo) {
    mustRescanInfo = rescanInfo;
  }
  if (std::this_thread::get_id() == mainThreadID) {
    onMainThread();
  } else if (!rescanInfo) {
    queueRescanValues = true;
  } else {
    host->request_callback(host);
  }
}

bool S2WClapPluginBase::saveState(const clap_ostream_t* stream)
{
  std::lock_guard lock(synthMutex);
  if (filePath.empty() || !instrument || synth->channels.empty()) {
    // No state to save
    return true;
  }

  std::ostringstream ss;
  ss << filePath << std::endl;
  ss << currentInstID << std::endl;
  ss << chanParams.size();
  ss << std::hexfloat;
  double now = synth->currentTime();
  for (uint32_t p : chanParams) {
    if (p == 'inst') {
      // This was serialized separately
      continue;
    }
    ss << " " << fourccToString(p) << " " << synth->channels[0]->paramValue(p, now);
  }

  std::string state = ss.str();
  std::cerr << state << std::endl;
  return stream->write(stream, state.c_str(), state.size()) == state.size();
}

bool S2WClapPluginBase::loadState(const clap_istream_t* stream)
{
  std::string state;
  char buffer[1024];
  int bytesRead = 0;
  do {
    bytesRead = stream->read(stream, buffer, 1024);
    state.append(buffer, bytesRead);
  } while (bytesRead > 0);

  std::istringstream ss(state);
  std::getline(ss, filePath);
  std::cerr << filePath << std::endl;
  uint64_t instID = 0;
  ss >> instID;
  std::cerr << "instID=" << instID << std::endl;
  int numParams = 0;
  ss >> numParams;
  std::cerr << "numParams=" << numParams << std::endl;

  {
    std::lock_guard lock(synthMutex);
    seq.reset();
    delete synth;
    std::ifstream file(filePath);
    synth = createContext(ctx, filePath, file);
    synth->addChannel(&seq);
    IInstrument* found = selectInstrumentByID(instID, true, false);
    if (!found) {
      requestParamSync(true);
      return false;
    }

    char fourcc[5] = { '\0', '\0', '\0', '\0', '\0' };
    uint32_t paramID;
    std::string strValue;
    double value;
    for (int i = 0; i < numParams; i++) {
      ss.ignore(1, ' ');
      ss.read(fourcc, 4);
      paramID = parseIntBE<uint32_t>(fourcc, 0);
      ss >> strValue;
      value = std::stod(strValue);
      std::cerr << fourcc << "\t" << paramID << "\t" << value << std::endl;
      if (paramID != 'inst') {
        ChannelEvent* ch = new ChannelEvent(paramID, value);
        seq.addEvent(ch);
      }
    }
  }

  requestParamSync(true);
  return true;
}
#endif

#ifdef BUILD_CLAP
#include "plugin/clapplugin.h"
#include "synth/channel.h"
#include <fstream>

#define C_P const clap_plugin_t* plugin
#define WRAP_METHOD(RET, method, sig, ...) []sig -> RET { return reinterpret_cast<S2WClapPluginBase*>(plugin->plugin_data)->method(__VA_ARGS__); }
#define WRAP_METHOD_VOID(method, sig, ...) []sig { reinterpret_cast<S2WClapPluginBase*>(plugin->plugin_data)->method(__VA_ARGS__); }
S2WClapPluginBase::S2WClapPluginBase(const clap_host_t* host)
: host(host), ctx(new S2WContext(true)), synth(nullptr)
{
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
}

S2WClapPluginBase::~S2WClapPluginBase()
{
  if (synth) {
    delete synth;
  }
}

bool S2WClapPluginBase::init() {
  return true;
}

void S2WClapPluginBase::destroy() {
  if (synth) {
    delete synth;
    synth = nullptr;
  }
  delete this;
}

bool S2WClapPluginBase::activate(double sampleRate, uint32_t minFrames, uint32_t maxFrames) {
  std::cerr << "activate - " << (synth ? "has synth" : "creating synth") << std::endl;
  if (!synth) {
    std::ifstream file("/home/coda/dse2wav/B_EVENT_BOSS_02.smd");
    synth = createContext(ctx, "/home/coda/dse2wav/B_EVENT_BOSS_02.smd", file);
    synth->addChannel(&seq);
    seq.addEvent(new ChannelEvent('inst', uint64_t(95)));
    seq.sync();
  }
  synth->setSampleRate(sampleRate);
  return true;
}

void S2WClapPluginBase::deactivate() {
}

bool S2WClapPluginBase::startProcessing() {
  return true;
}

void S2WClapPluginBase::stopProcessing() {
}

void S2WClapPluginBase::reset() {
  if (!synth) {
    return;
  }
  double sampleRate = synth->sampleRate;
  deactivate();
  activate(sampleRate, 0, 0);
}

clap_process_status S2WClapPluginBase::process(const clap_process_t* process) {
  if (!synth) {
    return CLAP_PROCESS_CONTINUE;
  }

  steadyTime = process->steady_time;
  try {
    uint32_t numEvents = process->in_events->size(process->in_events);

    for (uint32_t i = 0; i < numEvents; i++) {
      const clap_event_header_t *event = process->in_events->get(process->in_events, i);
      dispatchEvent(event);
    }

    if (numEvents > 0) {
      seq.sync();
    }

    float* buffers[2] = {
      process->audio_outputs[0].data32[0],
      process->audio_outputs[0].data32[1],
    };
    synth->fillBuffers(buffers, process->frames_count);

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

void S2WClapPluginBase::dispatchEvent(const clap_event_header_t* event) {
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

const void* S2WClapPluginBase::getExtension(const char* id) {
  if (!strcmp(id, CLAP_EXT_NOTE_PORTS)) {
    return &notePorts;
  } else if (!strcmp(id, CLAP_EXT_AUDIO_PORTS)) {
    return &audioPorts;
  }
  return nullptr;
}

void S2WClapPluginBase::onMainThread() {
}

BaseNoteEvent* S2WClapPluginBase::createNoteEvent(const clap_event_note_t* event)
{
  InstrumentNoteEvent* note = new InstrumentNoteEvent();
  //OscillatorEvent* note = new OscillatorEvent();
  note->pitch = event->key;
  //note->frequency = noteToFreq(event->key);
  note->volume = event->velocity;
  return note;
}

void S2WClapPluginBase::noteEvent(const clap_event_note_t* event)
{
  if (event->header.type == CLAP_EVENT_NOTE_ON) {
    BaseNoteEvent* note = createNoteEvent(event);
    note->timestamp = eventTimestamp(event);
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
  if (event->note_id >= 0) {
    ModulatorEvent* mod = new ModulatorEvent(event->note_id, event->param_id, event->value);
    mod->timestamp = eventTimestamp(event);
    seq.addEvent(mod);
  } else if (event->key >= 0) {
    const auto& chan = synth->channels[0];
    // TODO: Modulating a note that started this process interval will fail this lookup.
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

#endif

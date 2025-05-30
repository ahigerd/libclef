#ifndef CLEF_SEQUENCEEVENT_H
#define CLEF_SEQUENCEEVENT_H

#include <cstdint>
#include <vector>
#include <memory>
#include "synth/audionode.h"

class BaseNoteEvent;

class IEventID {
  template <class T, int ID>
  friend class EventID;

public:
  inline int eventType() const { return m_eventType; }
  inline bool isNoteEvent() const { return m_isNote; }

protected:
  IEventID() : m_eventType(-1), m_isNote(false) {}

private:
  int m_eventType;
  bool m_isNote;
};

class SequenceEvent : virtual public IEventID {
public:
  virtual ~SequenceEvent() {}

  enum EventTypes {
    Sample,
    Oscillator,
    InstrumentNote,
    AudioNode,
    Channel,
    Modulator,
    Kill,
    UserBase,
  };

  double timestamp;

  template<typename T>
  inline const T* cast() const {
    if (eventType() == T::TypeID) {
      return static_cast<const T*>(this);
    }
    return nullptr;
  }

  template<typename T>
  inline T* cast() {
    if (eventType() == T::TypeID) {
      return static_cast<T*>(this);
    }
    return nullptr;
  }

protected:
  SequenceEvent();
};

template <class THIS, int TYPE_ID>
class EventID : virtual public IEventID {
public:
  enum { TypeID = TYPE_ID };

  template <typename SE>
  static inline std::shared_ptr<THIS> castShared(std::shared_ptr<SE>& event) {
    if (event->eventType() == TypeID) {
      return std::static_pointer_cast<THIS>(event);
    }
    return nullptr;
  }

protected:
  EventID() {
    m_eventType = TYPE_ID;
    m_isNote = std::is_base_of<BaseNoteEvent, THIS>::value;
  }
};

struct BaseNoteEvent : public SequenceEvent {
  static uint64_t nextPlaybackID();
  BaseNoteEvent();

  uint64_t playbackID;  // for modulation
  double duration;
  double volume;
  double pan;

  bool useEnvelope : 1;
  bool expAttack : 1;
  bool expDecay : 1;
  double startGain, attack, hold, decay, sustain, fade, release;
  void setEnvelope(double attack, double decay, double sustain, double release);
  void setEnvelope(double attack, double hold, double decay, double sustain, double release);
  void setEnvelope(double start, double attack, double hold, double decay, double sustain, double release);
  void setEnvelope(double start, double attack, double hold, double decay, double sustain, double fade, double release);

  template <typename SE>
  static inline std::shared_ptr<BaseNoteEvent> castShared(std::shared_ptr<SE>& event) {
    if (event->isNoteEvent()) {
      return std::static_pointer_cast<BaseNoteEvent>(event);
    }
    return nullptr;
  }
};

template <>
inline const BaseNoteEvent* SequenceEvent::cast<BaseNoteEvent>() const
{
  if (isNoteEvent()) {
    return static_cast<const BaseNoteEvent*>(this);
  }
  return nullptr;
}

template <>
inline BaseNoteEvent* SequenceEvent::cast<BaseNoteEvent>()
{
  if (isNoteEvent()) {
    return static_cast<BaseNoteEvent*>(this);
  }
  return nullptr;
}

class SampleEvent : public BaseNoteEvent, public EventID<SampleEvent, SequenceEvent::Sample> {
public:
  SampleEvent();

  uint64_t sampleID;    // looked up in sound bank
  double pitchBend;
};

class OscillatorEvent : public BaseNoteEvent, public EventID<OscillatorEvent, SequenceEvent::Oscillator> {
public:
  OscillatorEvent();

  uint64_t waveformID;  // passed to PSG
  double frequency;
};

class InstrumentNoteEvent : public BaseNoteEvent, public EventID<InstrumentNoteEvent, SequenceEvent::InstrumentNote> {
public:
  InstrumentNoteEvent();

  double pitch;
  std::vector<double> floatParams;
  std::vector<uint64_t> intParams;
};

class AudioNodeEvent : public BaseNoteEvent, public EventID<AudioNodeEvent, SequenceEvent::AudioNode> {
public:
  AudioNodeEvent(std::shared_ptr<::AudioNode> node);

  std::shared_ptr<::AudioNode> node;
};

class ModulatorEvent : public SequenceEvent, public EventID<ModulatorEvent, SequenceEvent::Modulator> {
public:
  ModulatorEvent(uint64_t playbackID, int32_t param, double value);
  ModulatorEvent(int32_t param, double value);

  uint64_t playbackID;
  int32_t param;
  double value;
};

class KillEvent : public SequenceEvent, public EventID<KillEvent, SequenceEvent::Kill> {
public:
  KillEvent(uint64_t playbackID, double timestamp);

  uint64_t playbackID;
  bool immediate;
};

class ChannelEvent : public SequenceEvent, public EventID<ChannelEvent, SequenceEvent::Channel> {
public:
  ChannelEvent(uint32_t param, double value);
  ChannelEvent(uint32_t param, uint64_t intValue);

  uint32_t param;
  union {
    double value;
    uint64_t intValue;
  };
};

#endif

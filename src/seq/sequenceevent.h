#ifndef S2W_SEQUENCEEVENT_H
#define S2W_SEQUENCEEVENT_H

#include <cstdint>
#include <vector>
#include <memory>
#include "synth/audionode.h"

class SequenceEvent {
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

  virtual int eventType() const = 0;
  virtual bool isNoteEvent() const;

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

  template<typename T, typename SE>
  static inline std::shared_ptr<T> castShared(std::shared_ptr<SE>& event) {
    if (event->eventType() == T::TypeID) {
      return std::static_pointer_cast<T>(event);
    }
    return nullptr;
  }
};

template <class THIS, int TYPE_ID>
class BaseEvent : public SequenceEvent {
public:
  enum { TypeID = TYPE_ID };

  virtual int eventType() const { return TypeID; }

  template <typename SE>
  static inline std::shared_ptr<THIS> castShared(std::shared_ptr<SE>& event) {
    return SequenceEvent::castShared<THIS>(event);
  }
};

struct BaseNoteEvent : public SequenceEvent {
  static uint64_t nextPlaybackID();
  BaseNoteEvent();

  virtual bool isNoteEvent() const;

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

template <class THIS, int TYPE_ID>
class NoteEvent : public BaseNoteEvent {
public:
  enum { TypeID = TYPE_ID };

  virtual int eventType() const { return TypeID; }

  template <typename SE>
  static inline std::shared_ptr<THIS> castShared(std::shared_ptr<SE>& event) {
    return SequenceEvent::castShared<THIS>(event);
  }
};

class SampleEvent : public NoteEvent<SampleEvent, SequenceEvent::Sample> {
public:
  SampleEvent();

  uint64_t sampleID;    // looked up in sound bank
  double pitchBend;
};

class OscillatorEvent : public NoteEvent<OscillatorEvent, SequenceEvent::Oscillator> {
public:
  OscillatorEvent();

  uint64_t waveformID;  // passed to PSG
  double frequency;
};

class InstrumentNoteEvent : public NoteEvent<InstrumentNoteEvent, SequenceEvent::InstrumentNote> {
public:
  InstrumentNoteEvent();

  double pitch;
  std::vector<double> floatParams;
  std::vector<uint64_t> intParams;
};

class AudioNodeEvent : public NoteEvent<AudioNodeEvent, SequenceEvent::AudioNode> {
public:
  AudioNodeEvent(std::shared_ptr<::AudioNode> node);

  std::shared_ptr<::AudioNode> node;
};

class ModulatorEvent : public BaseEvent<ModulatorEvent, SequenceEvent::Modulator> {
public:
  ModulatorEvent(uint64_t playbackID, int32_t param, double value);

  uint64_t playbackID;
  int32_t param;
  double value;
};

class KillEvent : public BaseEvent<KillEvent, SequenceEvent::Kill> {
public:
  KillEvent(uint64_t playbackID, double timestamp);

  uint64_t playbackID;
  bool immediate;
};

class ChannelEvent : public BaseEvent<ChannelEvent, SequenceEvent::Channel> {
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

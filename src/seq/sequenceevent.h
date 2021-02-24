#ifndef S2W_SEQUENCEEVENT_H
#define S2W_SEQUENCEEVENT_H

#include <cstdint>
struct IModulator;

class SequenceEvent {
public:
  virtual ~SequenceEvent() {}

  enum EventTypes {
    Sample,
    Oscillator,
    Modulator,
    Kill,
    Channel,
    UserBase,
  };

  double timestamp;

  virtual int eventType() const = 0;

  template<typename T>
  const T* cast() const {
    if (eventType() == T::TypeID) {
      return static_cast<const T*>(this);
    }
    return nullptr;
  }

  template<typename T>
  T* cast() {
    if (eventType() == T::TypeID) {
      return static_cast<T*>(this);
    }
    return nullptr;
  }
};

template <int TYPE_ID>
class BaseEvent : public SequenceEvent {
public:
  enum { TypeID = TYPE_ID };

  virtual int eventType() const { return TypeID; }
};

struct BaseNoteEvent {
  static uint64_t nextPlaybackID();
  BaseNoteEvent();

  uint64_t playbackID;  // for modulation
  double duration;
  double volume;
  double pan;

  bool useEnvelope : 1;
  bool expAttack : 1;
  bool expDecay : 1;
  double startGain, attack, hold, decay, sustain, release;
  void setEnvelope(double attack, double decay, double sustain, double release);
  void setEnvelope(double attack, double hold, double decay, double sustain, double release);
  void setEnvelope(double start, double attack, double hold, double decay, double sustain, double release);
};

template <int TYPE_ID>
class NoteEvent : public BaseEvent<TYPE_ID>, public BaseNoteEvent {
public:
};

class SampleEvent : public NoteEvent<SequenceEvent::Sample> {
public:
  SampleEvent();

  uint64_t sampleID;    // looked up in sound bank
  double pitchBend;
};

class OscillatorEvent : public NoteEvent<SequenceEvent::Oscillator> {
public:
  OscillatorEvent();

  uint64_t waveformID;  // passed to PSG
  double frequency;
};

class ModulatorEvent : public BaseEvent<SequenceEvent::Modulator> {
public:
  uint64_t playbackID;
  IModulator* modulator;
};

class KillEvent : public BaseEvent<SequenceEvent::Kill> {
public:
  KillEvent(uint64_t playbackID, double timestamp);

  uint64_t playbackID;
  bool immediate;
};

class ChannelEvent : public BaseEvent<SequenceEvent::Channel> {
public:
  ChannelEvent(uint32_t param, double value);

  uint32_t param;
  double value;
};

#endif

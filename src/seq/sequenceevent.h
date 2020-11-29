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

  bool useEnvelope;
  double startGain, attack, hold, sustain, decay, release;
  void setEnvelope(double attack, double sustain, double decay, double release);
  void setEnvelope(double attack, double hold, double sustain, double decay, double release);
  void setEnvelope(double start, double attack, double hold, double sustain, double decay, double release);
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

#endif

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

class SampleEvent : public BaseEvent<SequenceEvent::Sample> {
public:
  static uint64_t nextPlaybackID();
  SampleEvent();

  uint64_t sampleID;    // looked up in sound bank
  uint64_t playbackID;  // for modulation
  double duration;
  double sampleRate;
  double volume;
  double pan;
};

class OscillatorEvent : public BaseEvent<SequenceEvent::Oscillator> {
public:
  static uint64_t nextPlaybackID();
  OscillatorEvent();

  uint64_t waveformID;  // passed to PSG
  uint64_t playbackID;  // for modulation
  double duration;
  double frequency;
  double volume;
  double pan;
};

class ModulatorEvent : public BaseEvent<SequenceEvent::Modulator> {
public:
  uint64_t playbackID;
  IModulator* modulator;
};

#endif

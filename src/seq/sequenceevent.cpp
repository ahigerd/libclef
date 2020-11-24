#include "sequenceevent.h"

static uint64_t _nextPlaybackID = 0;

inline uint64_t SampleEvent::nextPlaybackID()
{
  return _nextPlaybackID++;
}

SampleEvent::SampleEvent() : playbackID(nextPlaybackID()), duration(-1), sampleRate(-1), volume(1.0), pan(0.5)
{
  // initializers only
}

uint64_t OscillatorEvent::nextPlaybackID()
{
  return SampleEvent::nextPlaybackID();
}

OscillatorEvent::OscillatorEvent() : playbackID(nextPlaybackID()), volume(1.0), pan(0.5)
{
  // initializers only
}

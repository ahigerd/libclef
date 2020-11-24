#include "sequenceevent.h"

static uint64_t _nextPlaybackID = 0;

inline uint64_t SampleEvent::nextPlaybackID()
{
  return _nextPlaybackID++;
}

SampleEvent::SampleEvent() : playbackID(nextPlaybackID())
{
  // initializers only
}

uint64_t OscillatorEvent::nextPlaybackID()
{
  return SampleEvent::nextPlaybackID();
}

OscillatorEvent::OscillatorEvent() : playbackID(nextPlaybackID())
{
  // initializers only
}

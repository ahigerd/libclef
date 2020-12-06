#include "sequenceevent.h"

static uint64_t _nextPlaybackID = 1;

inline uint64_t BaseNoteEvent::nextPlaybackID()
{
  return _nextPlaybackID++;
}

BaseNoteEvent::BaseNoteEvent()
: playbackID(nextPlaybackID()), duration(-1), volume(1.0), pan(0.5), useEnvelope(false)
{
  // initializers only
}

void BaseNoteEvent::setEnvelope(double attack, double sustain, double decay, double release)
{
  useEnvelope = true;
  startGain = 0.0;
  hold = 0.0;
  this->attack = attack;
  this->sustain = sustain;
  this->decay = decay;
  this->release = release;
}

void BaseNoteEvent::setEnvelope(double attack, double hold, double sustain, double decay, double release)
{
  useEnvelope = true;
  startGain = 0.0;
  this->attack = attack;
  this->hold = hold;
  this->sustain = sustain;
  this->decay = decay;
  this->release = release;
}

void BaseNoteEvent::setEnvelope(double start, double attack, double hold, double sustain, double decay, double release)
{
  useEnvelope = true;
  this->startGain = start;
  this->attack = attack;
  this->hold = hold;
  this->sustain = sustain;
  this->decay = decay;
  this->release = release;
}

SampleEvent::SampleEvent()
: pitchBend(1.0)
{
  // initializers only
}

OscillatorEvent::OscillatorEvent()
: frequency(440.0)
{
  // initializers only
}

KillEvent::KillEvent(uint64_t playbackID, double timestamp)
: playbackID(playbackID)
{
  this->timestamp = timestamp;
}

#include "sequenceevent.h"
#include <iostream>
#include <cmath>
#include <atomic>

static std::atomic_uint64_t _nextPlaybackID = 1;

SequenceEvent::SequenceEvent()
: timestamp(0)
{
  // initializers only
}

bool SequenceEvent::isNoteEvent() const
{
  return false;
}

bool BaseNoteEvent::isNoteEvent() const
{
  return true;
}

inline uint64_t BaseNoteEvent::nextPlaybackID()
{
  return _nextPlaybackID.fetch_add(1, std::memory_order_relaxed);
}

BaseNoteEvent::BaseNoteEvent()
: playbackID(nextPlaybackID()), duration(-1), volume(1.0), pan(0.5), useEnvelope(false),
  expAttack(false), expDecay(false), attack(0), hold(0), decay(0), sustain(1), fade(HUGE_VAL), release(0)
{
  // initializers only
}

void BaseNoteEvent::setEnvelope(double attack, double decay, double sustain, double release)
{
  useEnvelope = true;
  startGain = 0.0;
  hold = 0.0;
  this->attack = attack;
  this->decay = decay;
  this->sustain = sustain;
  this->fade = HUGE_VAL;
  this->release = release;
}

void BaseNoteEvent::setEnvelope(double attack, double hold, double decay, double sustain, double release)
{
  useEnvelope = true;
  startGain = 0.0;
  this->attack = attack;
  this->hold = hold;
  this->decay = decay;
  this->sustain = sustain;
  this->fade = HUGE_VAL;
  this->release = release;
}

void BaseNoteEvent::setEnvelope(double start, double attack, double hold, double decay, double sustain, double release)
{
  useEnvelope = true;
  this->startGain = start;
  this->attack = attack;
  this->hold = hold;
  this->decay = decay;
  this->sustain = sustain;
  this->fade = HUGE_VAL;
  this->release = release;
}

void BaseNoteEvent::setEnvelope(double start, double attack, double hold, double decay, double sustain, double fade, double release)
{
  useEnvelope = true;
  this->startGain = start;
  this->attack = attack;
  this->hold = hold;
  this->decay = decay;
  this->sustain = sustain;
  this->fade = fade;
  this->release = release;
}

SampleEvent::SampleEvent()
: sampleID(0xFFFFFFFFFFFFFFFFULL), pitchBend(1.0)
{
  // initializers only
}

OscillatorEvent::OscillatorEvent()
: waveformID(0), frequency(440.0)
{
  // initializers only
}

InstrumentNoteEvent::InstrumentNoteEvent()
: pitch(69)
{
  // initializers only
}

AudioNodeEvent::AudioNodeEvent(std::shared_ptr<::AudioNode> node)
: node(node)
{
  // initializers only
}

ModulatorEvent::ModulatorEvent(uint64_t playbackID, int32_t param, double value)
: playbackID(playbackID), param(param), value(value)
{
  // initializers only
}

ModulatorEvent::ModulatorEvent(int32_t param, double value)
: playbackID(0), param(param), value(value)
{
  // initializers only
}

KillEvent::KillEvent(uint64_t playbackID, double timestamp)
: playbackID(playbackID), immediate(false)
{
  this->timestamp = timestamp;
}

ChannelEvent::ChannelEvent(uint32_t param, double value)
: param(param), value(value)
{
  // initializers only
}

ChannelEvent::ChannelEvent(uint32_t param, uint64_t intValue)
: param(param), intValue(intValue)
{
  // initializers only
}

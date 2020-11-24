#include "channel.h"
#include "oscillator.h"
#include "sampler.h"
#include "../seq/itrack.h"
#include "../seq/sequenceevent.h"
#include <iostream>

Channel::Note::Note(SequenceEvent* event, AudioNode* source, double duration)
: event(event), source(source), duration(duration)
{
  // initializers only
}

Channel::Channel(SynthContext* ctx, ITrack* track)
: gain(0.5), ctx(ctx), track(track), nextEvent(nullptr)
{
  timestamp = 0;
}

Channel::~Channel()
{
}

uint32_t Channel::fillBuffer(std::vector<int16_t>& buffer)
{
  int numSamples = buffer.size();
  double sampleTime = 1.0 / 44100.0;
  double endTime = timestamp + (numSamples * sampleTime);
  SequenceEvent* event;
  do {
    if (nextEvent) {
      event = nextEvent;
      nextEvent = nullptr;
    } else {
      event = track->nextEvent();
    }
    if (event) {
      if (event->timestamp >= endTime) {
        nextEvent = event;
        break;
      }
      if (OscillatorEvent* oscEvent = event->cast<OscillatorEvent>()) {
        BaseOscillator* osc = BaseOscillator::create(oscEvent->waveformID, oscEvent->frequency, oscEvent->volume);
        notes.emplace(std::make_pair(oscEvent->playbackID, new Note(event, osc, oscEvent->duration)));
      } else if (SampleEvent* sampEvent = event->cast<SampleEvent>()) {
        SampleData* sampleData = SampleData::get(sampEvent->sampleID);
        Sampler* samp = new Sampler(sampleData, sampEvent->sampleRate <= 0 ? sampleData->sampleRate : sampEvent->sampleRate);
        samp->gain = sampEvent->volume;
        notes.emplace(std::make_pair(sampEvent->playbackID, new Note(event, samp, sampleData->duration(samp->sampleRate))));
      }
    }
  } while (event);
  int pos = 0;
  while (pos < numSamples && !isFinished()) {
    int32_t sample = 0;
    std::vector<uint64_t> toErase;
    for (auto& note : notes) {
      double start = note.second->event->timestamp;
      if (start + note.second->duration < timestamp) {
        toErase.push_back(note.first);
      } else if (start <= timestamp) {
        // TODO: modulators
        sample += note.second->source->getSample(timestamp - start);
      }
    }
    for (uint64_t id : toErase) {
      notes.erase(id);
    }
    buffer[pos] = sample * gain;
    ++pos;
    timestamp += sampleTime;
  }
  return pos;
}

bool Channel::isFinished() const
{
  return !nextEvent && !notes.size() && track->isFinished();
}

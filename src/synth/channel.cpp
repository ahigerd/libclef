#include "channel.h"
#include "oscillator.h"
#include "sampler.h"
#include "../seq/itrack.h"
#include "../seq/sequenceevent.h"

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
  int numSamples = buffer.size() / 2;
  double sampleTime = 1.0 / 44100.0; // TODO: ctx
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
      AudioNode* noteNode = nullptr;
      BaseNoteEvent* noteEvent = nullptr;
      Note* note = nullptr;
      if (OscillatorEvent* oscEvent = event->cast<OscillatorEvent>()) {
        noteEvent = oscEvent;
        BaseOscillator* osc = BaseOscillator::create(oscEvent->waveformID, oscEvent->frequency, oscEvent->volume, oscEvent->pan);
        note = new Note(event, osc, oscEvent->duration);
        notes.emplace(std::make_pair(oscEvent->playbackID, note));
        noteNode = osc;
      } else if (SampleEvent* sampEvent = event->cast<SampleEvent>()) {
        noteEvent = sampEvent;
        SampleData* sampleData = SampleData::get(sampEvent->sampleID);
        Sampler* samp = new Sampler(sampleData, sampEvent->pitchBend);
        samp->gain = sampEvent->volume;
        samp->pan = sampEvent->pan;
        note = new Note(event, samp, sampleData->duration());
        notes.emplace(std::make_pair(sampEvent->playbackID, note));
        noteNode = samp;
      }
      if (noteEvent && noteEvent->useEnvelope) {
        Envelope* env = new Envelope(noteEvent->attack, noteEvent->hold, noteEvent->sustain, noteEvent->decay, noteEvent->release);
        env->startGain = noteEvent->startGain;
        noteNode->gain.connect(env);
        note->envelope.reset(env);
      }
    }
  } while (event);
  int pos = 0;
  while (pos < buffer.size() && !isFinished()) {
    for (int ch = 0; ch < 2; ch++) {
      int32_t sample = 0;
      std::vector<uint64_t> toErase;
      for (auto& note : notes) {
        double start = note.second->event->timestamp;
        bool stop = false;
        if (!note.second->source->isActive()) {
          stop = true;
        } else if (start + note.second->duration < timestamp) {
          if (note.second->envelope && note.second->envelope->isActive()) {
            note.second->envelope->trigger = 0;
          } else {
            stop = true;
          }
        }
        if (stop) {
          toErase.push_back(note.first);
        } else if (start <= timestamp) {
          // TODO: modulators
          sample += note.second->source->getSample(timestamp - start, ch);
        }
      }
      for (uint64_t id : toErase) {
        notes.erase(id);
      }
      buffer[pos] = sample * gain;
      ++pos;
    }
    timestamp += sampleTime;
  }
  return pos;
}

bool Channel::isFinished() const
{
  return !nextEvent && !notes.size() && track->isFinished();
}

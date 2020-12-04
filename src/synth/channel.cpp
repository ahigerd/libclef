#include "channel.h"
#include "oscillator.h"
#include "sampler.h"
#include "synthcontext.h"
#include "seq/itrack.h"
#include "seq/sequenceevent.h"
#include <iostream>

Channel::Note::Note(std::shared_ptr<SequenceEvent> event, std::shared_ptr<AudioNode> source, double duration)
: event(event), source(source), duration(duration)
{
  // initializers only
}

Channel::Channel(const SynthContext* ctx, ITrack* track)
: gain(0.5), ctx(ctx), track(track), nextEvent(nullptr)
{
  timestamp = 0;
}

Channel::~Channel()
{
}

uint32_t Channel::fillBuffer(std::vector<int16_t>& buffer, ssize_t numSamples)
{
  if (numSamples < 0 || numSamples * 2 > buffer.size()) {
    numSamples = buffer.size() / 2;
  }
  double endTime = timestamp + (numSamples * ctx->sampleTime);
  std::shared_ptr<SequenceEvent> event;
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
      std::shared_ptr<AudioNode> noteNode = nullptr;
      BaseNoteEvent* noteEvent = nullptr;
      Note* note = nullptr;
      if (OscillatorEvent* oscEvent = event->cast<OscillatorEvent>()) {
        noteEvent = oscEvent;
        BaseOscillator* osc = BaseOscillator::create(ctx, oscEvent->waveformID, oscEvent->frequency, oscEvent->volume, oscEvent->pan);
        noteNode.reset(osc);
        note = new Note(event, noteNode, oscEvent->duration);
        notes.emplace(std::make_pair(oscEvent->playbackID, note));
      } else if (SampleEvent* sampEvent = event->cast<SampleEvent>()) {
        noteEvent = sampEvent;
        SampleData* sampleData = SampleData::get(sampEvent->sampleID);
        if (!sampleData) {
          std::cerr << "ERROR: sample " << std::hex << sampEvent->sampleID << std::dec << " not found" << std::endl;
          continue;
        }
        Sampler* samp = new Sampler(ctx, sampleData, sampEvent->pitchBend);
        noteNode.reset(samp);
        samp->param(AudioNode::Gain)->setConstant(sampEvent->volume);
        samp->param(AudioNode::Pan)->setConstant(sampEvent->pan);
        note = new Note(event, noteNode, sampEvent->duration >= 0 ? sampEvent->duration : sampleData->duration());
        notes.emplace(std::make_pair(sampEvent->playbackID, note));
      } else if (KillEvent* killEvent = event->cast<KillEvent>()) {
        auto noteIter = notes.find(killEvent->playbackID);
        if (noteIter != notes.end()) {
          noteIter->second->duration = killEvent->timestamp - noteIter->second->event->timestamp;
        }
      }
      if (noteEvent && noteEvent->useEnvelope) {
        Envelope* env = new Envelope(ctx, noteEvent->attack, noteEvent->hold, noteEvent->sustain, noteEvent->decay, noteEvent->release);
        env->param(Envelope::StartGain)->setConstant(noteEvent->startGain);
        env->connect(noteNode);
        noteNode.reset(env);
        note->source = noteNode;
      }
    }
  } while (event);
  int pos = 0;
  while (pos < buffer.size() && !isFinished()) {
    for (int ch = 0; ch < ctx->outputChannels; ch++) {
      int32_t sample = 0;
      std::vector<uint64_t> toErase;
      for (auto& iter : notes) {
        uint64_t noteID = iter.first;
        auto& note = iter.second;
        double start = note->event->timestamp;
        bool stop = false;
        if (!note->source->isActive()) {
          stop = true;
        } else if (start + note->duration < timestamp) {
          auto trigger = note->source->param(AudioNode::Trigger);
          if (trigger) {
            if (trigger->valueAt(timestamp - start)) {
              trigger->setConstant(0);
            }
            stop = !note->source->isActive();
          } else {
            stop = true;
          }
        }
        if (stop) {
          toErase.push_back(noteID);
        } else if (start <= timestamp) {
          // TODO: modulators
          // TODO: mixdown if source is stereo and output is mono
          sample += note->source->getSample(timestamp - start, ch);
        }
      }
      for (uint64_t id : toErase) {
        notes.erase(id);
      }
      buffer[pos] = sample * gain;
      ++pos;
    }
    timestamp += ctx->sampleTime;
  }
  return pos;
}

bool Channel::isFinished() const
{
  return !nextEvent && !notes.size() && track->isFinished();
}

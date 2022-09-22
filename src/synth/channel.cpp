#include "channel.h"
#include "synthcontext.h"
#include "s2wcontext.h"
#include "iinstrument.h"
#include "seq/itrack.h"
#include "seq/sequenceevent.h"
#include <iostream>

Channel::Note::Note()
: event(nullptr), source(nullptr), duration(0), kill(true)
{
  // initializers only
}

Channel::Note::Note(std::shared_ptr<BaseNoteEvent> event, std::shared_ptr<AudioNode> source, double duration)
: event(event), source(source), duration(duration), kill(false)
{
  // initializers only
}

Channel::Channel(const SynthContext* ctx, ITrack* track)
: AudioParamContainer(ctx), track(track), nextEvent(nullptr)
{
  instrument = ctx->defaultInstrument();
  gain = addParam(AudioNode::Gain, 0.5).get();
  pan = addParam(AudioNode::Pan, 0.5).get();
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
      if (auto chEvent = ChannelEvent::castShared(event)) {
        instrument->channelEvent(this, chEvent);
      } else if (auto modEvent = ModulatorEvent::castShared(event)) {
        instrument->modulatorEvent(this, modEvent);
      } else if (auto noteEvent = BaseNoteEvent::castShared(event)) {
        Note* note = instrument->noteEvent(this, noteEvent);
        if (note && !note->kill) {
          notes.emplace(std::make_pair(note->event->playbackID, note));
        }
      } else if (event->eventType() >= SequenceEvent::UserBase) {
        // This must go after the BaseNoteEvent case because user-defined
        // note events should be sent there.
        Note* note = instrument->userEvent(this, event);
        if (note && !note->kill) {
          notes.emplace(std::make_pair(note->event->playbackID, note));
        }
      } else if (KillEvent* killEvent = event->cast<KillEvent>()) {
        auto noteIter = notes.find(killEvent->playbackID);
        if (noteIter != notes.end()) {
          noteIter->second->duration = killEvent->timestamp - noteIter->second->event->timestamp;
          noteIter->second->kill = noteIter->second->kill || killEvent->immediate;
        }
      }
    }
  } while (event);
  int pos = 0;
  while (pos < buffer.size() && !isFinished()) {
    double panValue = ctx->outputChannels > 1 ? pan->valueAt(timestamp) : 1;
    for (int ch = 0; ch < ctx->outputChannels; ch++) {
      int32_t sample = 0;
      std::vector<uint64_t> toErase;
      for (auto& iter : notes) {
        uint64_t noteID = iter.first;
        auto& note = iter.second;
        double start = note->event->timestamp;
        bool stop = false;
        if (!note->source->isActive() || note->kill) {
          stop = true;
        } else if (note->duration > 0 && start + note->duration < timestamp) {
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
      buffer[pos] = sample * gain->valueAt(timestamp) * panValue;
      panValue = 1 - panValue;
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

void Channel::seek(double timestamp)
{
  if (timestamp == this->timestamp) {
    return;
  }
  notes.clear();
  nextEvent = std::shared_ptr<SequenceEvent>();
  track->seek(timestamp);
  this->timestamp = timestamp;
}

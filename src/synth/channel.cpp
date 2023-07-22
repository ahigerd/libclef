#include "channel.h"
#include "synthcontext.h"
#include "s2wcontext.h"
#include "iinstrument.h"
#include "seq/itrack.h"
#include "seq/sequenceevent.h"
#include <iostream>

Channel::Note::Note()
: event(nullptr), source(nullptr), duration(0), kill(true), inUse(false)
{
  // initializers only
}

Channel::Channel(const SynthContext* ctx, ITrack* track)
: AudioParamContainer(ctx), mute(false), track(track), nextEvent(nullptr)
{
  notePool.resize(128);
  poolFree = 128;
  poolNext = 0;

  instrument = ctx->defaultInstrument();
  gain = addParam(AudioNode::Gain, 1.0).get();
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
        if (chEvent->param == 'inst') {
          instrument = ctx->getInstrument(chEvent->intValue);
          if (!instrument) {
            instrument = ctx->defaultInstrument();
          }
        } else {
          instrument->channelEvent(this, chEvent);
        }
      } else if (auto modEvent = ModulatorEvent::castShared(event)) {
        instrument->modulatorEvent(this, modEvent);
      } else if (auto noteEvent = BaseNoteEvent::castShared(event)) {
        Note* note = instrument->noteEvent(this, noteEvent);
        trackNote(note);
      } else if (event->eventType() >= SequenceEvent::UserBase) {
        // This must go after the BaseNoteEvent case because user-defined
        // note events should be sent there.
        Note* note = instrument->userEvent(this, event);
        trackNote(note);
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
  while (pos < numSamples && !isFinished()) {
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
          deallocNote(note);
        } else if (start <= timestamp) {
          // TODO: modulators
          // TODO: mixdown if source is stereo and output is mono
          sample += note->source->getSample(timestamp - start, ch);
        }
      }
      for (uint64_t id : toErase) {
        notes.erase(id);
      }
      buffer[pos] = mute ? 0 : (sample * gain->valueAt(timestamp) * panValue);
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
  for (int i = notePool.size() - 1; i >= 0; --i) {
    notePool[i].inUse = false;
  }
  poolNext = 0;
  poolFree = notePool.size();
  notes.clear();
  nextEvent = std::shared_ptr<SequenceEvent>();
  track->seek(timestamp);
  this->timestamp = timestamp;
}

Channel::Note* Channel::allocNote(const std::shared_ptr<BaseNoteEvent>& event, const std::shared_ptr<AudioNode>& source, double duration)
{
  if (poolFree == 0) {
    poolNext = notePool.size();
    poolFree = notePool.size();
    notePool.resize(notePool.size() * 2);
  }
  int mask = notePool.size() - 1;
  Note* note = nullptr;
  do {
    note = &notePool[poolNext];
    poolNext = (poolNext + 1) & mask;
  } while (note->inUse);
  poolFree--;
  note->inUse = true;
  note->event = event;
  note->source = source;
  note->duration = duration;
  note->kill = false;
  return note;
}

void Channel::deallocNote(Note* note)
{
  note->event.reset();
  note->source.reset();
  note->inUse = false;
  poolFree++;
}

void Channel::trackNote(Note* note)
{
  if (note && note->inUse && !note->kill) {
    uint64_t id = note->event->playbackID;
    auto iter = notes.find(id);
    if (iter == notes.end()) {
      notes[id] = note;
    } else {
      deallocNote(iter->second);
      iter->second = note;
    }
  }
}

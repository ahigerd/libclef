#include "itrack.h"

ITrack::~ITrack() {}

void ITrack::reset()
{
  internalReset();
  lastEvent = seekEvent = std::shared_ptr<SequenceEvent>();
}

void ITrack::seek(double timestamp)
{
  double eventTime;
  if (lastEvent) {
    if (BaseNoteEvent* note = dynamic_cast<BaseNoteEvent*>(lastEvent.get())) {
      if (lastEvent->timestamp < timestamp && lastEvent->timestamp + note->duration > timestamp) {
        // No seeking necessary, it's just going to keep playing this event
        seekEvent = lastEvent;
        return;
      }
    }
    if (lastEvent->timestamp > timestamp) {
      // Seeking backwards
      reset();
    }
  } else {
    // No last event, be sure to reset just in case
    reset();
  }
  std::shared_ptr<SequenceEvent> event = readNextEvent();
  int i = 0;
  while (event && event->timestamp < timestamp) {
    BaseNoteEvent* note = dynamic_cast<BaseNoteEvent*>(event.get());
    if (note) {
      eventTime += note->duration;
      if (eventTime > timestamp) {
        break;
      }
    }
    if (isFinished()) {
      seekEvent = std::shared_ptr<SequenceEvent>();
      lastEvent = event;
      return;
    }
    event = readNextEvent();
  }
  lastEvent = seekEvent = event;
}

std::shared_ptr<SequenceEvent> ITrack::nextEvent()
{
  std::shared_ptr<SequenceEvent> event = seekEvent;
  if (event) {
    seekEvent = std::shared_ptr<SequenceEvent>();
  } else {
    event = readNextEvent();
  }
  if (event) {
    lastEvent = event;
  }
  return event;
}

BasicTrack::BasicTrack() : position(0)
{
  // initializers only
}

void BasicTrack::addEvent(SequenceEvent* event)
{
  events.push_back(std::shared_ptr<SequenceEvent>(event));
}

bool BasicTrack::isFinished() const
{
  return position >= events.size();
}

std::shared_ptr<SequenceEvent> BasicTrack::readNextEvent()
{
  if (position >= events.size()) {
    return nullptr;
  }
  auto& event = events[position];
  position++;
  return event;
}

void BasicTrack::internalReset()
{
  position = 0;
}

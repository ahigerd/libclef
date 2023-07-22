#include "realtimetrack.h"
#include <iostream>

RealTimeTrack::RealTimeTrack()
: queueHead(0), queueTail(0)
{
  // initializers only
}

bool RealTimeTrack::isFinished() const
{
  return false;
}

void RealTimeTrack::seek(double timestamp)
{
  /* not supported */
}

double RealTimeTrack::length() const
{
  return -1;
}

std::shared_ptr<SequenceEvent> RealTimeTrack::readNextEvent()
{
  if (queueHead == queueTail) {
    return nullptr;
  }
  std::shared_ptr<SequenceEvent> event(nullptr);
  event.swap(events[queueTail++]);
  return event;
}

void RealTimeTrack::internalReset()
{
  while (readNextEvent());
}

void RealTimeTrack::addEvent(SequenceEvent* event)
{
  events[queueHead++].reset(event);
}

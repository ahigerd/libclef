#include "itrack.h"

ITrack::~ITrack() {}

BasicTrack::BasicTrack() : position(0)
{
  // initializers only
}

void BasicTrack::addEvent(SequenceEvent* event)
{
  events.push_back(std::unique_ptr<SequenceEvent>(event));
}

bool BasicTrack::isFinished() const
{
  return position >= events.size();
}

SequenceEvent* BasicTrack::nextEvent()
{
  if (position >= events.size()) {
    return nullptr;
  }
  SequenceEvent* event = events[position].get();
  position++;
  return event;
}

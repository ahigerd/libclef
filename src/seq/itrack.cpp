#include "itrack.h"

ITrack::~ITrack() {}

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

std::shared_ptr<SequenceEvent> BasicTrack::nextEvent()
{
  if (position >= events.size()) {
    return nullptr;
  }
  auto& event = events[position];
  position++;
  return event;
}

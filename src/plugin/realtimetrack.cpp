#include "realtimetrack.h"
#include <iostream>

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
  if (readQueue.empty()) {
    return nullptr;
  }
  auto event = readQueue.front();
  readQueue.pop();
  return event;
}

void RealTimeTrack::internalReset()
{
  std::lock_guard lock(mutex);
  while (!readQueue.empty()) {
    readQueue.pop();
  }
  while (!writeQueue.empty()) {
    writeQueue.pop();
  }
}

void RealTimeTrack::addEvent(SequenceEvent* event)
{
  writeQueue.emplace(event);
}

void RealTimeTrack::sync()
{
  std::lock_guard lock(mutex);
  while (!writeQueue.empty()) {
    readQueue.push(std::move(writeQueue.front()));
    writeQueue.pop();
  }
}

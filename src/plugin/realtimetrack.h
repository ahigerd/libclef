#ifndef CLEF_REALTIMETRACK_H
#define CLEF_REALTIMETRACK_H

#include "seq/itrack.h"
#include <atomic>

class RealTimeTrack : public ITrack
{
public:
  RealTimeTrack();

  bool isFinished() const;
  void seek(double timestamp);
  virtual double length() const;

  void addEvent(SequenceEvent* event);

protected:
  std::shared_ptr<SequenceEvent> readNextEvent();
  void internalReset();

  std::shared_ptr<SequenceEvent> events[256];
  std::atomic<uint8_t> queueHead, queueTail;
};

#endif

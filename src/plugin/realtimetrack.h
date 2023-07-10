#ifndef S2W_REALTIMETRACK_H
#define S2W_REALTIMETRACK_H

#include "seq/itrack.h"
#include <mutex>
#include <queue>

class RealTimeTrack : public ITrack
{
public:
  bool isFinished() const;
  void seek(double timestamp);
  virtual double length() const;

  void addEvent(SequenceEvent* event);
  void sync();

protected:
  std::shared_ptr<SequenceEvent> readNextEvent();
  void internalReset();

  std::queue<std::shared_ptr<SequenceEvent>> readQueue;
  std::queue<std::shared_ptr<SequenceEvent>> writeQueue;
  std::mutex mutex;
};

#endif

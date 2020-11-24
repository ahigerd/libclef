#ifndef S2W_ITRACK_H
#define S2W_ITRACK_H

#include "sequenceevent.h"
#include <vector>
#include <memory>

class ITrack {
public:
  virtual ~ITrack();

  virtual bool isFinished() const = 0;
  virtual SequenceEvent* nextEvent() = 0;
};

class BasicTrack : public ITrack {
public:
  BasicTrack();

  virtual bool isFinished() const;
  virtual SequenceEvent* nextEvent();

  void addEvent(SequenceEvent* event);

protected:
  int position;
  std::vector<std::unique_ptr<SequenceEvent>> events;
};

#endif

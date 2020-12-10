#ifndef S2W_ITRACK_H
#define S2W_ITRACK_H

#include "sequenceevent.h"
#include <vector>
#include <memory>

class ITrack {
public:
  virtual ~ITrack();

  virtual bool isFinished() const = 0;
  virtual std::shared_ptr<SequenceEvent> nextEvent();

  virtual void reset();
  virtual void seek(double timestamp);
  virtual double length() const = 0;

protected:
  virtual std::shared_ptr<SequenceEvent> readNextEvent() = 0;
  virtual void internalReset() = 0;
  std::shared_ptr<SequenceEvent> seekEvent;
  std::shared_ptr<SequenceEvent> lastEvent;
};

class BasicTrack : public ITrack {
public:
  BasicTrack();

  virtual bool isFinished() const;
  virtual double length() const;

  void addEvent(SequenceEvent* event);

protected:
  virtual std::shared_ptr<SequenceEvent> readNextEvent();
  virtual void internalReset();
  int position;
  double maximumTimestamp;
  std::vector<std::shared_ptr<SequenceEvent>> events;
};

#endif

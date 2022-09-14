#include "isequence.h"
#include "sequenceevent.h"

ISequence::ISequence(S2WContext* ctx)
: ctx(ctx)
{
  // initializers only
}

ISequence::~ISequence() {}

S2WContext* ISequence::context() const
{
  return ctx;
}

double ISequence::duration() const
{
  return -1;
}

bool ISequence::canLoop() const
{
  return false;
}

bool ISequence::isFinished() const
{
  for (int i = numTracks() - 1; i >= 0; --i) {
    if (!getTrack(i)->isFinished()) {
      return false;
    }
  }
  return true;
}

StreamSequence::StreamSequence(S2WContext* ctx, uint64_t sampleID, double startTime)
: BaseSequence(ctx)
{
  BasicTrack* track = new BasicTrack;
  SampleEvent* event = new SampleEvent;
  event->sampleID = sampleID;
  event->timestamp = startTime;
  track->addEvent(event);
  tracks.emplace_back(track);
}

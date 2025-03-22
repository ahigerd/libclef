#include "isequence.h"
#include "sequenceevent.h"
#include "clefcontext.h"
#include "codec/sampledata.h"

ISequence::ISequence(ClefContext* ctx)
: ctx(ctx)
{
  // initializers only
}

ISequence::~ISequence() {}

ClefContext* ISequence::context() const
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

StreamSequence::StreamSequence(ClefContext* ctx, uint64_t sampleID, double startTime)
: BaseSequence(ctx)
{
  BasicTrack* track = new BasicTrack;
  SampleEvent* event = new SampleEvent;
  SampleData* sample = ctx->getSample(sampleID);
  event->sampleID = sampleID;
  event->timestamp = startTime;
  event->duration = sample ? sample->duration() : -1;
  track->addEvent(event);
  tracks.emplace_back(track);
}

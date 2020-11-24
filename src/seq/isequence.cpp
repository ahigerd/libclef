#include "isequence.h"

ISequence::~ISequence() {}

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

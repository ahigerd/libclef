#ifndef S2W_ISEQUENCE_H
#define S2W_ISEQUENCE_H

#include <cstdint>
#include <vector>
#include <memory>
#include "itrack.h"

class ISequence {
public:
  virtual ~ISequence();

  // Default returns -1 (play until isFinished())
  virtual double duration() const;

  // Default implementation returns false
  virtual bool canLoop() const;

  // Default implementation returns true if all tracks are finished
  virtual bool isFinished() const;

  virtual int numTracks() const = 0;
  virtual const ITrack* getTrack(int index) const = 0;
  virtual ITrack* getTrack(int index) = 0;
};

template <class Track = BasicTrack>
class BaseSequence : public ISequence {
public:
  virtual int numTracks() const {
    return tracks.size();
  }

  virtual ITrack* getTrack(int index) {
    return tracks[index].get();
  }

  virtual const ITrack* getTrack(int index) const {
    return tracks[index].get();
  }

protected:
  virtual void addTrack(Track* track) {
    tracks.emplace_back(track);
  }

  std::vector<std::unique_ptr<Track>> tracks;
};

class StreamSequence : public BaseSequence<> {
public:
  StreamSequence(uint64_t sampleID, double startTime = 0);
};

#endif

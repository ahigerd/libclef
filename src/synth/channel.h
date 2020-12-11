#ifndef S2W_CHANNEL_H
#define S2W_CHANNEL_H

#include "s2wconfig.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include "audionode.h"
#include "envelope.h"
class SynthContext;
class ITrack;
class SequenceEvent;

class Channel {
  friend class SynthContext;
public:
  Channel(const SynthContext* ctx, ITrack* track);
  ~Channel();

  double gain;

  void seek(double timestamp);
  uint32_t fillBuffer(std::vector<int16_t>& buffer, ssize_t numSamples = -1);
  bool isFinished() const;

private:
  const SynthContext* const ctx;
  ITrack* track;
  std::shared_ptr<SequenceEvent> nextEvent;

  struct Note {
    Note(std::shared_ptr<SequenceEvent> event, std::shared_ptr<AudioNode> source, double duration);
    Note(const Note& other) = default;
    Note(Note&& other) = default;
    Note& operator=(const Note& other) = default;

    std::shared_ptr<SequenceEvent> event;
    std::shared_ptr<AudioNode> source;
    double duration;
  };
  std::unordered_map<uint64_t, std::unique_ptr<Note>> notes;

  double timestamp;
};

#endif

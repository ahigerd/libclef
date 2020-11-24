#ifndef S2W_CHANNEL_H
#define S2W_CHANNEL_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <cstdint>
#include "audionode.h"
class SynthContext;
class ITrack;
class SequenceEvent;

class Channel {
public:
  Channel(SynthContext* ctx, ITrack* track);
  ~Channel();

  double gain;

  uint32_t fillBuffer(std::vector<int16_t>& buffer);
  bool isFinished() const;

private:
  SynthContext* ctx;
  ITrack* track;
  SequenceEvent* nextEvent;

  struct Note {
    Note(SequenceEvent* event, AudioNode* source, double duration);
    Note(const Note& other) = default;
    Note(Note&& other) = default;
    Note& operator=(const Note& other) = default;

    SequenceEvent* event;
    std::unique_ptr<AudioNode> source;
    double duration;
  };
  std::unordered_map<uint64_t, std::unique_ptr<Note>> notes;

  double timestamp;
};

#endif

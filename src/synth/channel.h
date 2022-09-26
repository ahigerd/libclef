#ifndef S2W_CHANNEL_H
#define S2W_CHANNEL_H

#include "s2wconfig.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include "audionode.h"
#include "audioparam.h"
#include "envelope.h"
class SynthContext;
class ITrack;
class SequenceEvent;
class BaseNoteEvent;
class IInstrument;

class Channel : public AudioParamContainer {
  friend class SynthContext;
public:
  Channel(const SynthContext* ctx, ITrack* track);
  ~Channel();

  AudioParam* gain;
  AudioParam* pan;

  void seek(double timestamp);
  uint32_t fillBuffer(std::vector<int16_t>& buffer, ssize_t numSamples = -1);
  bool isFinished() const;

  struct Note {
    Note();
    Note(std::shared_ptr<BaseNoteEvent> event, AudioNode* source, double duration);
    Note(std::shared_ptr<BaseNoteEvent> event, std::shared_ptr<AudioNode> source, double duration);
    Note(const Note& other) = default;
    Note(Note&& other) = default;
    Note& operator=(const Note& other) = default;

    std::shared_ptr<BaseNoteEvent> event;
    std::shared_ptr<AudioNode> source;
    double duration;
    bool kill;
  };
  std::unordered_map<uint64_t, std::unique_ptr<Note>> notes;

private:
  ITrack* track;
  IInstrument* instrument;
  std::shared_ptr<SequenceEvent> nextEvent;

  double timestamp;
};

#endif

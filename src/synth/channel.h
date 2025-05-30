#ifndef CLEF_CHANNEL_H
#define CLEF_CHANNEL_H

#include "clefconfig.h"
#include <unordered_map>
#include <vector>
#include <deque>
#include <memory>
#include <cstdint>
#include "audionode.h"
#include "audioparam.h"
#include "envelope.h"
class SynthContext;
class ITrack;
class SequenceEvent;
struct BaseNoteEvent;
class IInstrument;

class Channel : public AudioParamContainer {
  friend class SynthContext;
public:
  Channel(const SynthContext* ctx, ITrack* track);
  ~Channel();

  AudioParam* gain;
  AudioParam* pan;
  bool mute;

  void seek(double timestamp);
  uint32_t fillBuffer(std::vector<int16_t>& buffer, ssize_t numSamples = -1);
  bool isFinished() const;

  class Note {
  public:
    Note();
    std::shared_ptr<BaseNoteEvent> event;
    std::shared_ptr<AudioNode> source;
    double duration;
    bool kill;

  private:
    friend class Channel;
    bool inUse;
  };
  std::unordered_map<uint64_t, Note*> notes;
  Note* allocNote(const std::shared_ptr<BaseNoteEvent>& event, const std::shared_ptr<AudioNode>& source, double duration);
  inline Note* allocNote(const std::shared_ptr<BaseNoteEvent>& event, AudioNode* source, double duration) {
    return allocNote(event, std::shared_ptr<AudioNode>(source), duration);
  }

private:
  void trackNote(Note* note);
  void deallocNote(Note* note);

  ITrack* track;
  IInstrument* instrument;
  std::shared_ptr<SequenceEvent> nextEvent;
  std::deque<Note> notePool;
  int poolFree, poolNext;

  double timestamp;
};

#endif

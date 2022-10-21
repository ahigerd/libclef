#ifndef S2W_SYNTHCONTEXT_H
#define S2W_SYNTHCONTEXT_H

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
class Channel;
class IInterpolator;
class ITrack;
class RiffWriter;
class IInstrument;
class S2WContext;

class SynthContext {
public:
  SynthContext(S2WContext* ctx, double sampleRate, int outputChannels = 2);
  ~SynthContext();

  S2WContext* s2wContext() const;

  const double sampleRate;
  const double sampleTime;
  const int outputChannels;
  std::vector<std::unique_ptr<Channel>> channels;
  IInterpolator* interpolator;

  void addChannel(Channel* channel);
  void addChannel(ITrack* track);

  double currentTime() const;
  double maximumTime() const;
  void seek(double timestamp);
  size_t fillBuffer(uint8_t* buffer, size_t length);
  void stream(std::ostream& output);
  void save(RiffWriter* riff);

  void registerInstrument(uint64_t id, std::unique_ptr<IInstrument>&& inst);
  IInstrument* getInstrument(uint64_t id) const;
  IInstrument* defaultInstrument() const;

private:
  std::unordered_map<uint64_t, std::unique_ptr<IInstrument>> instruments;
  std::unique_ptr<IInstrument> defaultInst;
  std::vector<int16_t> mixBuffer;
  double currentTimestamp, maximumTimestamp;
  S2WContext* ctx;
};

#endif

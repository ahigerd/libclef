#ifndef S2W_SYNTHCONTEXT_H
#define S2W_SYNTHCONTEXT_H

#include <cstdint>
#include <vector>
#include <memory>
class Channel;
class IInterpolator;
class ITrack;
class RiffWriter;

struct SynthContext {
  SynthContext(double sampleRate, int outputChannels = 2);
  ~SynthContext();

  const double sampleRate;
  const int outputChannels;
  const double sampleTime;
  std::vector<std::unique_ptr<Channel>> channels;
  IInterpolator* interpolator;

  void addChannel(Channel* channel);
  void addChannel(ITrack* track);

  double currentTime() const;
  void seek(double timestamp);
  size_t fillBuffer(uint8_t* buffer, size_t length);
  void stream(std::ostream& output);
  void save(RiffWriter* riff);

private:
  std::vector<int16_t> mixBuffer;
  double currentTimestamp;
};

#endif

#ifndef CLEF_SYNTHCONTEXT_H
#define CLEF_SYNTHCONTEXT_H

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include <map>
class Channel;
class IInterpolator;
class ITrack;
class RiffWriter;
class IInstrument;
class ClefContext;

class SynthContext {
public:
  SynthContext(ClefContext* ctx, double sampleRate, int outputChannels = 2);
  ~SynthContext();

  ClefContext* clefContext() const;

  const double sampleRate;
  const double sampleTime;
  const int outputChannels;

  void setSampleRate(double newRate);

  std::vector<std::unique_ptr<Channel>> channels;
  IInterpolator* interpolator;

  void addChannel(Channel* channel);
  void addChannel(ITrack* track);

  double currentTime() const;
  double maximumTime() const;
  void seek(double timestamp);
  size_t fillBuffer(uint8_t* buffer, size_t length);
  void fillBuffers(float* buffers[], size_t numSamples);
  void stream(std::ostream& output);
  void save(RiffWriter* riff);

  void registerInstrument(uint64_t id, std::unique_ptr<IInstrument>&& inst);
  int numInstruments() const;
  uint64_t instrumentID(int index) const;
  IInstrument* getInstrument(uint64_t id) const;
  IInstrument* defaultInstrument() const;
  std::string instrumentName(uint64_t id) const;

protected:
  std::unique_ptr<IInstrument> defaultInst;
  std::vector<uint64_t> instrumentIDs;
  std::map<uint64_t, std::unique_ptr<IInstrument>> instruments;

private:
  std::vector<int16_t> mixBuffer;
  double currentTimestamp, maximumTimestamp;
  ClefContext* ctx;
};

#endif

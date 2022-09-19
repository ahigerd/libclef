#ifndef S2W_SAMPLEDATA_H
#define S2W_SAMPLEDATA_H

#include <cstdint>
#include <vector>
class S2WContext;

struct SampleData {
  enum {
    Uncached = 0xFFFFFFFFFFFFFFFFULL,
  };

  SampleData(S2WContext* ctx, uint64_t sampleID, double sampleRate = 44100.0, int loopStart = -1, int loopEnd = -1);
  SampleData(S2WContext* ctx, double sampleRate = 44100.0, int loopStart = -1, int loopEnd = -1);

  uint32_t numSamples() const;
  double duration() const;
  int16_t at(int index, int channel = 0) const;

  uint64_t sampleID;
  double sampleRate;
  int loopStart;
  int loopEnd;
  std::vector<std::vector<int16_t>> channels;

private:
  mutable int32_t m_numSamples;
  mutable double m_duration;
};

#endif

#ifndef CLEF_SAMPLEDATA_H
#define CLEF_SAMPLEDATA_H

#include <cstdint>
#include <vector>
class ClefContext;

struct SampleData {
  enum {
    Uncached = 0xFFFFFFFFFFFFFFFFULL,
  };

  SampleData(ClefContext* ctx, uint64_t sampleID, double sampleRate = 44100.0, int loopStart = -1, int loopEnd = -1);
  SampleData(ClefContext* ctx, double sampleRate = 44100.0, int loopStart = -1, int loopEnd = -1);

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
  mutable uint8_t m_numChannels;
};

#endif

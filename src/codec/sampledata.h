#ifndef S2W_SAMPLEDATA_H
#define S2W_SAMPLEDATA_H

#include <cstdint>
#include <vector>

struct SampleData {
  static SampleData* get(uint64_t sampleID);

  SampleData(uint64_t sampleID, double sampleRate = 44100.0, int loopStart = -1, int loopEnd = -1);
  SampleData(double sampleRate = 44100.0, int loopStart = -1, int loopEnd = -1);

  uint32_t numSamples() const;
  double duration(double sampleRate = -1) const;
  uint16_t at(int index, int channel = 0) const;

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

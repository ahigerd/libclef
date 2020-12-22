#ifndef S2W_MSADPCMCODEC_H
#define S2W_MSADPCMCODEC_H

#include "icodec.h"

class MsAdpcmCodec : public ICodec {
public:
  MsAdpcmCodec(uint16_t blockSize, uint16_t channels);

  virtual SampleData* decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID = 0);

protected:
  int32_t c1, c2;
  uint16_t blockSize, channels;
  int16_t delta;

  int16_t getNextSample(int16_t& h1, int16_t& h2, int8_t value, int channel);
  void decodeBlock(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, int channel, std::vector<int16_t>& sample);
};

#endif


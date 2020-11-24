#ifndef S2W_PCMCODEC_H
#define S2W_PCMCODEC_H

#include "icodec.h"

class PcmCodec : public ICodec {
public:
  PcmCodec(int sampleBits, int channels = 1, bool bigEndian = true);

  virtual SampleData* decode(const std::vector<uint8_t>& buffer, uint64_t sampleID = 0);

protected:
  int sampleBytes;
  int channels;
  bool bigEndian;
};

#endif

#ifndef CLEF_PCMCODEC_H
#define CLEF_PCMCODEC_H

#include "icodec.h"

class PcmCodec : public ICodec {
public:
  PcmCodec(ClefContext* ctx, int sampleBits, int channels = 1, bool bigEndian = true);

  virtual SampleData* decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID = 0);

protected:
  void decodePcm4(SampleData* sampleData, std::vector<uint8_t>::const_iterator start, int length);
  int sampleBits;
  int sampleBytes;
  int channels;
  bool bigEndian;
};

#endif

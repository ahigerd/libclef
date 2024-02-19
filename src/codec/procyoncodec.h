#ifndef CLEF_PROCYONCODEC_H
#define CLEF_PROCYONCODEC_H

#include "icodec.h"

class ProcyonCodec : public ICodec {
public:
  ProcyonCodec(ClefContext* ctx, bool stereo = false);

  virtual SampleData* decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID = 0);

protected:
  int32_t history[2], predictor[2];
  int16_t factor0, factor1, scale;
  bool stereo;
  int16_t getNextSample(int8_t value, int channel);
};

#endif

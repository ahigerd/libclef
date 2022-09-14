#ifndef S2W_PROCYONCODEC_H
#define S2W_PROCYONCODEC_H

#include "icodec.h"

class ProcyonCodec : public ICodec {
public:
  ProcyonCodec(S2WContext* ctx);

  virtual SampleData* decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID = 0);

protected:
  int32_t history, predictor;
  int16_t factor0, factor1, scale;
  int16_t getNextSample(int8_t value);
};

#endif

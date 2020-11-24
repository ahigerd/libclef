#ifndef S2W_PROCYONCODEC_H
#define S2W_PROCYONCODEC_H

#include "icodec.h"

class ProcyonCodec : public ICodec {
public:
  ProcyonCodec();

  virtual std::vector<int16_t> decode(const std::vector<uint8_t>& buffer);

protected:
  int32_t history, predictor;
  int16_t factor0, factor1, scale;
  int16_t getNextSample(int8_t value);
};

#endif

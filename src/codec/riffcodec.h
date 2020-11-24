#ifndef S2W_RIFFCODEC_H
#define S2W_RIFFCODEC_H

#include "icodec.h"

class RiffCodec : public ICodec {
public:
  virtual SampleData* decode(const std::vector<uint8_t>& buffer, uint64_t sampleID = 0);
};

#endif

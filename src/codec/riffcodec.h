#ifndef S2W_RIFFCODEC_H
#define S2W_RIFFCODEC_H

#include "icodec.h"

class RiffCodec : public ICodec {
public:
  virtual SampleData* decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID = 0);
};

#endif

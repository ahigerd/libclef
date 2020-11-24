#ifndef S2W_ICODEC_H
#define S2W_ICODEC_H

#include <cstdint>
#include <vector>
#include <string>
#include "sampledata.h"

class ICodec {
public:
  virtual ~ICodec();

  virtual SampleData* decodeFile(const std::string& filename, uint64_t sampleID = 0);
  virtual SampleData* decode(const std::vector<uint8_t>& buffer, uint64_t sampleID = 0) = 0;
};

#endif

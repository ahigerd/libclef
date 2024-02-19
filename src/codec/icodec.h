#ifndef CLEF_ICODEC_H
#define CLEF_ICODEC_H

#include <cstdint>
#include <vector>
#include <string>
#include "sampledata.h"
class ClefContext;

class ICodec {
public:
  ICodec(ClefContext* ctx);
  virtual ~ICodec();

  // TODO: header
  virtual SampleData* decodeFile(const std::string& filename, uint64_t sampleID = 0);
  SampleData* decode(const std::vector<uint8_t>& buffer, uint64_t sampleID = 0);
  virtual SampleData* decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID = 0) = 0;

protected:
  ClefContext* context() const;

private:
  ClefContext* ctx;
};

#endif

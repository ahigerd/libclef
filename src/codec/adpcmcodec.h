#ifndef S2W_ADPCMCODEC_H
#define S2W_ADPCMCODEC_H

#include "icodec.h"

class AdpcmCodec : public ICodec {
public:
  enum Format {
    IMA,
    NDS,
    OKI4s,
    // TODO: Microsoft
  };
  AdpcmCodec(Format format);

  virtual SampleData* decode(const std::vector<uint8_t>& buffer, uint64_t sampleID = 0);

protected:
  Format format;
  const int16_t* stepTable;
  int maxStep;
  const int8_t* indexTable;

  int16_t predictor;
  int8_t index;

  int16_t getNextSample(uint8_t value);
};

#endif

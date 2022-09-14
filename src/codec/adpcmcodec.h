#ifndef S2W_ADPCMCODEC_H
#define S2W_ADPCMCODEC_H

#include "icodec.h"

class AdpcmCodec : public ICodec {
public:
  enum Format {
    IMA,
    DSP,
    OKI4s,
  };
  AdpcmCodec(S2WContext* ctx, Format format, int interleave = 0);

  virtual SampleData* decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID = 0);

protected:
  Format format;
  const int16_t* stepTable;
  int maxStep;
  const int8_t* indexTable;

  int16_t predictor[2];
  int8_t index[2];
  int interleave;

  int16_t getNextSample(uint8_t value, int channel);
};

#endif

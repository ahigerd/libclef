#ifndef S2W_RIFFCODEC_H
#define S2W_RIFFCODEC_H

#include "icodec.h"

struct WaveFormatEx {
  WaveFormatEx();
  WaveFormatEx(const WaveFormatEx& other) = default;
  WaveFormatEx(WaveFormatEx&& other) = default;
  WaveFormatEx(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end);

  WaveFormatEx& operator=(const WaveFormatEx& other) = default;
  WaveFormatEx& operator=(WaveFormatEx&& other) = default;

  uint16_t format;
  uint16_t channels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t sampleBits;
  std::vector<uint8_t> exData;
};

class RiffCodec : public ICodec {
public:
  RiffCodec(S2WContext* ctx);

  virtual SampleData* decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID = 0);
};

#endif

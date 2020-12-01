#include "pcmcodec.h"
#include "utility.h"

PcmCodec::PcmCodec(int sampleBits, int channels, bool bigEndian)
: sampleBytes(sampleBits >> 3), channels(channels), bigEndian(bigEndian)
{
  // initializers only
}

SampleData* PcmCodec::decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID)
{
  SampleData* sampleData = sampleID ? new SampleData(sampleID) : new SampleData();
  int length = (end - start) / channels / sampleBytes;
  for (int i = 0; i < channels; i++) {
    sampleData->channels.push_back(std::vector<int16_t>());
    sampleData->channels[i].reserve(length);
  }
  int offset = 0;
  for (int i = 0; i < length; i++) {
    for (int c = 0; c < channels; c++) {
      int32_t sample = 0;
      for (int b = 0; b < sampleBytes; b++) {
        if (bigEndian) {
          sample = (sample << 8) | *start++;
        } else {
          sample |= *start++ << (b * 8);
        }
        ++offset;
      }
      if (sampleBytes == 1) {
        sample <<= 8;
      }
      sampleData->channels[c].push_back(clamp<int16_t>(int16_t(sample), -0x8000, 0x7FFF));
    }
  }
  return sampleData;
}

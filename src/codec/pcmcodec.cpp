#include "pcmcodec.h"
#include "utility.h"

PcmCodec::PcmCodec(S2WContext* ctx, int sampleBits, int channels, bool bigEndian)
: ICodec(ctx), sampleBits(sampleBits), sampleBytes(sampleBits >> 3), channels(channels), bigEndian(bigEndian)
{
  // initializers only
}

SampleData* PcmCodec::decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID)
{
  SampleData* sampleData = sampleID ? new SampleData(context(), sampleID) : new SampleData(context());
  int length = 8 * (end - start) / channels / sampleBits;
  for (int i = 0; i < channels; i++) {
    sampleData->channels.push_back(std::vector<int16_t>());
    sampleData->channels[i].reserve(length);
  }
  if (sampleBytes == 0) {
    decodePcm4(sampleData, start, length);
    return sampleData;
  }
  for (int i = 0; i < length; i++) {
    for (int c = 0; c < channels; c++) {
      int32_t sample = 0;
      for (int b = 0; b < sampleBytes; b++) {
        if (bigEndian) {
          sample = (sample << 8) | *start++;
        } else {
          sample |= *start++ << (b * 8);
        }
      }
      if (sampleBytes == 1) {
        sample <<= 8;
      }
      sampleData->channels[c].push_back(clamp<int16_t>(int16_t(sample), -0x8000, 0x7FFF));
    }
  }
  return sampleData;
}

void PcmCodec::decodePcm4(SampleData* sampleData, std::vector<uint8_t>::const_iterator start, int length)
{
  int right = channels - 1;
  if (channels == 1) {
    length >>= 1;
  }
  for (int i = 0; i < length; i++, start++) {
    if (bigEndian) {
      sampleData->channels[0].push_back(((*start & 0xF0) << 8) - 0x8000);
      sampleData->channels[right].push_back(((*start & 0x0F) << 12) - 0x8000);
    } else {
      sampleData->channels[0].push_back(((*start & 0x0F) << 12) - 0x8000);
      sampleData->channels[right].push_back(((*start & 0xF0) << 8) - 0x8000);
    }
  }
}

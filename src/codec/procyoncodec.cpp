#include "procyoncodec.h"
#include "../utility.h"

static const int16_t procyonFactor[][2] = {
  { 0, 0 },     { 60, 0 }, { 115, -52 }, { 98, -55 },
  { 122, -60 }, { 0, 0 },  { 0, 0 },     { 0, 0 },
  { 0, 0 },     { 0, 0 },  { 0, 0 },     { 0, 0 },
  { 0, 0 },     { 0, 0 },  { 0, 0 },     { 0, 0 },
};
static const int16_t maxProcyonFactor = (sizeof(procyonFactor) >> 2) - 1;

ProcyonCodec::ProcyonCodec()
: history(0), predictor(0)
{
  // initializers only
}

int16_t ProcyonCodec::getNextSample(int8_t value)
{
  int32_t sample = int32_t(value << scale) + ((predictor * factor0 + history * factor1 + 32) >> 6);
  history = predictor;
  predictor = sample;
  return clamp<int16_t>((sample + 32) >> 6, -0x7FFF, 0x7FFF);
}

SampleData* ProcyonCodec::decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID)
{
  history = 0;
  predictor = 0;
  SampleData* sampleData = sampleID ? new SampleData(sampleID) : new SampleData();
  int length = end - start;
  sampleData->channels.push_back(std::vector<int16_t>());
  sampleData->channels[0].reserve(length);
  std::vector<int16_t>& sample = sampleData->channels[0];
  sample.reserve((length * 30) >> 4);
  uint8_t buffer[16];
  while (start != end) {
    for (int i = 0; i < 16; i++) {
      buffer[i] = *start++;
    }
    int index = uint8_t(buffer[15] ^ 0x80) >> 4;
    scale = 6 + (buffer[15] & 0x0f);
    factor0 = procyonFactor[index][0];
    factor1 = procyonFactor[index][1];

    for (int i = 0; i < 15; i++) {
      // Shift left and then right in order to get sign extension
      int8_t low = int8_t(uint8_t(buffer[i]) << 4) >> 4;
      int8_t high = int8_t(buffer[i] ^ 0x80) >> 4;
      sample.push_back(getNextSample(low));
      sample.push_back(getNextSample(high));
    }
  }
  return sampleData;
}

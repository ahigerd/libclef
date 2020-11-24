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

std::vector<int16_t> ProcyonCodec::decode(const std::vector<uint8_t>& buffer)
{
  history = 0;
  predictor = 0;
  int length = buffer.size();
  std::vector<int16_t> sample;
  sample.reserve((length * 30) >> 4);
  for (int offset = 0; offset < length; offset += 16) {
    int index = uint8_t(buffer[offset + 15] ^ 0x80) >> 4;
    scale = 6 + (buffer[offset + 15] & 0x0f);
    factor0 = procyonFactor[index][0];
    factor1 = procyonFactor[index][1];

    for (int i = 0; i < 15; i++) {
      // Shift left and then right in order to get sign extension
      int8_t low = int8_t(uint8_t(buffer[offset + i]) << 4) >> 4;
      int8_t high = int8_t(buffer[offset + i] ^ 0x80) >> 4;
      sample.push_back(getNextSample(low));
      sample.push_back(getNextSample(high));
    }
  }
  return sample;
}

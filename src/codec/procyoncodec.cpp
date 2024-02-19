#include "procyoncodec.h"
#include "utility.h"

static const int16_t procyonFactor[][2] = {
  { 0, 0 },     { 60, 0 }, { 115, -52 }, { 98, -55 },
  { 122, -60 }, { 0, 0 },  { 0, 0 },     { 0, 0 },
  { 0, 0 },     { 0, 0 },  { 0, 0 },     { 0, 0 },
  { 0, 0 },     { 0, 0 },  { 0, 0 },     { 0, 0 },
};
static const int16_t maxProcyonFactor = (sizeof(procyonFactor) >> 2) - 1;

ProcyonCodec::ProcyonCodec(ClefContext* ctx, bool stereo)
: ICodec(ctx), stereo(stereo)
{
  // initializers only
}

int16_t ProcyonCodec::getNextSample(int8_t value, int channel)
{
  int32_t sample = int32_t(value << scale) + ((predictor[channel] * factor0 + history[channel] * factor1 + 32) >> 6);
  history[channel] = predictor[channel];
  predictor[channel] = sample;
  return clamp<int16_t>((sample + 32) >> 6, -0x7FFF, 0x7FFF);
}

SampleData* ProcyonCodec::decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID)
{
  SampleData* sampleData = sampleID ? new SampleData(context(), sampleID) : new SampleData(context());
  int channels = stereo ? 2 : 1;
  int length = (end - start) / channels;
  for (int i = 0; i < channels; i++) {
    history[i] = 0;
    predictor[i] = 0;
    sampleData->channels.push_back(std::vector<int16_t>());
    sampleData->channels[i].reserve((length * 30) >> 4);
  }
  int channel = 0;
  uint8_t buffer[16];
  while (start != end) {
    std::vector<int16_t>& sample = sampleData->channels[channel];
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
      sample.push_back(getNextSample(low, channel));
      sample.push_back(getNextSample(high, channel));
    }

    if (stereo) {
      channel = (channel + 1) % 2;
    }
  }
  return sampleData;
}

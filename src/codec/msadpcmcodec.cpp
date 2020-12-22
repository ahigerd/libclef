#include "msadpcmcodec.h"
#include "utility.h"
#include <algorithm>

static const int16_t msFactor[][2] = {
  { 256, 0 }, { 512, -256 },
  { 0, 0 },   { 192, 64 },
  { 240, 0 }, { 460, -208 },
  { 392, -232 },
};

static const int16_t msStep[] = {
  230, 230, 230, 230, 307, 409, 512, 614,
  768, 614, 512, 409, 307, 230, 230, 230,
};

MsAdpcmCodec::MsAdpcmCodec(uint16_t blockSize, uint16_t channels) : blockSize(blockSize), channels(channels)
{
  // initializers only
}

int16_t MsAdpcmCodec::getNextSample(int16_t& h1, int16_t& h2, int8_t value, int channel)
{
  // sign-extension
  value = int8_t(uint8_t(value) << 4) >> 4;
  int16_t result = clamp<int16_t>((h1 * c1 + h2 * c2) / 256 + value * delta, -0x8000, 0x7fff);
  delta = std::max(16, (msStep[value & 0x0f] * delta) / 256);
  h2 = h1;
  return h1 = result;
}

void MsAdpcmCodec::decodeBlock(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, int channel, std::vector<int16_t>& sample)
{
  int8_t step = start[channel];
  c1 = msFactor[step][0];
  c2 = msFactor[step][1];
  delta = parseInt<int16_t>(start, channels + channel * 2);
  int16_t history1 = parseInt<int16_t>(start, channels * 3 + channel * 2);
  int16_t history2 = parseInt<int16_t>(start, channels * 5 + channel * 2);

  sample.push_back(history1);
  sample.push_back(history2);

  int shift = channel ? 0 : 4;
  start += channels * 7;
  for (; start < end; ++start) {
    if (channels == 2) {
      sample.push_back(getNextSample(history1, history2, (*start >> shift) & 0x0F, channel));
    } else {
      sample.push_back(getNextSample(history1, history2, (*start >> 4) & 0x0F, channel));
      sample.push_back(getNextSample(history1, history2, *start & 0x0F, channel));
    }
  }
}

SampleData* MsAdpcmCodec::decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID)
{
  SampleData* sampleData = sampleID ? new SampleData(sampleID) : new SampleData();

  int samplesPerBlock = (blockSize - 7 * channels) * 2 / channels + 2;
  for (int i = 0; i < channels; i++) {
    sampleData->channels.push_back(std::vector<int16_t>());
    sampleData->channels[i].reserve(samplesPerBlock);
  }

  auto blockStart = start;
  while (blockStart < end) {
    auto blockEnd = blockStart + blockSize;
    if (blockEnd > end) {
      blockEnd = end;
    }
    for (int i = 0; i < channels; i++) {
      decodeBlock(blockStart, blockEnd, i, sampleData->channels[i]);
    }
    blockStart = blockEnd;
  }
  return sampleData;
}

#include "adpcmcodec.h"
#include "utility.h"

static const int8_t adpcmIndex[] = { -1, -1, -1, -1, 2, 4, 6, 8, };

static const int16_t imaStep[] = {
      7,     8,     9,    10,    11,    12,    13,    14,    16,    17,
     19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
     50,    55,    60,    66,    73,    80,    88,    97,   107,   118,
    130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
    337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
    876,   963,  1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
   2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
   5894,  6484,  7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
  15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767,
};
static const int maxImaStep = (sizeof(imaStep) >> 1) - 1;

static const int16_t oki4sStep[] = {
    256,   272,   304,   336,   368,   400,   448,   496,   544,   592,
    656,   720,   800,   880,   960,  1056,  1168,  1280,  1408,  1552,
   1712,  1888,  2080,  2288,  2512,  2768,  3040,  3344,  3680,  4048,
   4464,  4912,  5392,  5936,  6528,  7184,  7904,  8704,  9568, 10528,
  11584, 12736, 14016, 15408, 16960, 18656, 20512, 22576, 24832,
};
static const int maxOki4sStep = (sizeof(oki4sStep) >> 1) - 1;

AdpcmCodec::AdpcmCodec(AdpcmCodec::Format format, int interleave)
: format(format), predictor{ 0, 0 }, index{ 0, 0 }, interleave(interleave)
{
  if (format == OKI4s) {
    stepTable = oki4sStep;
    maxStep = maxOki4sStep;
  } else {
    stepTable = imaStep;
    maxStep = maxImaStep;
  }
  indexTable = adpcmIndex;
}

int16_t AdpcmCodec::getNextSample(uint8_t value, int channel)
{
  int16_t step = stepTable[index[channel]];
  int32_t delta = step >> 3;
  if (value & 0x04) delta += step;
  if (value & 0x02) delta += step >> 1;
  if (value & 0x01) delta += step >> 2;
  if (value & 0x08) delta = -delta;
  if (format == NDS) {
    if (predictor[channel] + delta == -0x8000) {
      predictor[channel] = -0x8000;
    } else {
      predictor[channel] = clamp<int16_t>(predictor[channel] + delta, -0x7fff, 0x7fff);
    }
  } else {
    predictor[channel] = clamp<int16_t>(predictor[channel] + delta, -0x8000, 0x7fff);
  }
  index[channel] = clamp<int8_t>(index[channel] + indexTable[value & 0x07], 0, maxStep);
  return predictor[channel];
}

SampleData* AdpcmCodec::decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID)
{
  SampleData* sampleData = sampleID ? new SampleData(sampleID) : new SampleData();
  int length = end - start;

  if (length > 4 && format == NDS) {
    predictor[0] = (*start++ << 8) | *start++;
    index[0] = clamp<int8_t>(int16_t((*start++ << 8) | *start++), 0, maxStep);
    length -= 4;
  } else {
    predictor[0] = 0;
    predictor[1] = 0;
    index[0] = 0;
    index[1] = 0;
  }

  sampleData->channels.push_back(std::vector<int16_t>());
  int highChannel = interleave < 0 ? 1 : 0;
  if (interleave) {
    sampleData->channels.push_back(std::vector<int16_t>());
    sampleData->channels[0].reserve(length);
    sampleData->channels[1].reserve(length);
  } else {
    sampleData->channels[0].reserve(length << 1);
  }
  if (interleave <= 0) {
    while (start != end) {
      uint8_t byte = *start++;
      sampleData->channels[0].push_back(getNextSample(byte & 0x0f, 0));
      sampleData->channels[highChannel].push_back(getNextSample((byte & 0xf0) >> 4, highChannel));
    }
  }
  return sampleData;
}

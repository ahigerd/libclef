#include "iinterpolator.h"
#include "../codec/sampledata.h"
#include "../utility.h"
#include <cmath>
#include <iostream>

static LinearInterpolator* iLin = new LinearInterpolator;

// Keep in the same order as InterpolationMode
// Use nullptr for interpolators with nontrivial constructors
static IInterpolator* allInterpolators[] = {
  new ZeroInterpolator,
  iLin,
  nullptr,
  new SharpInterpolator,
  nullptr,
  new Lagrange4Interpolator,
  new Lagrange6Interpolator,
};

IInterpolator* IInterpolator::get(IInterpolator::InterpolationMode mode)
{
  if (allInterpolators[mode] == nullptr) {
    switch (mode) {
      case Cosine:
        return allInterpolators[mode] = new CosineInterpolator;
      case Sinc:
        return allInterpolators[mode] = new SincInterpolator;
      default:
        break;
    }
  }
  return allInterpolators[mode];
}

int16_t ZeroInterpolator::interpolate(const SampleData* data, double time, int channel, double sampleStep) const
{
  return data->at(std::floor(time), channel);
}

int16_t LinearInterpolator::interpolate(const SampleData* data, double time, int channel, double sampleStep) const
{
  return lerp(data->at(time, channel), data->at(time + 1, channel), time - std::floor(time));
}

CosineInterpolator::CosineInterpolator()
{
  for(int i = 0; i < Resolution; i++) {
    lut[i] = (1.0 - std::cos(M_PI * i / Resolution) * M_PI) * 0.5;
  }
}

int16_t CosineInterpolator::interpolate(const SampleData* data, double time, int channel, double sampleStep) const
{
  int16_t left = data->at(time, channel);
  int16_t right = data->at(time + 1, channel);
  double weight = time - std::floor(time);
  return lerp(left, right, lut[size_t(weight * Resolution)]);
}

int16_t SharpInterpolator::interpolate(const SampleData* data, double time, int channel, double sampleStep) const
{
  size_t index = size_t(time);
  int left = data->at(index - 1, channel);
  int sample = data->at(index, channel);
  int right = data->at(index + 1, channel);
  if ((sample >= left) == (sample >= right)) {
    // Always preserve extrema as-is
    return sample;
  }
  int left2 = data->at(index - 2, channel);
  int right2 = data->at(index + 2, channel);
  double subsample = time - std::floor(time);
  if ((right > right2) == (right > sample) || (left > left2) == (left > sample)) {
    // Wider history window is non-monotonic
    return lerp(sample, right, subsample);
  }

  // Include a linear interpolation of the surrounding samples to try to smooth out single-sample errors
  double linear = lerp(left, right, 1.0 + subsample);
  // Projection approaching from the left
  double mLeft = sample - left;
  double pLeft = sample + mLeft * subsample;
  // Projection approaching from the right
  double negSubsample = 1 - subsample;
  double mRight = right - sample;
  double pRight = right - mRight * negSubsample;
  int16_t result = (mLeft * negSubsample + mRight * subsample + linear) / 3;
  if ((left <= result) != (result <= right)) {
    // If the result isn't monotonic, fall back to linear
    return lerp(sample, right, subsample);
  }
  return result;
}

SincInterpolator::SincInterpolator()
{
  double dy = M_PI / Samples, y = dy;
  double dx = dy * Width, x = dx;
  sincLut[0] = 0;
  windowLut[0] = 1;
  for (int i = 1; i <= Samples; ++i, x += dx, y += dy)
  {
    sincLut[i] = x < Width ? std::sin(x) / x : 0.0;
    windowLut[i] = 0.40897 + 0.5 * std::cos(y) + 0.09103 * std::cos(2 * y);
  }
}

int16_t SincInterpolator::interpolate(const SampleData* data, double time, int channel, double sampleStep) const
{
  size_t index = size_t(time);
  double subsample = time - std::floor(time);
  int step = sampleStep > 1 ? Resolution * sampleStep : Resolution;
  int yShift = subsample * step;
  int xShift = subsample * Resolution;
  double sum = 0.0;
  double kernelSum = 0;
  double kernel;
  for (int i = 1 - Width; i <= Width; ++i) {
    kernel = sincLut[std::abs(i * step - xShift)] * windowLut[std::abs(i * Resolution - yShift)];
    kernelSum += kernel;
    sum += data->at(index + i, channel) * kernel;
  }
  return sum / kernelSum;
}

int16_t Lagrange4Interpolator::interpolate(const SampleData* data, double time, int channel, double sampleStep) const
{
  size_t index = size_t(time);
  double subsample = time - std::floor(time);
  int16_t d0 = data->at(index, channel);
  int16_t d1 = data->at(index + 1, channel);
  int16_t dm1 = data->at(index - 1, channel);
  int16_t d2 = data->at(index + 2, channel);
  double result = (d2 - dm1) / 6.0 + (d0 - d1) * 0.5;
  result = result * subsample + (dm1 + d1) * 0.5 - d0;
  result = result * subsample + d1 - (dm1 / 3.0) - (d0 * 0.5) - (d2 / 6.0);
  return result * subsample + d0;
}

int16_t Lagrange6Interpolator::interpolate(const SampleData* data, double time, int channel, double sampleStep) const
{
  size_t index = size_t(time);
  double subsample = time - std::floor(time) - 0.5;
  int16_t d0 = data->at(index, channel);
  int16_t d1 = data->at(index + 1, channel);
  int16_t dm1 = data->at(index - 1, channel);
  int16_t d2 = data->at(index + 2, channel);
  int16_t dm2 = data->at(index - 2, channel);
  int16_t d3 = data->at(index + 3, channel);
  double even1 = dm2 + d3, odd1 = dm2 - d3;
  double even2 = dm1 + d2, odd2 = dm1 - d2;
  double even3 = d0 + d1, odd3 = d0 - d1;
  double result = 1 / 24.0 * odd2 - 1 / 12.0 * odd3 - 1 / 120.0 * odd1;
  result = result * subsample + 1 / 48.0 * even1 - 0.0625 * even2 + 1 / 24.0 * even3;
  result = result * subsample + 1 / 48.0 * odd1 - 13 / 48.0 * odd2 + 17 / 24.0 * odd3;
  result = result * subsample + 0.40625 * even2 - 17 / 48.0 * even3 - 5 / 96.0 * even1;
  result = result * subsample + 25 / 384.0 * odd2 - 1.171875 * odd3 - 0.0046875 * odd1;
  return result * subsample + 0.01171875 * even1 - 0.09765625 * even2 + 0.5859375 * even3;
}

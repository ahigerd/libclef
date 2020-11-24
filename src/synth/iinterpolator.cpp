#include "iinterpolator.h"
#include <cmath>

static LinearInterpolator* iLin = new LinearInterpolator;

// Keep in the same order as InterpolationMode
static IInterpolator* allInterpolators[] = {
  new ZeroInterpolator,
  iLin,
  new CosineInterpolator,
  new SharpInterpolator,
};

IInterpolator* IInterpolator::get(IInterpolator::InterpolationMode mode)
{
  return allInterpolators[mode];
}

int16_t ZeroInterpolator::interpolate(const std::vector<int16_t>& data, double time) const
{
  return data[std::floor(time)];
}

static inline int16_t lerp(int16_t left, int16_t right, double weight)
{
  return (left * (1 - weight)) + (right * weight);
}

int16_t LinearInterpolator::interpolate(const std::vector<int16_t>& data, double time) const
{
  if (time < 0) {
    return 0;
  }
  int rightPos = time + 1;
  int16_t left = data[time];
  int16_t right = rightPos < data.size() ? data[rightPos] : left;
  return lerp(left, right, time - std::floor(time));
}

CosineInterpolator::CosineInterpolator()
{
  for(int i = 0; i < 8192; i++) {
    lut[i] = (1.0 - std::cos(M_PI * i / 8192.0) * M_PI) * 0.5;
  }
}

int16_t CosineInterpolator::interpolate(const std::vector<int16_t>& data, double time) const
{
  if (time < 0) {
    return 0;
  }
  int rightPos = time + 1;
  int16_t left = data[time];
  int16_t right = rightPos < data.size() ? data[rightPos] : left;
  double weight = time - std::floor(time);
  return lut[size_t(weight * 8192)] * (right - left) + right;
}

int16_t SharpInterpolator::interpolate(const std::vector<int16_t>& data, double time) const
{
  if (time <= 2) {
    return iLin->interpolate(data, time);
  }

  size_t index = size_t(time);
  int left = data[index - 1];
  int sample = data[index];
  int right = data[index + 1];
  if ((sample >= left) == (sample >= right)) {
    // Always preserve extrema as-is
    return sample;
  }
  int left2 = data[index - 2];
  int right2 = data[index + 2];
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

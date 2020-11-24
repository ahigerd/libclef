#include "sampler.h"
#include "iinterpolator.h"
#include "../utility.h"

Sampler::Sampler(const SampleData* sample, double sampleRate)
: sampleRate(sampleRate < 0 ? sample->sampleRate : sampleRate), gain(1.0),
  interpolator(IInterpolator::get(IInterpolator::Cosine)),
  sample(sample), offset(0), lastTime(0)
{
  // initializers only
}

int16_t Sampler::getSample(double time)
{
  if (sample->loopStart < 0 && offset >= sample->numSamples()) {
    return 0;
  }
  double timeStep = time - lastTime;
  double sampleStep = timeStep * sampleRate * (sampleRate / sample->sampleRate);
  lastTime = time;
  double result = interpolator->interpolate(sample->channels[0], offset) * gain;
  offset += sampleStep;
  if (sample->loopStart >= 0 && offset >= sample->loopEnd) {
    offset -= sample->loopStart;
  }
  return clamp<int16_t>(result, -0x8000, 0x7FFF);
}

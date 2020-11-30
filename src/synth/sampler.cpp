#include "sampler.h"
#include "iinterpolator.h"
#include "../utility.h"
#include <iostream>

Sampler::Sampler(const SampleData* sample, double pitchBend)
: interpolator(IInterpolator::get(IInterpolator::Sinc)),
  sample(sample), offset(0), lastTime(0)
{
  addParam(PitchBend, pitchBend);
  addParam(Gain, 1.0);
  addParam(Pan, 0.5);
}

bool Sampler::isActive() const
{
  return sample->loopStart >= 0 || offset < sample->numSamples();
}

int16_t Sampler::generateSample(double time, int channel)
{
  if (!isActive()) {
    return 0;
  }
  double timeStep = time - lastTime;
  double sampleStep = timeStep * sample->sampleRate * paramValue(PitchBend, time);
  lastTime = time;
  double result = interpolator->interpolate(sample, offset, channel, sampleStep);
  offset += sampleStep;
  if (sample->loopStart >= 0 && offset >= sample->loopEnd) {
    offset -= sample->loopStart;
  }
  return clamp<int16_t>(result, -0x8000, 0x7FFF);
}

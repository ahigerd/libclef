#include "sampler.h"
#include "iinterpolator.h"
#include "../utility.h"
#include <iostream>

Sampler::Sampler(const SampleData* sample, double sampleRate)
: sampleRate(sampleRate < 0 ? sample->sampleRate : sampleRate),
  interpolator(IInterpolator::get(IInterpolator::Cosine)),
  sample(sample), offset(0), lastTime(0), gain(1.0), pan(0.5)
{
  updateGain();
}

int16_t Sampler::getSample(double time, int channel)
{
  if (sample->loopStart < 0 && offset >= sample->numSamples()) {
    return 0;
  }
  double timeStep = time - lastTime;
  double sampleStep = timeStep * sampleRate * (sampleRate / sample->sampleRate);
  lastTime = time;
  // TODO: 2D panning?
  // TODO: mixdown using context
  double result = interpolator->interpolate(sample, offset, channel) * (channel % 2 ? rightGain : leftGain);
  offset += sampleStep;
  if (sample->loopStart >= 0 && offset >= sample->loopEnd) {
    offset -= sample->loopStart;
  }
  return clamp<int16_t>(result, -0x8000, 0x7FFF);
}

void Sampler::setGain(double gain)
{
  this->gain = gain;
  updateGain();
}

void Sampler::setPan(double pan)
{
  this->pan = pan;
  updateGain();
}

void Sampler::updateGain()
{
  this->leftGain = (1 - pan) * gain;
  this->rightGain = pan * gain;
}

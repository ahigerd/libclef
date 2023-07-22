#include "sampler.h"
#include "synthcontext.h"
#include "iinterpolator.h"
#include "utility.h"

Sampler::Sampler(const SynthContext* ctx, const SampleData* sample, double pitch)
: AudioNode(ctx), interpolator(ctx->interpolator), sample(sample), offset(0), lastTime(0)
{
  pPitch = addParam(Pitch, pitch).get();
  pPitchBend = addParam(PitchBend, 1.0).get();
  addParam(Gain, 1.0);
  addParam(Pan, 0.5);
}

bool Sampler::isActive() const
{
  return sample->loopEnd > 0 || offset < sample->numSamples();
}

int16_t Sampler::generateSample(double time, int channel)
{
  if (!isActive()) {
    return 0;
  }
  double sampleStepBase = ctx->sampleTime * sample->sampleRate;
  double sampleStep = sampleStepBase * pPitch->valueAt(lastTime) * pPitchBend->valueAt(lastTime);
  while (time > lastTime) {
    offset += sampleStep;
    lastTime += ctx->sampleTime;
    sampleStep = sampleStepBase * paramValue(Pitch, lastTime) * paramValue(PitchBend, lastTime);
  }
  if (sample->loopStart >= 0 && offset >= sample->loopEnd) {
    offset -= (sample->loopEnd - sample->loopStart);
  }
  double result = sampleStep == 1 ? sample->at(offset, channel) : interpolator->interpolate(sample, offset, channel, sampleStep);
  return clamp<int16_t>(result, -0x8000, 0x7FFF);
}

#include "sampler.h"
#include "synthcontext.h"
#include "iinterpolator.h"
#include "utility.h"

Sampler::Sampler(const SynthContext* ctx, const SampleData* sample, double pitchBend)
: AudioNode(ctx), interpolator(ctx->interpolator), sample(sample), offset(0), lastTime(-1)
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
  double sampleStep = ctx->sampleTime * sample->sampleRate * paramValue(PitchBend, time);
  if (time != lastTime) {
    offset += sampleStep;
    lastTime = time;
  }
  double result = sampleStep == 1 ? sample->at(offset, channel) : interpolator->interpolate(sample, offset, channel, sampleStep);
  if (sample->loopStart >= 0 && offset >= sample->loopEnd) {
    offset -= (sample->loopEnd - sample->loopStart);
  }
  return clamp<int16_t>(result, -0x8000, 0x7FFF);
}

#include "envelope.h"
#include "../utility.h"

Envelope::Envelope(double attack, double hold, double sustain, double decay, double release)
: stepAt(0), lastLevel(0), step(Attack)
{
  addParam(StartGain, 0.0);
  addParam(Attack, attack);
  addParam(Hold, hold);
  addParam(Sustain, sustain);
  addParam(Decay, decay);
  addParam(Release, release);
  addParam(Trigger, 1.0);
}

bool Envelope::isActive() const
{
  return step != 0;
}

int16_t Envelope::filterSample(double time, int channel, int16_t sample)
{
  if (step && step != Release && paramValue(Trigger, time) <= 0) {
    stepAt = time;
    step = Release;
  }
  switch (step) {
    case Attack: {
      double a = paramValue(Attack, time);
      if (time < a) {
        return (lastLevel = lerp(paramValue(StartGain, time), 1.0, time, 0.0, a)) * sample;
      }
      step = Hold;
      stepAt = time;
    }
    case Hold: {
      double h = stepAt + paramValue(Hold, time);
      if (time < h) {
        lastLevel = 1.0;
        return sample;
      }
      step = Sustain;
      stepAt = time;
    }
    case Sustain: {
      double d = stepAt + paramValue(Decay, time);
      if (time < d) {
        return (lastLevel = lerp(1.0, paramValue(Sustain, time), time, stepAt, d)) * sample;
      }
      step = Decay;
      stepAt = time;
    }
    case Decay: {
      lastLevel = paramValue(Sustain, time);
      return lastLevel * sample;
    }
    case Release: {
      double r = paramValue(Release, time);
      if (time > stepAt + r) {
        step = 0;
        return 0;
      }
      return lerp(lastLevel, 0.0, time, stepAt, stepAt + r) * sample;
    }
    default: {
      return 0;
    }
  }
}

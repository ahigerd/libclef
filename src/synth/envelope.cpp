#include "envelope.h"
#include "utility.h"
#include <cmath>

Envelope::Envelope(const SynthContext* ctx, double attack, double hold, double decay, double sustain, double release)
: FilterNode(ctx), expAttack(false), expDecay(false), stepAt(0), lastLevel(0), step(Attack)
{
  addParam(StartGain, 0.0);
  addParam(Attack, attack);
  addParam(Hold, hold);
  addParam(Decay, decay);
  addParam(Sustain, sustain);
  addParam(Release, release);
  addParam(Trigger, 1.0);
}

bool Envelope::isActive() const
{
  return step != 0 && FilterNode::isActive();
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
      if (expAttack) {
        double dt = time - stepAt;
        lastLevel = paramValue(StartGain, time) + fastExp(a, dt);
        if (lastLevel < 1.0) {
          return lastLevel * sample;
        }
        return (lastLevel = lastLevel - a * time * time) * sample;
      } else if (time < a) {
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
      step = Decay;
      stepAt = time;
    }
    case Decay: {
      double d = paramValue(Decay, time);
      double s = paramValue(Sustain, time);
      if (expDecay && d < 0) {
        double dt = time - stepAt;
        lastLevel = fastExp(-d, dt);
        if (lastLevel > s) {
          return lastLevel * sample;
        }
      } else if (time < d + stepAt) {
        lastLevel = lerp(1.0, paramValue(Sustain, time), time, stepAt, d + stepAt);
        return lastLevel * sample;
      }
      step = Sustain;
      stepAt = time;
    }
    case Sustain: {
      lastLevel = paramValue(Sustain, time);
      return lastLevel * sample;
    }
    case Release: {
      double r = paramValue(Release, time);
      if (expDecay && r < 0) {
        double dt = time - stepAt;
        double level = lastLevel * fastExp(-r, dt);
        if (level <= 0) {
          step = 0;
          return 0;
        }
        return level * sample;
      } else if (time > stepAt + r) {
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

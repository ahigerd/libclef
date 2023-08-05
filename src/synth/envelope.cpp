#include "envelope.h"
#include "utility.h"
#include <cmath>

Envelope::Envelope(const SynthContext* ctx, double attack, double hold, double decay, double sustain, double release)
: Envelope(ctx, attack, hold, decay, sustain, HUGE_VAL, release)
{
  // forwarded constructor only
}

Envelope::Envelope(const SynthContext* ctx, double attack, double hold, double decay, double sustain, double fade, double release)
: FilterNode(ctx), expAttack(false), expDecay(false), stepAt(0), lastLevel(0), step(Attack)
{
  pStartGain = addParam(StartGain, 0.0).get();
  pAttack = addParam(Attack, attack).get();
  pHold = addParam(Hold, hold).get();
  pDecay = addParam(Decay, decay).get();
  pSustain = addParam(Sustain, sustain).get();
  pFade = addParam(Fade, fade).get();
  pRelease = addParam(Release, release).get();
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
      double a = pAttack->valueAt(time);
      if (expAttack) {
        double dt = time - stepAt;
        lastLevel = pStartGain->valueAt(time) + fastExp(a, dt);
        if (lastLevel < 1.0) {
          return lastLevel * sample;
        }
        return (lastLevel = lastLevel - a * time * time) * sample;
      } else if (time < a) {
        return (lastLevel = lerp(pStartGain->valueAt(time), 1.0, time, 0.0, a)) * sample;
      }
      step = Hold;
      stepAt = time;
    }
    case Hold: {
      double h = stepAt + pHold->valueAt(time);
      if (time < h) {
        lastLevel = 1.0;
        return sample;
      }
      step = Decay;
      stepAt = time;
    }
    case Decay: {
      double d = pDecay->valueAt(time);
      double s = pSustain->valueAt(time);
      if (expDecay && d < 0) {
        double dt = time - stepAt;
        lastLevel = fastExp(-d, dt);
        if (lastLevel > s) {
          return lastLevel * sample;
        }
      } else if (time < d + stepAt) {
        lastLevel = lerp(1.0, pSustain->valueAt(time), time, stepAt, d + stepAt);
        return lastLevel * sample;
      }
      step = Sustain;
      stepAt = time;
    }
    case Sustain: {
      double f = pFade->valueAt(time);
      if (std::isfinite(f)) {
        if (expDecay && f < 0) {
          double dt = time - stepAt;
          lastLevel = fastExp(-f, dt);
          if (lastLevel < 1.0 / 32767.0) {
            lastLevel = 0;
          }
        } else if (time < f + stepAt) {
          lastLevel = lerp(pSustain->valueAt(time), 0.0, time, stepAt, f + stepAt);
        }
      } else {
        lastLevel = pSustain->valueAt(time);
      }
      return lastLevel * sample;
    }
    case Release: {
      double r = pRelease->valueAt(time);
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

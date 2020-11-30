#include "envelope.h"
#include "../utility.h"

Envelope::Envelope(double attack, double hold, double sustain, double decay, double release)
: startGain(0.0), attack(attack), hold(hold), sustain(sustain), decay(decay), release(release), stepAt(0), lastLevel(0), step(0)
{
  // initializers only
}

bool Envelope::isActive() const
{
  return step < 5;
}

int16_t Envelope::filterSample(double time, int channel, int16_t sample)
{
  if (step < 4 && trigger(time) <= 0) {
    stepAt = time;
    step = 4;
  }
  switch (step) {
    case 0: {
      double a = attack(time);
      if (time < a) {
        return (lastLevel = lerp(startGain(time), 1.0, time, 0.0, a)) * sample;
      }
      step = 1;
      stepAt = time;
    }
    case 1: {
      double h = stepAt + hold(time);
      if (time < h) {
        lastLevel = 1.0;
        return sample;
      }
      step = 2;
      stepAt = time;
    }
    case 2: {
      double d = stepAt + decay(time);
      if (time < d) {
        return (lastLevel = lerp(1.0, sustain(time), time, stepAt, d)) * sample;
      }
      step = 3;
      stepAt = time;
    }
    case 3: {
      lastLevel = sustain(time);
      return lastLevel * sample;
    }
    case 4: {
      double r = release(time);
      if (time > stepAt + r) {
        step = 5;
        return 0;
      }
      return lerp(lastLevel, 0.0, time, stepAt, stepAt + r) * sample;
    }
    default: {
      return 0;
    }
  }
}

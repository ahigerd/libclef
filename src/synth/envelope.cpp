#include "envelope.h"
#include "../utility.h"

Envelope::Envelope(double attack, double hold, double sustain, double decay, double release)
: startGain(0.0), attack(attack), hold(hold), sustain(sustain), decay(decay), release(release), trigger(1.0), stepAt(0), lastLevel(0), step(0)
{
  // initializers only
}

bool Envelope::isActive() const
{
  return step < 5;
}

int16_t Envelope::generateSample(double time, int channel)
{
  if (step < 4 && trigger(time) <= 0) {
    stepAt = time;
    step = 4;
  }
  if (step == 5) {
    return 0;
  } else if (step == 4) {
    double r = release(time);
    if (time > stepAt + r) {
      step = 5;
      return 0;
    }
    return lerp(lastLevel, 0.0, time, stepAt, stepAt + r);
  }
  if (step == 0) {
    double a = attack(time);
    if (time < a) {
      return lastLevel = lerp(startGain(time), 1.0, time, 0.0, a) * 8192;
    }
    step = 1;
    stepAt = time;
  }
  if (step == 1) {
    double h = stepAt + hold(time);
    if (time < h) {
      return lastLevel = 8192;
    }
    step = 2;
    stepAt = time;
  }
  double s = sustain(time);
  if (step == 2) {
    double d = stepAt + decay(time);
    if (time < d) {
      return lastLevel = lerp(1.0, s, time, stepAt, d) * 8192;
    }
    step = 3;
    stepAt = time;
  }
  return lastLevel = s * 8192;
}

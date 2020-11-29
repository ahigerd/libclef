#include "envelope.h"
#include "../utility.h"

Envelope::Envelope(double attack, double hold, double sustain, double decay, double release)
: startGain(0.0), attack(attack), hold(hold), sustain(sustain), decay(decay), release(release), trigger(1.0), releasedAt(-1)
{
  // initializers only
}

bool Envelope::isActive() const
{
  return !(releasedAt >= 0 && lastLevel == 0);
}

int16_t Envelope::generateSample(double time, int channel)
{
  if (releasedAt < 0 && trigger(time) <= 0) {
    releasedAt = time;
  }
  if (releasedAt >= 0) {
    double r = release(time);
    if (time > releasedAt + r) {
      return lastLevel = 0;
    }
    return lerp(lastLevel, 0.0, time, releasedAt, releasedAt + r);
  }
  double a = attack(time);
  if (time < a) {
    return lastLevel = lerp(startGain(time), 1.0, time, 0.0, a) * 8192;
  }
  double h = a + hold(time);
  if (time < h) {
    return lastLevel = 8192;
  }
  double d = h + decay(time);
  double s = sustain(time);
  if (time < d) {
    return lastLevel = lerp(1.0, s, time, h, d) * 8192;
  }
  return lastLevel = s * 8192;
}

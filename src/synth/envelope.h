#ifndef S2W_ENVELOPE_H
#define S2W_ENVELOPE_H

#include "audionode.h"
#include "audioparam.h"

class Envelope : public FilterNode
{
public:
  Envelope(double attack, double hold, double sustain, double decay, double release);

  AudioParam startGain, attack, hold, sustain, decay, release;

  virtual bool isActive() const;

protected:
  virtual int16_t filterSample(double time, int channel, int16_t sample);
  double stepAt, lastLevel;
  uint8_t step;
};

#endif

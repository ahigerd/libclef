#ifndef S2W_ENVELOPE_H
#define S2W_ENVELOPE_H

#include "audionode.h"

class Envelope : public AudioNode
{
public:
  Envelope(double attack, double hold, double sustain, double decay, double release);

  AudioParameter startGain, attack, hold, sustain, decay, release, trigger;

  virtual bool isActive() const;

protected:
  virtual int16_t generateSample(double time, int channel = 0);
  double releasedAt, lastLevel;
};

#endif

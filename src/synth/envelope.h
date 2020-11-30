#ifndef S2W_ENVELOPE_H
#define S2W_ENVELOPE_H

#include "audionode.h"
#include "audioparam.h"

class Envelope : public FilterNode
{
public:
  enum ParamType {
    StartGain = 'strt',
    Attack = 'atk ',
    Hold = 'hold',
    Sustain = 'sus ',
    Decay = 'decy',
    Release = 'rels',
  };

  Envelope(double attack, double hold, double sustain, double decay, double release);

  virtual bool isActive() const;

protected:
  virtual int16_t filterSample(double time, int channel, int16_t sample);
  double stepAt, lastLevel;
  int step;
};

#endif

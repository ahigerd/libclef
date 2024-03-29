#ifndef CLEF_ENVELOPE_H
#define CLEF_ENVELOPE_H

#include "audionode.h"
#include "audioparam.h"

class Envelope : public FilterNode
{
public:
  enum ParamType {
    StartGain = 'strt',
    Attack = 'atk ',
    Hold = 'hold',
    Decay = 'decy',
    Sustain = 'sus ',
    Fade = 'fade',
    Release = 'rels',
  };

  Envelope(const SynthContext* ctx, double attack, double hold, double decay, double sustain, double release);
  Envelope(const SynthContext* ctx, double attack, double hold, double decay, double sustain, double fade, double release);

  virtual bool isActive() const;
  bool expAttack, expDecay;

protected:
  virtual int16_t filterSample(double time, int channel, int16_t sample);
  double stepAt, lastLevel;
  int step;
  AudioParam* pStartGain;
  AudioParam* pAttack;
  AudioParam* pHold;
  AudioParam* pDecay;
  AudioParam* pSustain;
  AudioParam* pFade;
  AudioParam* pRelease;
};

#endif

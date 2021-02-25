#ifndef S2W_SAMPLER_H
#define S2W_SAMPLER_H

#include "audionode.h"
#include "codec/sampledata.h"
#include <vector>
class IInterpolator;

class Sampler : public AudioNode {
public:
  enum ParamType {
    Pitch = 'ptch',
    PitchBend = 'bend',
  };

  Sampler(const SynthContext* ctx, const SampleData* sample, double pitch = 1.0);

  IInterpolator* interpolator;

  virtual bool isActive() const;
  virtual int16_t generateSample(double time, int channel = 0);

protected:
  const SampleData* sample;
  double offset;
  double lastTime;
};

#endif

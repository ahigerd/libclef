#ifndef S2W_SAMPLER_H
#define S2W_SAMPLER_H

#include "audionode.h"
#include "../codec/sampledata.h"
#include <vector>
class IInterpolator;

class Sampler : public AudioNode {
public:
  Sampler(const SampleData* sample, double pitchBend = 1.0);

  AudioParameter pitchBend;
  IInterpolator* interpolator;

  virtual int16_t generateSample(double time, int channel = 0);

protected:
  const SampleData* sample;
  double offset;
  double lastTime;
};

#endif

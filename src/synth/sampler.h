#ifndef S2W_SAMPLER_H
#define S2W_SAMPLER_H

#include "audionode.h"
#include "../codec/sampledata.h"
#include <vector>
class IInterpolator;

class Sampler : public AudioNode {
public:
  Sampler(const SampleData* sample, double sampleRate = -1);

  double sampleRate;
  double gain;
  IInterpolator* interpolator;

  // TODO: stereo
  virtual int16_t getSample(double time);

protected:
  const SampleData* sample;
  double offset;
  double lastTime;
};

#endif

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
  IInterpolator* interpolator;

  void setGain(double gain);
  void setPan(double pan);

  virtual int16_t getSample(double time, int channel = 0);

protected:
  const SampleData* sample;
  double offset;
  double lastTime;
  double gain, pan, leftGain, rightGain;
  void updateGain();
};

#endif

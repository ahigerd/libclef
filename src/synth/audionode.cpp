#include "audionode.h"

AudioParameter::AudioParameter(double initialValue)
: source(nullptr), constant(initialValue)
{
  // initializers only
}

AudioParameter::AudioParameter(AudioNode* source, double scale, double offset)
{
  connect(source, scale, offset);
}

double AudioParameter::valueAt(double time) const
{
  if (source) {
    return source->getSample(time) * scale + constant;
  } else {
    return constant;
  }
}

void AudioParameter::setConstant(double value)
{
  source = nullptr;
  constant = value;
}

void AudioParameter::connect(AudioNode* source, double scale, double offset)
{
  source = source;
  this->scale = scale;
  constant = offset;
}

AudioNode::AudioNode()
: gain(1.0), pan(0.5)
{
  // initializers only
}

int16_t AudioNode::getSample(double time, int channel)
{
  double scale = gain(time) * (channel % 2 ? pan(time) : (1 - pan(time)));
  return scale ? generateSample(time, channel) * scale : 0;
}

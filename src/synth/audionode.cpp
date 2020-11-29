#include "audionode.h"

AudioParameter::AudioParameter(double initialValue)
{
  setConstant(initialValue);
}

AudioParameter::AudioParameter(AudioNode* source, double scale, double offset)
{
  connect(source, scale, offset);
}

AudioParameter::AudioParameter(AudioParameter* source, double scale, double offset)
{
  connect(source, scale, offset);
}

double AudioParameter::valueAt(double time) const
{
  if (source) {
    return source->getSample(time) * scale + constant;
  } else if (sourceParam) {
    return sourceParam->valueAt(time) * scale + constant;
  } else {
    return constant;
  }
}

void AudioParameter::setConstant(double value)
{
  source = nullptr;
  sourceParam = nullptr;
  constant = value;
}

void AudioParameter::connect(AudioNode* source, double scale, double offset)
{
  this->source = source;
  this->sourceParam = nullptr;
  this->scale = scale;
  this->constant = offset;
}

void AudioParameter::connect(AudioParameter* source, double scale, double offset)
{
  this->source = nullptr;
  this->sourceParam = source;
  this->scale = scale;
  this->constant = offset;
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

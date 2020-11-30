#include "audioparam.h"
#include "audionode.h"

AudioParam::AudioParam(const SynthContext* ctx, double initialValue) : ctx(ctx)
{
  setConstant(initialValue);
}

AudioParam::AudioParam(std::shared_ptr<AudioNode> source, double scale, double offset)
: ctx(source->ctx)
{
  connect(source, scale, offset);
}

AudioParam::AudioParam(std::shared_ptr<AudioParam> source, double scale, double offset)
: ctx(source->ctx)
{
  connect(source, scale, offset);
}

double AudioParam::valueAt(double time) const
{
  if (sourceNode) {
    return sourceNode->getSample(time) * scale + constant;
  } else if (sourceParam) {
    return sourceParam->valueAt(time) * scale + constant;
  } else {
    return constant;
  }
}

void AudioParam::setConstant(double value)
{
  sourceNode = nullptr;
  sourceParam = nullptr;
  constant = value;
}

void AudioParam::connect(std::shared_ptr<AudioNode> source, double scale, double offset)
{
  this->sourceNode = source;
  this->sourceParam = nullptr;
  this->scale = scale;
  this->constant = offset;
}

void AudioParam::connect(std::shared_ptr<AudioParam> source, double scale, double offset)
{
  this->sourceNode = nullptr;
  this->sourceParam = source;
  this->scale = scale;
  this->constant = offset;
}


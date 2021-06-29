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

AudioParamContainer::AudioParamContainer(const SynthContext* ctx) : ctx(ctx)
{
  // initializers only
}

std::shared_ptr<AudioParam> AudioParamContainer::param(int32_t param) const
{
  auto iter = params.find(param);
  if (iter != params.end()) {
    return iter->second;
  }
  return nullptr;
}

void AudioParamContainer::addParam(int32_t key, double initialValue)
{
  params[key].reset(new AudioParam(ctx, initialValue));
}

void AudioParamContainer::addParam(int32_t key, std::shared_ptr<AudioParam> param)
{
  if (param) {
    params[key] = param;
  }
}

double AudioParamContainer::paramValue(int32_t key, double time, double defaultValue) const
{
  std::shared_ptr<AudioParam> obj(param(key));
  if (obj) {
    return obj->valueAt(time);
  }
  return defaultValue;
}


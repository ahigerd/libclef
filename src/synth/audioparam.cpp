#include "audioparam.h"
#include "audionode.h"
#include "automationnode.h"
#include <iostream>

AudioParam::AudioParam(const SynthContext* ctx, double initialValue) : ctx(ctx), scale(1.0)
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
  scale = 1.0;
  constant = value;
}

void AudioParam::setValueAt(double time, double value)
{
  if (time <= 0.0) {
    setConstant(value);
    return;
  }
  auto automation = std::dynamic_pointer_cast<AutomationNode>(sourceNode);
  if (!automation) {
    connect(std::shared_ptr<AudioNode>(new AutomationNode(ctx, valueAt(time))));
    automation = std::dynamic_pointer_cast<AutomationNode>(sourceNode);
  }
  automation->setValueAt(time, value / scale);
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

std::shared_ptr<AudioParam> AudioParamContainer::addParam(int32_t key, double initialValue)
{
  std::shared_ptr<AudioParam> p(new AudioParam(ctx, initialValue));
  addParam(key, p);
  return p;
}

void AudioParamContainer::addParam(int32_t key, std::shared_ptr<AudioParam> param)
{
  if (param) {
    onParamAdded(key, params[key] = param);
  }
}

void AudioParamContainer::onParamAdded(int32_t key, std::shared_ptr<AudioParam>& param)
{
  // default implementation does nothing
}

double AudioParamContainer::paramValue(int32_t key, double time, double defaultValue) const
{
  std::shared_ptr<AudioParam> obj(param(key));
  if (obj) {
    return obj->valueAt(time);
  }
  return defaultValue;
}


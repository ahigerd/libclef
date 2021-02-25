#include "audionode.h"

AudioNode::AudioNode(const SynthContext* ctx) : ctx(ctx)
{
  // initializers only
}

std::shared_ptr<AudioParam> AudioNode::param(int32_t param) const
{
  auto iter = params.find(param);
  if (iter != params.end()) {
    return iter->second;
  }
  return nullptr;
}

void AudioNode::addParam(int32_t key, double initialValue)
{
  params[key].reset(new AudioParam(ctx, initialValue));
}

void AudioNode::addParam(int32_t key, std::shared_ptr<AudioParam> param)
{
  if (param) {
    params[key] = param;
  }
}

double AudioNode::paramValue(int32_t key, double time, double defaultValue) const
{
  std::shared_ptr<AudioParam> obj(param(key));
  if (obj) {
    return obj->valueAt(time);
  }
  return defaultValue;
}

int16_t AudioNode::getSample(double time, int channel)
{
  double pan = paramValue(Pan, time, 0.5);
  double scale = paramValue(Gain, time, 1.0) * (channel % 2 ? pan : (1 - pan));
  return scale ? generateSample(time, channel) * scale : 0;
}

FilterNode::FilterNode(const SynthContext* ctx) : AudioNode(ctx)
{
  // initializers only
}

void FilterNode::connect(std::shared_ptr<AudioNode> source)
{
  this->source = source;
  bool addTrigger = true;
  addParam(Trigger, 1.0);
  for (const auto& iter : source->params) {
    if (iter.first == Gain || iter.first == Pan) continue;
    addParam(iter.first, iter.second);
  }
}

bool FilterNode::isActive() const
{
  return source && source->isActive();
}

int16_t FilterNode::generateSample(double time, int channel)
{
  if (!source) {
    return 0;
  }
  double sample = source->getSample(time, channel);
  return filterSample(time, channel, sample);
}


#include "audionode.h"

AudioNode::AudioNode()
: gain(1.0), pan(0.5), trigger(1.0)
{
  // initializers only
}

int16_t AudioNode::getSample(double time, int channel)
{
  double scale = gain(time) * (channel % 2 ? pan(time) : (1 - pan(time)));
  return scale ? generateSample(time, channel) * scale : 0;
}

void FilterNode::connect(std::shared_ptr<AudioNode> source)
{
  this->source = source;
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


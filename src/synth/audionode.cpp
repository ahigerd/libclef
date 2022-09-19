#include "audionode.h"
#include "synthcontext.h"

AudioNode::AudioNode(const SynthContext* ctx) : AudioParamContainer(ctx)
{
  // initializers only
}

int16_t AudioNode::getSample(double time, int channel)
{
  double scale = paramValue(Gain, time, 1.0);
  if (ctx->outputChannels > 1) {
    double pan = paramValue(Pan, time, 0.5);
    scale *= ((channel % 2) ? pan : (1 - pan));
  }
  return scale ? generateSample(time, channel) * scale : 0;
}

FilterNode::FilterNode(const SynthContext* ctx)
: AudioNode(ctx)
{
  // initializers only
}

FilterNode::FilterNode(std::shared_ptr<AudioNode> source)
: AudioNode(source->ctx)
{
  connect(source);
}

void FilterNode::connect(std::shared_ptr<AudioNode> source)
{
  this->source = source;
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

DelayNode::DelayNode(const SynthContext* ctx, double delay, double initial)
: FilterNode(ctx)
{
  init(delay, initial);
}

DelayNode::DelayNode(std::shared_ptr<AudioNode> source, double delay, double initial)
: FilterNode(source)
{
  init(delay, initial);
}

void DelayNode::init(double delay, double initial)
{
  addParam(Delay, delay);
  addParam(Initial, initial);
}

int16_t DelayNode::generateSample(double time, int channel)
{
  time -= paramValue(Delay, time, 0);
  if (time < 0) {
    return paramValue(Initial, time, 0);
  }
  return FilterNode::generateSample(time, channel);
}

int16_t DelayNode::filterSample(double time, int channel, int16_t sample)
{
  // No sample filtering by default, but a subclass could still override it
  return sample;
}

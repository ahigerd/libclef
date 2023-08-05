#include "audionode.h"
#include "synthcontext.h"

AudioNode::AudioNode(const SynthContext* ctx)
: AudioParamContainer(ctx), pGain(nullptr), pPan(nullptr)
{
  // initializers only
}

void AudioNode::onParamAdded(int32_t key, std::shared_ptr<AudioParam>& param)
{
  if (key == Gain) {
    pGain = &param;
  } else if (key == Pan) {
    pPan = &param;
  }
}

int16_t AudioNode::getSample(double time, int channel)
{
  double scale = pGain ? (*pGain)->valueAt(time) : 1.0;
  if (ctx->outputChannels > 1) {
    if (pPan) {
      double pan = (*pPan)->valueAt(time);
      scale *= ((channel & 0x1) ? pan : (1.0 - pan));
    } else {
      scale *= .5;
    }
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
  pDelay = addParam(Delay, delay).get();
  pInitial = addParam(Initial, initial).get();
}

int16_t DelayNode::generateSample(double time, int channel)
{
  time -= pDelay->valueAt(time);
  if (time < 0) {
    return pInitial->valueAt(time);
  }
  return FilterNode::generateSample(time, channel);
}

int16_t DelayNode::filterSample(double time, int channel, int16_t sample)
{
  // No sample filtering by default, but a subclass could still override it
  return sample;
}

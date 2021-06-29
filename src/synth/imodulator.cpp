#include "imodulator.h"
#include "oscillator.h"

int IModulator::indexedParam(int param, uint8_t index)
{
  // handle character literal endianness
#if ('1110' & 0xFF) == '0'
  return (param & 0xFFFFFF00) | index;
#else
  return (param & 0x00FFFFFF) | (index << 24);
#endif
}

IModulator::IModulator(const SynthContext* ctx)
: AudioParamContainer(ctx), source(nullptr)
{
  // initializers only
}

IModulator::IModulator(std::shared_ptr<AudioNode> source)
: AudioParamContainer(source->ctx), source(source)
{
  // initializers only
}

void IModulator::install(std::shared_ptr<AudioNode> dest, int32_t route, uint8_t index)
{
  auto destParam = dest->param(route);
  if (!destParam) {
    return;
  }
  for (const auto& iter : params) {
    if (dest->param(iter.first)) {
      // don't overwrite existing parameters
      continue;
    }
    dest->addParam(indexedParam(iter.first, index), iter.second);
  }
  connectParams(dest, index);
  destParam->connect(source);
}

void IModulator::connectParams(std::shared_ptr<AudioNode> dest, uint8_t index)
{
  // default implementation does nothing
}

LFOModulator::LFOModulator(const SynthContext* ctx, uint64_t waveformID)
: IModulator(std::shared_ptr<AudioNode>(new DelayNode(std::shared_ptr<AudioNode>(BaseOscillator::create(ctx, waveformID, 1.0, 1.0)))))
{
  init();
}

LFOModulator::LFOModulator(std::shared_ptr<AudioNode> source)
: IModulator(std::shared_ptr<AudioNode>(new DelayNode(source)))
{
  init();
}

void LFOModulator::init()
{
  addParam(Frequency, source->param(BaseOscillator::Frequency));
  addParam(Depth, source->param(BaseOscillator::Gain));
  addParam(Delay, source->param(DelayNode::Delay));
}

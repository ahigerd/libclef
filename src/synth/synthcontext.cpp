#include "synthcontext.h"
#include "channel.h"
#include "iinterpolator.h"

SynthContext::SynthContext(double sampleRate)
: sampleRate(sampleRate), sampleTime(1.0 / sampleRate), interpolator(IInterpolator::get(IInterpolator::Zero))
{
  // initializers only
}

SynthContext::~SynthContext()
{
}

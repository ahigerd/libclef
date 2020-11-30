#include "synthcontext.h"
#include "channel.h"

SynthContext::SynthContext(double sampleRate)
: sampleRate(sampleRate), sampleTime(1.0 / sampleRate)
{
  // initializers only
}

SynthContext::~SynthContext()
{
}

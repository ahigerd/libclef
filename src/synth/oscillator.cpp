#include "oscillator.h"
#include "utility.h"
#include <cmath>
#include <iostream>

BaseOscillator* BaseOscillator::create(const SynthContext* ctx, uint64_t waveformID, double frequency, double amplitude, double pan)
{
  BaseOscillator* osc;
  switch(waveformID) {
    case 0:
      osc = new SquareOscillator(ctx, 0.5);
      break;
    case 1:
      osc = new SquareOscillator(ctx, 0.75);
      break;
    case 2:
      osc = new SquareOscillator(ctx, 0.25);
      break;
    case 3:
      osc = new SquareOscillator(ctx, 0.125);
      break;
    case 4:
      osc = new TriangleOscillator(ctx);
      break;
    case 5:
      osc = new NESNoiseOscillator(ctx);
      break;
    default:
      osc = new SineOscillator(ctx);
      break;
  }
  osc->param(Frequency)->setConstant(frequency);
  osc->param(Gain)->setConstant(amplitude);
  osc->param(Pan)->setConstant(pan);
  return osc;
}

BaseOscillator::BaseOscillator(const SynthContext* ctx)
: AudioNode(ctx), lastSample(0), lastTime(0), phase(0)
{
  frequency = addParam(Frequency, 440.0).get();
  addParam(Gain, 1.0);
  addParam(Pan, 0.5);
  pitchBend = addParam(PitchBend, 1.0).get();
}

bool BaseOscillator::isActive() const
{
  return true;
}

int16_t BaseOscillator::generateSample(double time, int channel)
{
  if (time != lastTime) {
    lastSample = std::round(calcSample(time) * 32767);
    double phaseOffset = (time - lastTime) * frequency->valueAt(time) * pitchBend->valueAt(time);
    phase = std::fmod(phase + phaseOffset, 1.0);
  }
  lastTime = time;
  return lastSample;
}

SineOscillator::SineOscillator(const SynthContext* ctx) : BaseOscillator(ctx)
{
}

double SineOscillator::calcSample(double)
{
  return fastSin(phase * M_PI * 2);
}

SquareOscillator::SquareOscillator(const SynthContext* ctx, double duty)
: BaseOscillator(ctx)
{
  dutyCycle = addParam(DutyCycle, duty).get();
}

double SquareOscillator::calcSample(double time)
{
  return phase < dutyCycle->valueAt(time) ? 1.0 : -1.0;
}

TriangleOscillator::TriangleOscillator(const SynthContext* ctx, int quantize, bool skew)
: BaseOscillator(ctx), quantize(quantize), skew(skew)
{
  // initializers only
}

double TriangleOscillator::calcSample(double)
{
  double lphase = phase * 2.0;
  double level = lphase < 1.0 ? lphase : 2.0 - lphase;
  if (quantize > 1) {
    level = std::floor(level * quantize) / double(quantize - 1);
    if (skew) {
      // This isn't physically accurate to how the hardware actually generates this,
      // which has a nonlinearity that's somewhat difficult to model accurately.
      if (phase < 0.5) {
        level = 1.0 - level;
        level = (level + level * level) * .5;
        level = 1.0 - level;
      } else {
        level = (level + level * level) * .5;
      }
    }
  }
  return level * 2.0 - 1.0;
}

NESNoiseOscillator::NESNoiseOscillator(const SynthContext* ctx)
: BaseOscillator(ctx), state(1), lastPhase(0)
{
  // initializers only
}

double NESNoiseOscillator::calcSample(double)
{
  if (lastPhase > phase) {
    bool bit = !(state & 0x0001) != !(state & 0x0002);
    state = (state >> 1) | (bit ? 0x4000 : 0);
  }
  lastPhase = phase;
  return (state & 0x0001) ? 1 : -1;
}

NESNoise93Oscillator::NESNoise93Oscillator(const SynthContext* ctx, uint16_t seed)
: BaseOscillator(ctx), state(seed), lastPhase(0)
{
  // initializers only
}

double NESNoise93Oscillator::calcSample(double)
{
  if (lastPhase > phase) {
    bool bit = !(state & 0x0001) != !(state & 0x0040);
    state = (state >> 1) | (bit ? 0x4000 : 0);
  }
  lastPhase = phase;
  return (state & 0x0001) ? 1 : -1;
}

GBNoiseOscillator::GBNoiseOscillator(const SynthContext* ctx)
: BaseOscillator(ctx), state(0x4000), lastPhase(0)
{
  // initializers only
}

double GBNoiseOscillator::calcSample(double)
{
  if (lastPhase > phase) {
    bool bit = state & 0x0001;
    state = (state >> 1) ^ (bit ? 0x6000 : 0);
  }
  lastPhase = phase;
  return (state & 0x0001) ? 1 : -1;
}

GBNoise127Oscillator::GBNoise127Oscillator(const SynthContext* ctx)
: BaseOscillator(ctx), state(0x40), lastPhase(0)
{
  // initializers only
}

double GBNoise127Oscillator::calcSample(double)
{
  if (lastPhase > phase) {
    bool bit = state & 0x0001;
    state = (state >> 1) ^ (bit ? 0x0060 : 0);
  }
  lastPhase = phase;
  return (state & 0x0001) ? 1 : -1;
}


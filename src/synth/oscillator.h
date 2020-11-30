#ifndef S2W_OSCILLATOR_H
#define S2W_OSCILLATOR_H

#include "audionode.h"

class BaseOscillator : public AudioNode {
public:
  enum ParamType {
    Frequency = 'freq',
    DutyCycle = 'duty',
  };

  static BaseOscillator* create(const SynthContext* ctx, uint64_t waveformID, double frequency = 440.0, double amplitude = 1.0, double pan = 0.5);

  virtual bool isActive() const;
  virtual int16_t generateSample(double time, int channel = 0);

protected:
  BaseOscillator(const SynthContext* ctx);

  int16_t lastSample;
  double lastTime;
  double phase;

  virtual double calcSample(double time) = 0;
};

class SineOscillator : public BaseOscillator {
public:
  SineOscillator(const SynthContext* ctx);

protected:
  virtual double calcSample(double time);
};

class SquareOscillator : public BaseOscillator {
public:
  SquareOscillator(const SynthContext* ctx, double duty = 0.5);

protected:
  virtual double calcSample(double time);
};

class TriangleOscillator : public BaseOscillator {
public:
  TriangleOscillator(const SynthContext* ctx, int quantize = 0, bool skew = false);

  int quantize;
  bool skew;

protected:
  virtual double calcSample(double time);
};

class NESNoiseOscillator : public BaseOscillator {
public:
  NESNoiseOscillator(const SynthContext* ctx);

protected:
  virtual double calcSample(double time);

private:
  uint16_t state;
  double lastPhase;
};

class NESNoise93Oscillator : public BaseOscillator {
public:
  NESNoise93Oscillator(const SynthContext* ctx, uint16_t seed = 1);

protected:
  virtual double calcSample(double time);

private:
  uint16_t state;
  double lastPhase;
};

#endif

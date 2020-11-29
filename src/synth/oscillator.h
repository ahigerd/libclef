#ifndef S2W_OSCILLATOR_H
#define S2W_OSCILLATOR_H

#include "audionode.h"

class BaseOscillator : public AudioNode {
public:
  static BaseOscillator* create(uint64_t waveformID, double frequency = 440.0, double amplitude = 1.0, double pan = 0.5);

  AudioParameter frequency;

  virtual bool isActive() const;
  virtual int16_t generateSample(double time, int channel = 0);

protected:
  BaseOscillator();

  int16_t lastSample;
  double lastTime;
  double phase;

  virtual double calcSample(double time) = 0;
};

class SineOscillator : public BaseOscillator {
protected:
  virtual double calcSample(double time);
};

class SquareOscillator : public BaseOscillator {
public:
  SquareOscillator(double duty = 0.5);

  AudioParameter dutyCycle;

protected:
  virtual double calcSample(double time);
};

class TriangleOscillator : public BaseOscillator {
public:
  TriangleOscillator(int quantize = 0, bool skew = false);

  int quantize;
  bool skew;

protected:
  virtual double calcSample(double time);
};

class NESNoiseOscillator : public BaseOscillator {
public:
  NESNoiseOscillator();

protected:
  virtual double calcSample(double time);

private:
  uint16_t state;
  double lastPhase;
};

class NESNoise93Oscillator : public BaseOscillator {
public:
  NESNoise93Oscillator(uint16_t seed = 1);

protected:
  virtual double calcSample(double time);

private:
  uint16_t state;
  double lastPhase;
};

#endif

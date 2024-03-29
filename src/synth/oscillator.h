#ifndef CLEF_OSCILLATOR_H
#define CLEF_OSCILLATOR_H

#include "audionode.h"

class BaseOscillator : public AudioNode {
public:
  enum WaveformPreset {
    Sine,
    Triangle,
    NESTriangle,
    Sawtooth,
    Square50,
    Square75,
    Square25,
    Square125,
    Noise,
    LinearNoise,
    CosineNoise,
    NESNoise,
    NESNoise93,
    GBNoise,
    GBNoise127,
    NumPresets,
  };

  enum ParamType {
    Frequency = 'freq',
    DutyCycle = 'duty',
    PitchBend = 'bend',
    DCOffset = 'dc  ',
  };

  static BaseOscillator* create(const SynthContext* ctx, WaveformPreset waveform, double frequency = 440.0, double amplitude = 1.0, double pan = 0.5);

  virtual bool isActive() const;
  virtual int16_t generateSample(double time, int channel = 0);

protected:
  BaseOscillator(const SynthContext* ctx);

  int16_t lastSample;
  double lastTime;
  double phase;
  AudioParam* frequency;
  AudioParam* pitchBend;

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

private:
  AudioParam* dutyCycle;
};

class SawtoothOscillator : public BaseOscillator {
public:
  SawtoothOscillator(const SynthContext* ctx);

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

class NoiseOscillator : public BaseOscillator {
public:
  enum Smoothing {
    None,
    Linear,
    Cosine,
  };
  NoiseOscillator(const SynthContext* ctx, Smoothing smoothing = None);

  Smoothing smoothing;

protected:
  virtual double calcSample(double time);

private:
  double lastPhase;
  double lastLevel;
  double nextLevel;
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

class GBNoiseOscillator : public BaseOscillator {
public:
  GBNoiseOscillator(const SynthContext* ctx);

protected:
  virtual double calcSample(double time);

private:
  uint16_t state;
  double lastPhase;
};

class GBNoise127Oscillator : public BaseOscillator {
public:
  GBNoise127Oscillator(const SynthContext* ctx);

protected:
  virtual double calcSample(double time);

private:
  uint16_t state;
  double lastPhase;
};

#endif

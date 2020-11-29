#ifndef S2W_IINTERPOLATOR_H
#define S2W_IINTERPOLATOR_H

#include <vector>
#include <cstdint>
class SampleData;

class IInterpolator
{
public:
  enum InterpolationMode {
    Zero,
    Linear,
    Cosine,
    Sharp,
    Sinc,
    Lagrange4,
    Lagrange6,
  };
  static IInterpolator* get(InterpolationMode mode);

  virtual ~IInterpolator() {}

  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0, double sampleStep = 0) const = 0;
};

class ZeroInterpolator : public IInterpolator
{
public:
  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0, double sampleStep = 0) const;
};

class LinearInterpolator : public IInterpolator
{
public:
  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0, double sampleStep = 0) const;
};

class CosineInterpolator : public IInterpolator
{
public:
  enum {
    Resolution = 8192
  };

  CosineInterpolator();

  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0, double sampleStep = 0) const;

private:
  double lut[Resolution];
};

class SharpInterpolator : public IInterpolator
{
public:
  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0, double sampleStep = 0) const;
};

class SincInterpolator : public IInterpolator
{
public:
  enum {
    Resolution = 8192,
    Width = 8,
    Samples = Resolution * Width
  };

  SincInterpolator();

  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0, double sampleStep = 0) const;

private:
  double sincLut[Samples + 1];
  double windowLut[Samples + 1];
};

class Lagrange4Interpolator : public IInterpolator
{
public:
  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0, double sampleStep = 0) const;
};

class Lagrange6Interpolator : public IInterpolator
{
public:
  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0, double sampleStep = 0) const;
};

#endif

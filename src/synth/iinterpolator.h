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
    // TODO: more
  };
  static IInterpolator* get(InterpolationMode mode);

  virtual ~IInterpolator() {}

  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0) const = 0;
};

class ZeroInterpolator : public IInterpolator
{
public:
  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0) const;
};

class LinearInterpolator : public IInterpolator
{
public:
  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0) const;
};

class CosineInterpolator : public IInterpolator
{
public:
  CosineInterpolator();

  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0) const;

private:
  double lut[8192];
};

class SharpInterpolator : public IInterpolator
{
public:
  virtual int16_t interpolate(const SampleData* data, double time, int channel = 0) const;
};


#endif

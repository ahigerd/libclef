#ifndef S2W_UTILITY_H
#define S2W_UTILITY_H

#include <cstdint>

template <typename T, typename T2>
inline T clamp(T2 value, T lower, T upper)
{
  return value < lower ? lower : (value > upper ? upper : value);
}

template <typename T>
inline T lerp(T left, T right, double weight)
{
  return weight * (right - left) + left;
}

template <typename T>
inline T lerp(T left, T right, double time, double t0, double t1)
{
  return (time - t0) / (t1 - t0) * (right - left) + left;
}

template <typename T, typename Container>
T parseInt(const Container& buffer, int offset)
{
  uint64_t result = 0;
  for (int i = sizeof(T) - 1; i >= 0; --i) {
    result = (result << 8) | uint8_t(buffer[offset + i]);
  }
  return T(result);
}

template <typename T, typename Container>
T parseIntBE(const Container& buffer, int offset)
{
  uint64_t result = 0;
  for (int i = 0; i < sizeof(T); i++) {
    result = (result << 8) | uint8_t(buffer[offset + i]);
  }
  return T(result);
}

int countBits(uint64_t value);

double noteToFreq(double midiNote);

#endif

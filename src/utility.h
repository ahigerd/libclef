#ifndef S2W_UTILITY_H
#define S2W_UTILITY_H

#include "s2wconfig.h"
#include <math.h>
#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include <iostream>
#include <vector>
#include <cmath>
#include <cctype>

#if defined(__GNUC__) || defined(__clang__)
#define force_inline [[gnu::always_inline]] [[gnu::gnu_inline]] inline
#elif defined(_MSC_VER)
#pragma warning(error: 4714)
#define force_inline __forceinline
#else
#define force_inline inline
#endif

using OpenFn = std::function<std::unique_ptr<std::istream>(const std::string&)>;
std::unique_ptr<std::istream> openFstream(const std::string& path);

using Iter8 = std::vector<uint8_t>::const_iterator;

template <typename T, typename T2>
force_inline T clamp(T2 value, T lower, T upper)
{
  return value < lower ? lower : (value > upper ? upper : value);
}

template <typename T>
force_inline T lerp(T left, T right, double weight)
{
  return weight * (right - left) + left;
}

template <typename T>
force_inline T lerp(T left, T right, double time, double t0, double t1)
{
  return (time - t0) / (t1 - t0) * (right - left) + left;
}

template <typename T, typename Container>
T parseInt(const Container& buffer, size_t offset)
{
  uint64_t result = 0;
  for (ssize_t i = sizeof(T) - 1; i >= 0 && i < sizeof(T); --i) {
    result = (result << 8) | uint8_t(buffer[offset + i]);
  }
  return T(result);
}

template <typename T, typename Container>
T parseIntBE(const Container& buffer, size_t offset)
{
  uint64_t result = 0;
  for (ssize_t i = 0; i < ssize_t(sizeof(T)); i++) {
    result = (result << 8) | uint8_t(buffer[offset + i]);
  }
  return T(result);
}

int countBits(uint64_t value);

double noteToFreq(double midiNote);
double fastExp(double r, double dt = 1.0);
double fastSin(double theta);
double fastSin1(double phase);
double fasterSin(double theta);
double fasterSin1(double phase);
force_inline double fastCos(double theta) { return fastSin(M_PI_2 - theta); }
force_inline double combinePan(double a, double b) {
  return clamp(a + b - 0.5, 0.0, 1.0);
}

std::string trim(const std::string& str);
std::wstring toUtf16(const std::string& str);
std::string toUtf8(const std::wstring& str);
std::string formatDuration(double seconds);
std::string formatDuration(const std::string& seconds);
std::string fourccToString(uint32_t magic);

#endif

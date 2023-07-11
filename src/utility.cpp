#include "utility.h"
#include <codecvt>
#include <locale>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>

#if __cplusplus < 201703L
namespace std { using namespace std::experimental; }
#endif

std::unique_ptr<std::istream> openFstream(const std::string& path)
{
  return std::unique_ptr<std::istream>(new std::ifstream(std::filesystem::u8path(path), std::ios::in | std::ios::binary));
}

double noteToFreq(double midiNote)
{
  return 440.0 * std::pow(2.0, (midiNote - 69) / 12.0);
}

int countBits(uint64_t value)
{
  int result = 0;
  while (value) {
    // Writing it this way will auto-optimize to use intrinsics when available
    value &= value - 1;
    result++;
  }
  return result;
}

std::string trim(const std::string& str)
{
  auto begin = str.begin(), end = str.end();
  while (begin != end && std::isspace(*begin)) {
    begin++;
  }
  while (end != begin && std::isspace(*end)) {
    end--;
  }
  return std::string(begin, end);
}

std::wstring toUtf16(const std::string& str)
{
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> utf16Codec;
  return utf16Codec.from_bytes(str);
}

std::string toUtf8(const std::wstring& str)
{
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> utf16Codec;
  return utf16Codec.to_bytes(str);
}

std::string formatDuration(double seconds)
{
  if (seconds < 0) {
    return std::string();
  }
  std::ostringstream ss;
  int ms = int(seconds * 1000) % 1000;
  int sec = int(seconds) % 60;
  int min = int(seconds / 60) % 60;
  int hr = int(seconds / 3600);
  if (hr) {
    ss << hr << ":";
    if (min < 10) {
      ss << "0";
    }
  }
  ss << min << ":" << std::setw(2) << std::setfill('0') << sec << "." << std::setw(3) << ms;
  return ss.str();
}

std::string formatDuration(const std::string& seconds)
{
  std::istringstream ss(seconds);
  double secondsVal = -1;
  ss >> secondsVal;
  return formatDuration(secondsVal);
}

double fastExp(double r, double dt)
{
  static double table[2048];
  static double interp[2048];
  static bool initialized = false;
  if (!initialized) {
    // Precompute e^x between -10.24 and +10.23
    for (int i = -1024; i < 1024; i++) {
      table[i + 1024] = std::exp(i * 0.01);
    }
    // Precompute deltas
    for (int i = 0; i < 2047; i++) {
      interp[i] = table[i] - table[i + 1];
    }
    interp[2047] = table[2047];
    initialized = true;
  }
  double pos = (r * dt * 100 + 1024);
  int idx = int(pos);

  // clamp results to appropriate range
  if (r < 0) {
    if (idx < 0) return HUGE_VAL;
    if (idx > 1023) return 1;
  } else {
    if (idx < 1024) return 1;
    if (idx > 2047) return 0;
  }

  // linear interpolation
  return table[idx] + interp[idx] * (idx - pos);
}

double fastSin(double theta)
{
  constexpr int tableSize = M_PI * 2 * 1000;
  static bool init = false;
  static double table[tableSize + 1];
  if (!init) {
    for (int i = 0; i < tableSize; i++) {
      table[i] = std::sin(i * 0.001);
    }
    table[tableSize] = 0;
    init = true;
  }
  if (theta < 0) {
    return -fastSin(-theta);
  }
  theta *= 1000;
  int pos = theta;
  double frac = theta - pos;
  pos = pos % tableSize;
  return lerp(table[pos], table[pos + 1], frac);
}

std::string fourccToString(uint32_t magic)
{
  char str[4] = {
    char((magic >> 24) & 0xFF),
    char((magic >> 16) & 0xFF),
    char((magic >> 8) & 0xFF),
    char((magic >> 0) & 0xFF),
  };
  return std::string(str, 4);
}

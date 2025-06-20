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
  std::ifstream* stream = new std::ifstream();
  try {
    stream->exceptions(std::ifstream::failbit);
    stream->open(std::filesystem::u8path(path), std::ios::in | std::ios::binary);
    stream->exceptions(std::ifstream::goodbit);
    return std::unique_ptr<std::istream>(stream);
  } catch (...) {
    delete stream;
    throw;
  }
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
    if (idx < 0) return 0;
    if (idx > 1023) return 1;
  } else {
    if (idx < 1024) return 1;
    if (idx > 2047) return HUGE_VAL;
  }

  // linear interpolation
  return table[idx] + interp[idx] * (idx - pos);
}

namespace {
  struct SinTable {
    enum { bits = 13, size = 1 << bits, mask = size - 1 };

    double table[size];
    double interp[size];

    SinTable() {
      static constexpr double unit = M_PI * 2 / size;
      for (int i = 0; i < size; i++) {
        table[i] = std::sin(i * unit);
      }
      for (int i = 0; i < size - 1; i++) {
        interp[i] = table[i + 1] - table[i];
      }
      interp[size - 1] = -table[size - 1];
    }

    inline double operator[](int x) const {
      return table[x];
    }

    inline double operator[](double x) const {
      return table[int(x) & mask];
    }

    inline double interpolate(int x, double frac) const {
      return table[x] + (interp[x] * frac);
    }
  };

  static constexpr double sinTableFactor = SinTable::size / (M_PI * 2);
  static SinTable sinTable;
}

double fastSin(double theta)
{
  if (theta < 0) {
    return -fastSin(-theta);
  }
  theta *= sinTableFactor;
  int pos = theta;
  double frac = theta - pos;
  pos = pos & SinTable::mask;
  return sinTable.interpolate(pos, frac);
}

double fastSin1(double phase)
{
  if (phase < 0) {
    return -fastSin1(-phase);
  }
  int pos = phase * SinTable::size;
  double frac = phase - pos;
  pos = pos & SinTable::mask;
  return sinTable.interpolate(pos, frac);
}

double fasterSin(double theta)
{
  if (theta < 0) {
    return -sinTable[-theta * sinTableFactor];
  }
  return sinTable[theta * sinTableFactor];
}

double fasterSin1(double phase)
{
  if (phase < 0) {
    return -sinTable[-phase * SinTable::size];
  }
  return sinTable[phase * SinTable::size];
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

void hexdump(const void* _buffer, int size)
{
  const char* buffer = reinterpret_cast<const char*>(_buffer);
  int offset = 0;
  while (offset < size) {
    int lineStart = offset;
    int i;
    std::string printable;
    std::printf("%04x: ", uint32_t(offset));
    for (i = 0; i < 16; i++) {
      if (offset < size) {
        std::printf("%02x ", uint32_t(uint8_t(buffer[offset])));
        printable += char((buffer[offset] >= 0x20 && buffer[offset] < 0x7F) ? buffer[offset] : '.');
      } else {
        std::printf("   ");
      }
      ++offset;
    }
    std::printf("%s\n", printable.c_str());
  }
}

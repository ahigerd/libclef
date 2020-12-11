#include "utility.h"
#include <cmath>
#include <cctype>
#include <codecvt>
#include <locale>
#include <sstream>
#include <iomanip>

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

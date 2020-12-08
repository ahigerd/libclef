#include "utility.h"
#include <cmath>

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

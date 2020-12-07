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

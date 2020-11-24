#ifndef S2W_IMODULATOR_H
#define S2W_IMODULATOR_H

#include <cstdint>
struct PlaybackInfo;

class IModulator {
public:
  virtual int16_t modulate(PlaybackInfo* playback, double time) = 0;
};

#endif

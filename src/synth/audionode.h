#ifndef S2W_AUDIONODE_H
#define S2W_AUDIONODE_H

#include <cstdint>

class AudioNode {
public:
  virtual ~AudioNode() {}

  virtual int16_t getSample(double time) = 0;
};

#endif

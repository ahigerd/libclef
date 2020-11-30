#ifndef S2W_AUDIONODE_H
#define S2W_AUDIONODE_H

#include <cstdint>
#include <unordered_map>
#include "audioparam.h"

class AudioNode {
public:
  virtual ~AudioNode() {}

  AudioParam gain, pan, trigger;

  virtual bool isActive() const = 0;
  virtual int16_t getSample(double time, int channel = 0);

protected:
  AudioNode();

  virtual int16_t generateSample(double time, int channel = 0) = 0;
};

class FilterNode : public AudioNode {
public:
  void connect(std::shared_ptr<AudioNode> source);

  virtual bool isActive() const;

protected:
  virtual int16_t generateSample(double time, int channel = 0);
  virtual int16_t filterSample(double time, int channel, int16_t sample) = 0;
  std::shared_ptr<AudioNode> source;
};

#endif

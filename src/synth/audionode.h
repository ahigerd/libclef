#ifndef S2W_AUDIONODE_H
#define S2W_AUDIONODE_H

#include "s2wconfig.h"
#include <unordered_map>
#include "audioparam.h"
class SynthContext;

class AudioNode {
public:
  enum ParamType {
    Gain = 'gain',
    Pan = 'pan ',
    Trigger = 'trig',
  };

  virtual ~AudioNode() {}

  std::shared_ptr<AudioParam> param(int32_t key) const;
  void addParam(int32_t key, double initialValue);
  void addParam(int32_t key, std::shared_ptr<AudioParam> param);
  double paramValue(int32_t key, double time, double defaultValue = 0) const;

  virtual bool isActive() const = 0;
  virtual int16_t getSample(double time, int channel = 0);

  const SynthContext* const ctx;

protected:
  AudioNode(const SynthContext* ctx);

  virtual int16_t generateSample(double time, int channel = 0) = 0;

private:
  friend class FilterNode;
  std::unordered_map<int32_t, std::shared_ptr<AudioParam>> params;
};

class FilterNode : public AudioNode {
public:
  void connect(std::shared_ptr<AudioNode> source);

  virtual bool isActive() const;

protected:
  FilterNode(const SynthContext* ctx);

  virtual int16_t generateSample(double time, int channel = 0);
  virtual int16_t filterSample(double time, int channel, int16_t sample) = 0;
  std::shared_ptr<AudioNode> source;
};

#endif

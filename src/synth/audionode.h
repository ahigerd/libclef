#ifndef S2W_AUDIONODE_H
#define S2W_AUDIONODE_H

#include "s2wconfig.h"
#include "audioparam.h"
class SynthContext;
class IModulator;

class AudioNode : public AudioParamContainer {
public:
  enum ParamType {
    Gain = 'gain',
    Pan = 'pan ',
    Trigger = 'trig',
  };

  virtual ~AudioNode() {}

  std::shared_ptr<IModulator> modulator(int32_t key) const;
  void addModulator(int32_t key, std::shared_ptr<IModulator> mod);
  void removeModulator(int32_t key);
  void removeModulator(std::shared_ptr<IModulator> mod);

  virtual bool isActive() const = 0;
  virtual int16_t getSample(double time, int channel = 0);

protected:
  AudioNode(const SynthContext* ctx);

  virtual void onParamAdded(int32_t key, std::shared_ptr<AudioParam>& param);
  virtual int16_t generateSample(double time, int channel = 0) = 0;

private:
  friend class FilterNode;
  std::map<int32_t, std::shared_ptr<IModulator>> mods;
  std::shared_ptr<AudioParam>* pGain;
  std::shared_ptr<AudioParam>* pPan;
};

class FilterNode : public AudioNode {
public:
  void connect(std::shared_ptr<AudioNode> source);

  virtual bool isActive() const;

protected:
  FilterNode(const SynthContext* ctx);
  FilterNode(std::shared_ptr<AudioNode> source);

  virtual int16_t generateSample(double time, int channel = 0);
  virtual int16_t filterSample(double time, int channel, int16_t sample) = 0;
  std::shared_ptr<AudioNode> source;
  AudioParam* pTrigger;
};

class DelayNode : public FilterNode {
public:
  enum ParamType {
    Delay = 'dela',
    Initial = 'init',
  };

  DelayNode(const SynthContext* ctx, double delay = 0, double initial = 0);
  DelayNode(std::shared_ptr<AudioNode> source, double delay = 0, double initial = 0);

protected:
  virtual int16_t generateSample(double time, int channel = 0);
  virtual int16_t filterSample(double time, int channel, int16_t sample);

private:
  void init(double delay, double initial);
  AudioParam* pDelay;
  AudioParam* pInitial;
};

#endif

#ifndef S2W_IMODULATOR_H
#define S2W_IMODULATOR_H

#include "audionode.h"
#include "iinterpolator.h"
#include <vector>

class IModulator : public AudioParamContainer {
public:
  static int indexedParam(int param, uint8_t index);

  virtual ~IModulator() {}

  void install(std::shared_ptr<AudioNode> dest, int32_t route, uint8_t index = 0);

protected:
  IModulator(const SynthContext* ctx);
  IModulator(std::shared_ptr<AudioNode> source);

  virtual void connectParams(std::shared_ptr<AudioNode> dest, uint8_t index);

  std::shared_ptr<AudioNode> source;
};

class LFOModulator : public IModulator {
public:
  enum ParamType {
    Frequency = 'Lfr\0',
    Depth = 'Ldp\0',
    Delay = 'Ldl\0',
    // TODO: Fade = 'Lfd\0',
  };

  LFOModulator(const SynthContext* ctx, uint64_t waveformID);
  LFOModulator(std::shared_ptr<AudioNode> source);

protected:
  virtual void connectParams(std::shared_ptr<AudioNode> dest);

private:
  void init();
};

#endif

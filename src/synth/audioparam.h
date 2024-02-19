#ifndef CLEF_AUDIOPARAM_H
#define CLEF_AUDIOPARAM_H

#include "clefconfig.h"
#include <memory>
#include <map>
class AudioNode;
class SynthContext;

class AudioParam {
public:
  AudioParam(const SynthContext* ctx, double initialValue = 1.0);
  AudioParam(std::shared_ptr<AudioNode> source, double scale = 1/8192.0, double offset = 0);
  AudioParam(std::shared_ptr<AudioParam> source, double scale = 1.0, double offset = 0);

  double valueAt(double time) const;
  void setConstant(double value);
  void setValueAt(double time, double value);
  void connect(std::shared_ptr<AudioNode> source, double scale = 1/8192.0, double offset = 0);
  void connect(std::shared_ptr<AudioParam> source, double scale = 1.0, double offset = 0);

  inline AudioParam& operator=(double value) {
    setConstant(value);
    return *this;
  }

  inline double operator()(double time) const {
    return valueAt(time);
  }

  const SynthContext* const ctx;

private:
  std::shared_ptr<AudioNode> sourceNode;
  std::shared_ptr<AudioParam> sourceParam;
  double scale;
  double constant;
};

class AudioParamContainer {
public:
  std::shared_ptr<AudioParam> param(int32_t key) const;
  std::shared_ptr<AudioParam> addParam(int32_t key, double initialValue);
  void addParam(int32_t key, std::shared_ptr<AudioParam> param);
  double paramValue(int32_t key, double time, double defaultValue = 0) const;

  const SynthContext* const ctx;

protected:
  AudioParamContainer(const SynthContext* ctx);

  virtual void onParamAdded(int32_t key, std::shared_ptr<AudioParam>& param);

  std::map<int32_t, std::shared_ptr<AudioParam>> params;
};

#endif

#ifndef S2W_AUDIONODE_H
#define S2W_AUDIONODE_H

#include <cstdint>
#include <vector>

class AudioNode;

class AudioParameter {
public:
  AudioParameter(double initialValue = 1.0);
  AudioParameter(AudioNode* source, double scale = 1/8192.0, double offset = 0);
  AudioParameter(AudioParameter* source, double scale = 1.0, double offset = 0);

  double valueAt(double time) const;
  void setConstant(double value);
  void connect(AudioNode* source, double scale = 1/8192.0, double offset = 0);
  void connect(AudioParameter* source, double scale = 1.0, double offset = 0);

  inline AudioParameter& operator=(double value) {
    setConstant(value);
    return *this;
  }

  inline double operator()(double time) const {
    return valueAt(time);
  }

  AudioNode* source;
  AudioParameter* sourceParam;
  double scale;
  double constant;
};

class AudioNode {
public:
  virtual ~AudioNode() {}

  AudioParameter gain, pan;

  virtual bool isActive() const = 0;
  int16_t getSample(double time, int channel = 0);

protected:
  AudioNode();

  virtual int16_t generateSample(double time, int channel = 0) = 0;
};

#endif

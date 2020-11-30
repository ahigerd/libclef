#ifndef S2W_SYNTHCONTEXT_H
#define S2W_SYNTHCONTEXT_H

#include <cstdint>
#include <vector>
#include <memory>
struct Channel;

struct SynthContext {
  SynthContext(double sampleRate);
  ~SynthContext();

  double sampleRate;
  double sampleTime;
  std::vector<std::unique_ptr<Channel>> channels;
};

#endif

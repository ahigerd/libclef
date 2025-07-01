#ifndef CLEF_AUTOMATIONNODE_H
#define CLEF_AUTOMATIONNODE_H

#include "audionode.h"
#include <vector>

class AutomationNode : public AudioNode {
public:
  AutomationNode(const SynthContext* ctx, double initialValue = 0);

  bool isActive() const;
  int16_t getSample(double time, int channel = 0) override;

  void addTransition(AudioParam::Transition transition, double time, double value);
  inline void setValueAt(double time, double value) { addTransition(AudioParam::Step, time, value); }
  inline void linearTo(double time, double value) { addTransition(AudioParam::Linear, time, value); }
  inline void cosineTo(double time, double value) { addTransition(AudioParam::Cosine, time, value); }
  inline void easeInTo(double time, double value) { addTransition(AudioParam::EaseIn, time, value); }
  inline void easeOutTo(double time, double value) { addTransition(AudioParam::EaseOut, time, value); }

protected:
  // Unused: overrides getSample instead
  virtual int16_t generateSample(double time, int channel = 0) { return 0; }

private:
  double initialValue;
  struct Endpoint {
    double time;
    double value;
    AudioParam::Transition transition;
  };
  std::vector<Endpoint> phases;
  int phase;
  double previousTime;
};

#endif

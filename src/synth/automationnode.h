#ifndef S2W_AUTOMATIONNODE_H
#define S2W_AUTOMATIONNODE_H

#include "audionode.h"
#include <vector>

class AutomationNode : public AudioNode {
public:
  AutomationNode(const SynthContext* ctx, double initialValue = 0);

  bool isActive() const;
  int16_t getSample(double time, int channel = 0);

  inline void setValueAt(double time, double value) { addTransition(Step, time, value); }
  inline void linearTo(double time, double value) { addTransition(Linear, time, value); }
  inline void cosineTo(double time, double value) { addTransition(Cosine, time, value); }
  inline void easeInTo(double time, double value) { addTransition(EaseIn, time, value); }
  inline void easeOutTo(double time, double value) { addTransition(EaseOut, time, value); }

protected:
  // Unused: overrides getSample instead
  virtual int16_t generateSample(double time, int channel = 0) { return 0; }

private:
  double initialValue;
  enum Transition {
    Step,
    Linear,
    Cosine,
    EaseIn,
    EaseOut,
  };
  struct Endpoint {
    double time;
    double value;
    Transition transition;
  };
  std::vector<Endpoint> phases;
  int phase;
  void addTransition(Transition transition, double time, double value);
};

#endif

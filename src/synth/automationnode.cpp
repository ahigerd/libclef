#include "automationnode.h"
#include "utility.h"
#include <iostream>

AutomationNode::AutomationNode(const SynthContext* ctx, double initialValue)
: AudioNode(ctx), initialValue(initialValue), phase(0)
{
  // initializers only
}

bool AutomationNode::isActive() const
{
  return true;
}

int16_t AutomationNode::generateSample(double time, int)
{
  if (!phases.size()) {
    return initialValue;
  }
  double startTime = 0, startValue = initialValue;
  while (phase < phases.size()) {
    const Endpoint& endPhase = phases[phase];
    double endTime = endPhase.time, endValue = endPhase.value;
    if (time > endTime) {
      ++phase;
      continue;
    }

    if (phase > 0) {
      const Endpoint& startPhase = phases[phase - 1];
      startTime = startPhase.time;
      startValue = startPhase.value;
    }
    double timeStep = (time - startTime) / (endTime - startTime);
    switch (endPhase.transition) {
      case Step:
        return startValue;
      case Linear:
        return lerp(startValue, endValue, timeStep);
      case Cosine:
        return lerp(startValue, endValue, fastCos(timeStep * M_PI) * -0.5 + 0.5);
      case EaseIn:
        return lerp(startValue, endValue, timeStep * timeStep * timeStep);
      case EaseOut:
        timeStep = 1.0 - timeStep;
        return lerp(startValue, endValue, 1.0 - timeStep * timeStep * timeStep);
    }
  }
  return phases.back().value;
}

void AutomationNode::addTransition(Transition transition, double time, double value)
{
  if (time <= 0) {
    phases.clear();
    phase = 0;
    initialValue = value;
    return;
  }
  while (phases.size() > 0 && time < phases.back().time) {
    phases.pop_back();
  }
  phases.push_back(Endpoint{ time, value, transition });
}

#include "automationnode.h"
#include "utility.h"
#include <iostream>

AutomationNode::AutomationNode(const SynthContext* ctx, double initialValue)
: AudioNode(ctx), initialValue(initialValue), phase(0), previousTime(-1)
{
  // initializers only
}

bool AutomationNode::isActive() const
{
  return true;
}

int16_t AutomationNode::getSample(double time, int ch)
{
  if (!phases.size()) {
    return initialValue;
  }
  if (time < previousTime) {
    phase = 0;
  }
  previousTime = time;
  double startTime = 0, startValue = initialValue;
  while (phase < phases.size()) {
    const Endpoint& endPhase = phases[phase];
    double endTime = endPhase.time, endValue = endPhase.value;
    if (time >= endTime) {
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
      case AudioParam::Step:
        return startValue;
      case AudioParam::Linear:
        return lerp(startValue, endValue, timeStep);
      case AudioParam::Cosine:
        return lerp(startValue, endValue, fastCos(timeStep * M_PI) * -0.5 + 0.5);
      case AudioParam::EaseIn:
        return lerp(startValue, endValue, timeStep * timeStep * timeStep);
      case AudioParam::EaseOut:
        timeStep = 1.0 - timeStep;
        return lerp(startValue, endValue, 1.0 - timeStep * timeStep * timeStep);
    }
  }
  return phases.back().value;
}

void AutomationNode::addTransition(AudioParam::Transition transition, double time, double value)
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
  phase = 0;
}

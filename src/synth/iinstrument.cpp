#include "iinstrument.h"
#include "channel.h"
#include "sampler.h"
#include "oscillator.h"
#include "synthcontext.h"
#include "clefcontext.h"
#include "seq/sequenceevent.h"
#include <iostream>

std::vector<int32_t> IInstrument::supportedChannelParams() const
{
  return { AudioNode::Gain, AudioNode::Pan };
}

std::vector<int32_t> IInstrument::supportedNoteParams() const
{
  return { AudioNode::Gain, AudioNode::Pan };
}

void IInstrument::channelEvent(Channel* channel, std::shared_ptr<ChannelEvent> event)
{
  // TODO: more types
  auto param = channel->param(event->param);
  if (param) {
    if (event->transitionDuration > 0) {
      param->setValueAt(event->timestamp, param->valueAt(event->timestamp));
      param->addTransition(event->transition, event->timestamp + event->transitionDuration, event->value);
    } else {
      param->setValueAt(event->timestamp, event->value);
    }
  }
}

void IInstrument::modulatorEvent(Channel* channel, std::shared_ptr<ModulatorEvent> event)
{
  if (event->playbackID) {
    auto noteIter = channel->notes.find(event->playbackID);
    if (noteIter != channel->notes.end()) {
      auto param = noteIter->second->source->param(event->param);
      if (param) {
        if (event->transitionDuration >= 0) {
          if (event->transitionDuration > 0 && event->transition != AudioParam::Step) {
            param->setValueAt(event->timestamp, param->valueAt(event->timestamp));
          }
          param->addTransition(event->transition, event->timestamp + event->transitionDuration, event->value);
        } else {
          param->setConstant(event->value);
        }
      }
    }
  } else {
    for (auto& pair : channel->notes) {
      auto param = pair.second->source->param(event->param);
      if (param) {
        if (event->transitionDuration >= 0) {
          if (event->transitionDuration > 0 && event->transition != AudioParam::Step) {
            param->setValueAt(event->timestamp, param->valueAt(event->timestamp));
          }
          param->addTransition(event->transition, event->timestamp + event->transitionDuration, event->value);
        } else {
          param->setConstant(event->value);
        }
      }
    }
  }
}

Channel::Note* IInstrument::userEvent(Channel* channel, std::shared_ptr<SequenceEvent> event)
{
  std::cerr << "WARNING: unhandled user event" << std::endl;
  return nullptr;
}

std::string IInstrument::displayName() const
{
  return std::string();
}

Channel::Note* DefaultInstrument::noteEvent(Channel* channel, std::shared_ptr<BaseNoteEvent> event)
{
  std::shared_ptr<AudioNode> node;
  double duration = event->duration;
  if (AudioNodeEvent* nodeEvent = event->cast<AudioNodeEvent>()) {
    node = nodeEvent->node;
  } else if (OscillatorEvent* oscEvent = event->cast<OscillatorEvent>()) {
    BaseOscillator* osc = BaseOscillator::create(channel->ctx, BaseOscillator::WaveformPreset(oscEvent->waveformID), oscEvent->frequency, oscEvent->volume, oscEvent->pan);
    duration = oscEvent->duration;
    node.reset(osc);
  } else if (SampleEvent* sampEvent = event->cast<SampleEvent>()) {
    SampleData* sampleData = channel->ctx->clefContext()->getSample(sampEvent->sampleID);
    if (!sampleData) {
      std::cerr << "ERROR: sample " << std::hex << sampEvent->sampleID << std::dec << " not found" << std::endl;
      return nullptr;
    }
    Sampler* samp = new Sampler(channel->ctx, sampleData, sampEvent->pitchBend, sampEvent->modPitchBend);
    node.reset(samp);
    samp->param(AudioNode::Gain)->setConstant(sampEvent->volume);
    samp->param(AudioNode::Pan)->setConstant(sampEvent->pan);
    if (!duration) {
      duration = sampleData->duration();
    }
  } else {
    std::cerr << "ERROR: unhandled instrument or user event" << std::endl;
    return nullptr;
  }
  Channel::Note* note = channel->allocNote(event, node, duration);
  if (event->useEnvelope) {
    applyEnvelope(channel, note);
  }
  return note;
}

void DefaultInstrument::applyEnvelope(Channel* channel, Channel::Note* note)
{
  const auto& event = note->event;
  Envelope* env = new Envelope(channel->ctx, event->attack, event->hold, event->decay, event->sustain, event->fade, event->release);
  env->expAttack = event->expAttack;
  env->expDecay = event->expDecay;
  env->param(Envelope::StartGain)->setConstant(event->startGain);
  env->connect(note->source, true);
  note->source.reset(env);
}

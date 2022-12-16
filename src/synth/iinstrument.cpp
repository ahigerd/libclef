#include "iinstrument.h"
#include "channel.h"
#include "sampler.h"
#include "oscillator.h"
#include "synthcontext.h"
#include "s2wcontext.h"
#include "seq/sequenceevent.h"
#include <iostream>

void IInstrument::channelEvent(Channel* channel, std::shared_ptr<ChannelEvent> event)
{
  // TODO: more types
  auto target = channel->param(event->param);
  if (target) {
    target->setValueAt(event->timestamp, event->value);
  }
}

void IInstrument::modulatorEvent(Channel* channel, std::shared_ptr<ModulatorEvent> event)
{
  auto noteIter = channel->notes.find(event->playbackID);
  if (noteIter != channel->notes.end()) {
    auto param = noteIter->second->source->param(event->param);
    if (param) {
      param->setConstant(event->value);
    }
  }
}

Channel::Note* IInstrument::userEvent(Channel* channel, std::shared_ptr<SequenceEvent> event)
{
  std::cerr << "ERROR: unhandled user event" << std::endl;
  return nullptr;
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
    SampleData* sampleData = channel->ctx->s2wContext()->getSample(sampEvent->sampleID);
    if (!sampleData) {
      std::cerr << "ERROR: sample " << std::hex << sampEvent->sampleID << std::dec << " not found" << std::endl;
      return nullptr;
    }
    Sampler* samp = new Sampler(channel->ctx, sampleData, sampEvent->pitchBend);
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
  Channel::Note* note = new Channel::Note(event, node, duration);
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
  env->connect(note->source);
  note->source.reset(env);
}

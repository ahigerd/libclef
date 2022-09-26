#include "testsequence.h"
#include "itrack.h"
#include "utility.h"
#include "codec/riffcodec.h"
#include "synth/oscillator.h"

static const double testNotes[] = { 440, 554.36, 659.26, 880, 1108.73 };

TestSequence::TestSequence(S2WContext* ctx) : BaseSequence(ctx)
{
  RiffCodec riff(ctx);
  SampleData* sample = riff.decodeFile("cembalo-1.wav");

  for (int i = 0; i < 3; i++) {
    addTrack(new BasicTrack());
  }

  static const BaseOscillator::WaveformPreset waveforms[] = {
    BaseOscillator::Triangle,
    BaseOscillator::Square25,
    BaseOscillator::Square50,
  };
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      OscillatorEvent* event = new OscillatorEvent;
      event->timestamp = i * .25;
      event->waveformID = 4 - j * 2;
      event->duration = .2;
      event->volume = .1;
      event->frequency = testNotes[i + j] * (j ? 1 : 0.5);
      if (j == 0) {
        event->pan = 0.5;
      } else if (j == 1) {
        event->pan = 0.25;
      } else {
        event->pan = 0.75;
      }
      tracks[j]->addEvent(event);
    }
  }

  SampleEvent* samp = new SampleEvent;
  samp->sampleID = sample->sampleID;
  samp->timestamp = 1.0;
  samp->volume = 0.5;
  tracks[0]->addEvent(samp);

  samp = new SampleEvent;
  samp->sampleID = sample->sampleID;
  samp->timestamp = 1.0 + sample->duration();
  samp->pitchBend = noteToFreq(74) / 440.0;
  samp->volume = 0.5;
  samp->pan = 1;
  tracks[0]->addEvent(samp);

  samp = new SampleEvent;
  samp->sampleID = sample->sampleID;
  samp->timestamp = 1.0 + sample->duration() * 2;
  samp->pitchBend = noteToFreq(78) / 880.0;
  samp->volume = 0.5;
  samp->pan = 0;
  tracks[0]->addEvent(samp);

  OscillatorEvent* e = new OscillatorEvent;
  e->setEnvelope(.25, .5, .5, .5, .5, .5);
  e->waveformID = BaseOscillator::Sine;
  e->duration = 2.5;
  e->timestamp = 1.0 + sample->duration() * 3;
  tracks[0]->addEvent(e);

  for (int i = 0; i < BaseOscillator::NumPresets; i++) {
    OscillatorEvent* e = new OscillatorEvent;
    e->waveformID = i;
    e->frequency = (i >= BaseOscillator::Noise ? 880.0 : 440.0);
    e->duration = .5;
    e->volume = .5;
    e->timestamp = 4.5 + sample->duration() * 3 + i * .75;
    tracks[0]->addEvent(e);
  }
}

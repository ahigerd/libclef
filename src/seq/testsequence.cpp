#include "testsequence.h"
#include "itrack.h"

static const double testNotes[] = { 440, 554.36, 659.26, 880, 1108.73 };

TestSequence::TestSequence() : BaseSequence()
{
  for (int i = 0; i < 3; i++) {
    tracks.push_back(BasicTrack());
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      OscillatorEvent* event = new OscillatorEvent;
      event->timestamp = i * .25;
      event->waveformID = 4 - j * 2;
      event->duration = .2;
      event->frequency = testNotes[i + j] * (j ? 1 : 0.5);
      tracks[j].addEvent(event);
    }
  }
}

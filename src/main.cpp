#include "seq/testsequence.h"
#include "synth/channel.h"
#include "synth/synthcontext.h"
#include "s2wcontext.h"
#include "riffwriter.h"
#include <iostream>

int main(int argc, char** argv) {
  S2WContext s2w;
  TestSequence seq(&s2w);
  SynthContext ctx(&s2w, 44100);
  int numTracks = seq.numTracks();
  for (int i = 0; i < numTracks; i++) {
    ctx.channels.emplace_back(new Channel(&ctx, seq.getTrack(i)));
  }

  RiffWriter riff(ctx.sampleRate, true);
  riff.open("sample.wav");
  ctx.save(&riff);
  riff.close();
}

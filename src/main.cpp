#include "seq/testsequence.h"
#include "synth/channel.h"
#include "synth/synthcontext.h"
#include "riffwriter.h"
#include <iostream>

int main(int argc, char** argv) {
  TestSequence seq;
  SynthContext ctx(44100);
  int numTracks = seq.numTracks();
  for (int i = 0; i < numTracks; i++) {
    ctx.channels.emplace_back(new Channel(&ctx, seq.getTrack(i)));
  }

  RiffWriter riff(ctx.sampleRate, true);
  riff.open("/dev/stdout");
  ctx.save(&riff);
  riff.close();
}

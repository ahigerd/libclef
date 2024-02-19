#include "clefcontext.h"
#include "riffwriter.h"
#include "synth/synthcontext.h"

int main(int argc, char** argv)
{
  // This sample main() function does nothing but
  // generate a valid but empty wave file.
  ClefContext clef;
  SynthContext context(&clef, 44100, 2);
  RiffWriter riff(context->sampleRate, true, sampleLength * 2);
  riff.open("output.wav");
  context->save(&riff);
  return 0;
}

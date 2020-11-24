#include "seq/testsequence.h"
#include "synth/channel.h"
#include "riffwriter.h"
#include <iostream>

int main(int argc, char** argv) {
  TestSequence seq;
  std::vector<std::unique_ptr<Channel>> channels;
  int numTracks = seq.numTracks();
  for (int i = 0; i < numTracks; i++) {
    channels.emplace_back(new Channel(nullptr, seq.getTrack(i)));
  }

  RiffWriter riff(44100, true);
  riff.open("/dev/stdout");
  std::vector<int16_t> mixBuffer(1024);
  std::vector<int16_t> buffer(1024);
  bool done;
  do {
    done = true;
    for (int i = 0; i < 1024; i++) { mixBuffer[i] = 0; }
    for (auto& channel : channels) {
      uint32_t written = channel->fillBuffer(buffer);
      for (int i = 0; i < written; i++) {
        mixBuffer[i] += buffer[i];
      }
      done = done && channel->isFinished();
    }
    riff.write(mixBuffer);
  } while (!done);
  riff.close();
}

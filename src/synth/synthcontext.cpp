#include "synthcontext.h"
#include "channel.h"
#include "seq/itrack.h"
#include "iinterpolator.h"
#include "riffwriter.h"
#include "utility.h"
#include "iinstrument.h"

static const int BUFFER_SIZE = 10240;

SynthContext::SynthContext(S2WContext* ctx, double sampleRate, int outputChannels)
: sampleRate(sampleRate), sampleTime(1.0 / sampleRate), outputChannels(outputChannels),
  interpolator(IInterpolator::get(IInterpolator::Zero)), mixBuffer(BUFFER_SIZE >> 1),
  currentTimestamp(0), maximumTimestamp(0), ctx(ctx)
{
  defaultInst.reset(new DefaultInstrument);
}

SynthContext::~SynthContext()
{
}

S2WContext* SynthContext::s2wContext() const
{
  return ctx;
}

void SynthContext::addChannel(Channel* channel)
{
  double length = channel->track->length();
  if (length > maximumTimestamp) {
    maximumTimestamp = length;
  }
  channels.emplace_back(channel);
}

void SynthContext::addChannel(ITrack* track)
{
  addChannel(new Channel(this, track));
}

double SynthContext::currentTime() const
{
  return currentTimestamp;
}

double SynthContext::maximumTime() const
{
  return maximumTimestamp;
}

void SynthContext::seek(double timestamp)
{
  currentTimestamp = timestamp;
  for (auto& ch : channels) {
    ch->seek(timestamp);
  }
}

size_t SynthContext::fillBuffer(uint8_t* buffer, size_t length)
{
  size_t numSamples = length >> 1;
  if (mixBuffer.size() < numSamples) {
    mixBuffer.resize(numSamples);
  }
  uint32_t written = 0;
  for (auto& channel : channels) {
    uint32_t chWritten = channel->fillBuffer(mixBuffer, numSamples) << 1;
    for (int i = 0; i < chWritten; i += 2) {
      int16_t sample = clamp<int32_t>((i >= written ? 0 : parseInt<int16_t>(buffer, i)) + mixBuffer[i >> 1], -0x8000, 0x7FFF);
      buffer[i] = sample & 0xFF;
      buffer[i + 1] = uint16_t(sample >> 8) & 0xFF;
    }
    if (chWritten > written) {
      written = chWritten;
    }
  }
  currentTimestamp += written * sampleTime / (2 * channels.size());
  return written;
}

void SynthContext::stream(std::ostream& output)
{
  size_t written = 1;
  char buffer[BUFFER_SIZE];
  while (output.good() && written > 0) {
    written = fillBuffer(reinterpret_cast<uint8_t*>(buffer), BUFFER_SIZE);
    output.write(buffer, written);
  }
}

void SynthContext::save(RiffWriter* riff)
{
  size_t written;
  uint8_t buffer[BUFFER_SIZE];
  do {
    written = fillBuffer(buffer, BUFFER_SIZE);
    riff->write(buffer, written);
  } while (written > 0);
}

void SynthContext::registerInstrument(uint64_t id, std::unique_ptr<IInstrument>&& inst)
{
  instruments[id] = std::move(inst);
}

IInstrument* SynthContext::getInstrument(uint64_t id) const
{
  auto iter = instruments.find(id);
  if (iter == instruments.end()) {
    return nullptr;
  }
  return iter->second.get();
}

IInstrument* SynthContext::defaultInstrument() const
{
  return defaultInst.get();
}

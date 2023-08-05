#include "sampledata.h"
#include "s2wcontext.h"
#include <unordered_map>
#include <memory>

SampleData::SampleData(S2WContext* ctx, uint64_t sampleID, double sampleRate, int loopStart, int loopEnd)
: sampleID(sampleID), sampleRate(sampleRate), loopStart(loopStart), loopEnd(loopEnd)
{
  if (sampleID != Uncached && ctx) {
    ctx->sampleCache[sampleID].reset(this);
  }
  m_numSamples = -1;
  m_duration = -1;
}

SampleData::SampleData(S2WContext* ctx, double sampleRate, int loopStart, int loopEnd)
: SampleData(ctx, ctx ? ctx->nextSampleID() : 0, sampleRate, loopStart, loopEnd)
{
  // forwarded constructor only
}

uint32_t SampleData::numSamples() const
{
  if (m_numSamples < 0) {
    m_numChannels = channels.size();
    if (m_numChannels == 0) {
      m_numSamples = 0;
      return 0;
    }
    m_numSamples = channels[0].size();
    for (int i = 1; i < m_numChannels; i++) {
      if (channels[i].size() < m_numSamples) {
        m_numSamples = channels[i].size();
      }
    }
  }
  return m_numSamples;
}

double SampleData::duration() const
{
  if (m_duration < 0) {
    m_duration = numSamples() / this->sampleRate;
  }
  return m_duration;
}

int16_t SampleData::at(int index, int channel) const
{
  if (index < 0) {
    return 0;
  }
  if (loopEnd > 0 && loopEnd > loopStart && index > loopEnd) {
    index = loopStart + (index - loopStart) % (loopEnd - loopStart);
  }
  if (index >= m_numSamples) {
    if (m_numSamples >= 0 || index >= numSamples()) {
      return 0;
    }
  }
  return channels[channel % m_numChannels][index];
}

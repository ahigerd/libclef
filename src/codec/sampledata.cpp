#include "sampledata.h"
#include <unordered_map>
#include <memory>

static std::unordered_map<uint64_t, std::unique_ptr<SampleData>> sampleCache;
static uint64_t lastSampleID = 0;

SampleData* SampleData::get(uint64_t sampleID)
{
  auto iter = sampleCache.find(sampleID);
  if (iter == sampleCache.end()) {
    return nullptr;
  }
  return iter->second.get();
}

uint64_t nextSampleID()
{
  do {
    ++lastSampleID;
  } while (sampleCache.find(lastSampleID) != sampleCache.end());
  return lastSampleID;
}

SampleData::SampleData(uint64_t sampleID, double sampleRate, int loopStart, int loopEnd)
: sampleID(sampleID), sampleRate(sampleRate), loopStart(loopStart), loopEnd(loopEnd)
{
  sampleCache[sampleID].reset(this);
  m_numSamples = -1;
  m_duration = -1;
}

SampleData::SampleData(double sampleRate, int loopStart, int loopEnd)
: SampleData(nextSampleID(), sampleRate, loopStart, loopEnd)
{
  // forwarded constructor only
}

uint32_t SampleData::numSamples() const
{
  if (m_numSamples < 0) {
    m_numSamples = 0;
    for (const std::vector<int16_t>& channel : channels) {
      if (channel.size() > m_numSamples) {
        m_numSamples = channel.size();
      }
    }
  }
  return m_numSamples;
}

double SampleData::duration(double sampleRate) const
{
  if (m_duration < 0) {
    m_duration = numSamples() / this->sampleRate;
  }
  if (sampleRate < 0 || sampleRate == this->sampleRate) {
    return m_duration;
  }
  return m_duration * (this->sampleRate / sampleRate);
}

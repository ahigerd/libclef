#include "s2wcontext.h"
#include "codec/sampledata.h"

S2WContext::S2WContext(const OpenFn& openFile)
: S2WContext(false, openFile)
{
  // forwarded constructor only
}

S2WContext::S2WContext(bool isDawPlugin, const OpenFn& openFile)
: pluginData(nullptr), isDawPlugin(isDawPlugin), openFn(openFile), lastSampleID(0)
{
  // initializers only
}

std::unique_ptr<std::istream> S2WContext::openFile(const std::string& path) const
{
  return openFn(path);
}

SampleData* S2WContext::getSample(uint64_t sampleID) const
{
  auto iter = sampleCache.find(sampleID);
  if (iter == sampleCache.end()) {
    return nullptr;
  }
  return iter->second.get();
}

void S2WContext::purgeSamples()
{
  sampleCache.clear();
  lastSampleID = 0;
}

uint64_t S2WContext::nextSampleID()
{
  do {
    ++lastSampleID;
  } while (sampleCache.find(lastSampleID) != sampleCache.end());
  return lastSampleID;
}

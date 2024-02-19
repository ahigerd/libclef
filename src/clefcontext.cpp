#include "clefcontext.h"
#include "codec/sampledata.h"

ClefContext::ClefContext(const OpenFn& openFile)
: ClefContext(false, openFile)
{
  // forwarded constructor only
}

ClefContext::ClefContext(bool isDawPlugin, const OpenFn& openFile)
: pluginData(nullptr), isDawPlugin(isDawPlugin), openFn(openFile), lastSampleID(0)
{
  // initializers only
}

std::unique_ptr<std::istream> ClefContext::openFile(const std::string& path) const
{
  return openFn(path);
}

SampleData* ClefContext::getSample(uint64_t sampleID) const
{
  auto iter = sampleCache.find(sampleID);
  if (iter == sampleCache.end()) {
    return nullptr;
  }
  return iter->second.get();
}

void ClefContext::purgeSamples()
{
  sampleCache.clear();
  lastSampleID = 0;
}

uint64_t ClefContext::nextSampleID()
{
  do {
    ++lastSampleID;
  } while (sampleCache.find(lastSampleID) != sampleCache.end());
  return lastSampleID;
}

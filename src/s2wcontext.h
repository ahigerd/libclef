#ifndef S2W_S2WCONTEXT_H
#define S2W_S2WCONTEXT_H

#include <unordered_map>
#include "utility.h"
#include "codec/sampledata.h"

class S2WContext {
public:
  S2WContext(const OpenFn& openFile = openFstream);

  std::unique_ptr<std::istream> openFile(const std::string& path) const;

  SampleData* getSample(uint64_t sampleID) const;
  void purgeSamples();

  void* pluginData;

private:
  OpenFn openFn;

  friend class SampleData;
  std::unordered_map<uint64_t, std::unique_ptr<SampleData>> sampleCache;
  uint64_t lastSampleID;
  uint64_t nextSampleID();
};

#endif

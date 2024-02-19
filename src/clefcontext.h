#ifndef CLEF_CLEFCONTEXT_H
#define CLEF_CLEFCONTEXT_H

#include <unordered_map>
#include "utility.h"
#include "codec/sampledata.h"

class ClefContext {
public:
  ClefContext(const OpenFn& openFile = openFstream);
  ClefContext(bool isDawPlugin, const OpenFn& openFile = openFstream);

  std::unique_ptr<std::istream> openFile(const std::string& path) const;

  SampleData* getSample(uint64_t sampleID) const;
  void purgeSamples();

  void* pluginData;

  const bool isDawPlugin;

private:
  OpenFn openFn;

  friend struct SampleData;
  std::unordered_map<uint64_t, std::unique_ptr<SampleData>> sampleCache;
  uint64_t lastSampleID;
  uint64_t nextSampleID();
};

#endif

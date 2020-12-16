#include "plugin/baseplugin.h"
#include "codec/sampledata.h"
#include "ifs/ifssequence.h"
#include "ifs/ifs.h"
#include <array>

struct S2WPluginInfo {
  S2WPLUGIN_STATIC_FIELDS

  static bool isPlayable(std::istream& file) {
    // Will throw if invalid
    IFS ifs(file);
    return true;
  }

  static double length(const OpenFn& openFile, const std::string& filename, std::istream& file) {
    IFSSequence seq;
    seq.addIFS(new IFS(file));
    return seq.duration();
  }

  static TagMap readTags(const OpenFn& openFile, const std::string& filename, std::istream& /* unused */) {
    TagMap tagMap = TagsM3UMixin::readTags(openFile, filename);
    if (!tagMap.count("title")) {
      tagMap = TagsM3UMixin::readTags(openFile, IFS::pairedFile(filename));
    }
    return tagMap;
  }

  SynthContext* prepare(const OpenFn& openFile, const std::string& filename, std::istream& file) {
    seq.reset(new IFSSequence);
    seq->addIFS(new IFS(file));
    try {
      auto paired(openFile(IFS::pairedFile(filename)));
      if (paired) {
        seq->addIFS(new IFS(*paired));
      }
    } catch (...) {
      // no paired file, ignore
    }
    SampleData::purge();
    seq->load();
    return seq->initContext();
  }

  void release() {
    seq.reset(nullptr);
  }

  std::unique_ptr<IFSSequence> seq;
};

const std::string S2WPluginInfo::pluginName = "gitadora2wav Plugin";
const std::string S2WPluginInfo::pluginShortName = "gitadora2wav";
ConstPairList S2WPluginInfo::extensions = { { "ifs", "Konami IFS files (*.ifs)" } };
const std::string S2WPluginInfo::about =
  "gitadora2wav copyright (C) 2020 Adam Higerd\n"
  "Distributed under the MIT license.";

SEQ2WAV_PLUGIN(S2WPluginInfo);

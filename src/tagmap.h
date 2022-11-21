#ifndef S2W_TAGSM3U_H
#define S2W_TAGSM3U_H

#include "s2wconfig.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

using TagMap = std::unordered_map<std::string, std::string>;

class TagsM3U {
public:
  static std::string relativeTo(const std::string& trackPath);

  template <typename Iter>
  static TagsM3U fromData(Iter begin, Iter end) {
    TagsM3U m3u;
    for (Iter iter = begin, lineEnd = begin; iter < end; iter = lineEnd) {
      while (lineEnd < end && *lineEnd++ != '\n') {}
      if (lineEnd >= end) break;
      m3u.parseLine(std::string(iter, lineEnd));
    }
    return m3u;
  }

  TagsM3U();
  TagsM3U(std::istream& file);
  TagsM3U(const TagsM3U& other) = default;
  TagsM3U(TagsM3U&& other) = default;

  void parseLine(const std::string& line);

  void set(int trackIndex, const std::string& tag, const std::string& value);
  inline void set(const std::string& trackName, const std::string& tag, const std::string& value) { return set(findTrack(trackName), tag, value); }

  std::string get(int trackIndex, const std::string& tag) const;
  inline std::string get(const std::string& trackName, const std::string& tag) const { return get(findTrack(trackName), tag); }

  const TagMap& allTags(int trackIndex) const;
  inline const TagMap& allTags(const std::string& trackName) const { return allTags(findTrack(trackName)); }

  int numTracks() const;
  std::string trackName(int trackIndex) const;
  int findTrack(const std::string& trackName) const;
  std::vector<int> findTracksByPrefix(const std::string& prefix) const;
  void addTrack(const std::string& trackName);

private:
  void populateTag(bool globalTag, std::string tag, std::string value);
  std::vector<std::string> tracks;
  std::unordered_map<int, TagMap> tags;
  TagMap globalTags, fileTags, invalid;
  bool autoTrack;
};

#endif

#ifndef S2W_TAGSM3U_H
#define S2W_TAGSM3U_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

class TagsM3U {
public:
  using TagMap = std::unordered_map<std::string, std::string>;

  TagsM3U();
  TagsM3U(std::istream& file);

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
  void addTrack(const std::string& trackName);

private:
  void populateTag(bool globalTag, std::string tag, const std::string& value);
  std::vector<std::string> tracks;
  std::unordered_map<int, TagMap> tags;
  TagMap globalTags, fileTags, invalid;
  int autoTrack;
};

#endif

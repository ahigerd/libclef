#include "tagmap.h"
#include "utility.h"
#include <sstream>
#include <cctype>

static TagMap m3uKeys = {
  { "ALB", "ALBUM" },
  { "ART", "ARTIST" },
  { "GENRE", "GENRE" },
  { "-X-TARGETDURATION", "LENGTH_SECONDS_FP" },
};

std::string TagsM3U::relativeTo(const std::string& trackPath)
{
  int slashPos = trackPath.find_last_of(PATH_CHARS);
  if (slashPos == std::string::npos) {
    return "!tags.m3u";
  }
  return trackPath.substr(0, slashPos + 1) + "!tags.m3u";
}

TagsM3U::TagsM3U() : autoTrack(false)
{
  // initializers only
}

TagsM3U::TagsM3U(std::istream& file) : autoTrack(false)
{
  std::string line;
  while (file) {
    std::getline(file, line);
    parseLine(line);
  }
}

void TagsM3U::parseLine(const std::string& _line)
{
  std::string line = trim(_line);
  if (line.empty()) {
    // Blank lines are ignored
    return;
  }
  // Lines that start with # are directives, everything else is a filename.
  if (line[0] != '#') {
    if (!fileTags.count("title")) {
      fileTags["title"] = line;
    }
    if (autoTrack && !fileTags.count("track")) {
      std::stringstream trackSS;
      trackSS << (tracks.size() + 1);
      fileTags["track"] = trackSS.str();
    }
    tags[tracks.size()] = fileTags;
    tracks.push_back(line);
    // Copy the current global tag state into the tags for the next file.
    fileTags = globalTags;
    return;
  }
  line = trim(line.substr(1));
  std::string tag, value;
  bool globalTag = line[0] == '@';
  if (line[0] == '$') {
    // vgmstream-style directives
    if (line == "$AUTOTRACK") {
      autoTrack = true;
    }
  } else if (line[0] == '%' || line[0] == '@') {
    // vgmstream-style tags
    int end = line.find(line[0], 1);
    if (end == std::string::npos) {
      // A tag can be terminated with another token
      // If it isn't, then it ends at the first whitespace
      end = line.find(' ');
      if (end == std::string::npos) {
        // If there's no whitespace at all then it's invalid
        return;
      }
    }
    tag = trim(line.substr(1, end - 1));
    value = trim(line.substr(end + 1));
  } else if (line.substr(0, 3) == "EXT") {
    // Ext-M3U directive
    int end = line.find(':');
    if (end == std::string::npos) {
      // not a data-bearing directive
      return;
    }
    tag = trim(line.substr(3, end - 3));
    value = trim(line.substr(end + 1));
    if (tag == "INF") {
      tag = "TITLE";
      size_t lengthEnd = std::string::npos;
      {
        bool quoted = false;
        for (size_t i = 0; i < value.size(); i++) {
          if (value[i] == '"') {
            quoted = !quoted;
          } else if (!quoted && value[i] == ',') {
            lengthEnd = i;
            break;
          }
        }
      }
      if (lengthEnd != std::string::npos) {
        // No delimiter is a violation of spec, but the spec says the length must have a comma.
        // So if there's no delimiter, treat the whole thing as a title.
        std::string length = trim(value.substr(0, lengthEnd));
        value = trim(value.substr(lengthEnd + 1));
        lengthEnd = length.find(' ');
        if (lengthEnd) {
          std::string attr = length.substr(lengthEnd + 1);
          length = length.substr(0, lengthEnd);
          int pos = 0;
          int start = 0;
          std::string subtag, subvalue;
          bool quoted = false, needValue = false;
          while (pos < attr.size()) {
            if (quoted) {
              if (attr[pos] == '"') {
                // TODO: not documented if there are escape sequences
                subvalue = attr.substr(start, pos - start);
                populateTag(false, subtag, subvalue);
                quoted = false;
                needValue = false;
                start = pos + 1;
              }
            } else if (needValue) {
              if (attr[pos] == '"') {
                start = pos + 1;
                quoted = true;
              } else if (attr[pos] == ' ') {
                subvalue = attr.substr(start, pos - start);
                populateTag(false, subtag, subvalue);
                needValue = false;
                start = pos + 1;
              }
            } else if (attr[pos] == '=') {
              subtag = attr.substr(start, pos - start);
              needValue = true;
              start = pos + 1;
            }
            ++pos;
          }
          if (needValue) {
            // fell off the end, capture the last value
            subvalue = attr.substr(start, pos - start);
            populateTag(false, subtag, subvalue);
          }
        }
        if (!length.empty() && length[0] != '-') {
          populateTag(false, "LENGTH_SECONDS_FP", length);
        }
      }
      ssize_t dashPos = value.find(" - ");
      if (dashPos != std::string::npos) {
        std::string artist = value.substr(0, dashPos);
        value = value.substr(dashPos + 3);
        populateTag(false, "ARTIST", artist);
      }
    } else {
      auto iter = m3uKeys.find(tag);
      if (iter != m3uKeys.end()) {
        tag = iter->second;
        // album, artist, and genre stick until overridden
        // everything else is an extension
        globalTag = tag[0] != '-';
      }
    }
  } else {
    // Not a recognized directive, probably a comment -- ignore it
    return;
  }
  populateTag(globalTag, tag, value);
}

void TagsM3U::populateTag(bool globalTag, std::string tag, std::string value)
{
  tag = trim(tag);
  value = trim(value);
  if (tag.empty()) {
    return;
  }
  for (char& ch : tag) {
    ch = std::tolower(ch);
  }
  fileTags[tag] = value;
  if (globalTag) {
    globalTags[tag] = value;
  }
}

void TagsM3U::set(int trackIndex, const std::string& tag, const std::string& value)
{
  if (trackIndex < 0 || trackIndex >= tracks.size()) {
    std::cerr << "Invalid track index" << std::endl;
    return;
  }
  tags[trackIndex][tag] = value;
}

std::string TagsM3U::get(int trackIndex, const std::string& tag) const
{
  if (trackIndex < 0 || trackIndex >= tracks.size()) {
    return std::string();
  }
  auto group = tags.at(trackIndex);
  auto value = group.find(tag);
  if (value == group.end()) {
    return std::string();
  }
  return value->second;
}

std::string TagsM3U::trackName(int trackIndex) const
{
  if (trackIndex < 0 || trackIndex >= tracks.size()) {
    return std::string();
  }
  return tracks.at(trackIndex);
}

int TagsM3U::numTracks() const
{
  return tracks.size();
}

void TagsM3U::addTrack(const std::string& trackName)
{
  TagMap newMap;
  newMap["title"] = trackName;
  tags[tracks.size()] = newMap;
  tracks.push_back(trackName);
}

int TagsM3U::findTrack(const std::string& trackName) const
{
  int slashPos = trackName.find_last_of(PATH_CHARS);
  std::string filename = trackName.substr(slashPos + 1);
  for (int i = 0; i < tracks.size(); i++) {
    if (tracks.at(i) == filename) {
      return i;
    }
  }
  return -1;
}

const TagMap& TagsM3U::allTags(int trackIndex) const
{
  if (trackIndex < 0 || trackIndex > tracks.size()) {
    return invalid;
  }
  return tags.at(trackIndex);
}

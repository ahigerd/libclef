#include "commandargs.h"
#include <sstream>
#include <iomanip>

CommandArgs::CommandArgs(std::initializer_list<FlagDef> _defs) {
  for (const FlagDef& def : _defs) {
    if (def.longFlag.empty() && def.shortFlag.empty()) {
      positionalDefs.push_back(def);
    } else {
      defs["--" + def.longFlag] = def;
      if (!def.shortFlag.empty()) {
        shortFlags["-" + def.shortFlag] = def.longFlag;
      }
    }
  }
}

std::string CommandArgs::parse(int argc, char** argv) {
  bool expectArg = false;
  std::string argKey;
  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);
    if (expectArg) {
      expectArg = false;
      flags[argKey] = arg;
      continue;
    }
    auto longFlag = shortFlags.find(arg);
    if (longFlag != shortFlags.end()) {
      arg = longFlag->second;
    }
    auto defIter = defs.find(arg);
    if (defIter == defs.end()) {
      _positional.push_back(arg);
    } else if (!defIter->second.argName.empty()) {
      expectArg = true;
      argKey = defIter->second.longFlag;
    } else {
      flags[defIter->second.longFlag] = std::string();
    }
  }
  if (expectArg) {
    return std::string(argv[0]) + ": " + argKey + " requires a parameter (" + defs[argKey].argName + ")";
  }
  return std::string();
}

const std::vector<std::string>& CommandArgs::positional() const {
  return _positional;
}

bool CommandArgs::hasKey(const std::string& longFlag) const {
  return flags.count(longFlag) > 0;
}

std::string CommandArgs::getString(const std::string& longFlag, std::string defaultValue) const {
  auto iter = flags.find(longFlag);
  if (iter == flags.end()) {
    return defaultValue;
  }
  return iter->second;
}

int CommandArgs::getInt(const std::string& longFlag, int defaultValue) const {
  auto iter = flags.find(longFlag);
  if (iter == flags.end()) {
    return defaultValue;
  }
  return std::stoi(iter->second, 0, 0);
}

double CommandArgs::getFloat(const std::string& longFlag, double defaultValue) const {
  auto iter = flags.find(longFlag);
  if (iter == flags.end()) {
    return defaultValue;
  }
  return std::stod(iter->second);
}

std::string CommandArgs::usageText(const std::string& programName) const {
  int columnWidth = 6;
  for (auto iter : defs) {
    int flagLength = 2 + iter.second.longFlag.length();
    if (flagLength > columnWidth) {
      columnWidth = flagLength;
    }
  }
  for (auto def : positionalDefs) {
    if (def.argName.length() > columnWidth) {
      columnWidth = def.argName.length();
    }
  }

  std::stringstream result;
  result << "Usage: " << programName;
  for (auto iter : defs) {
    result << " [" << iter.first;
    if (!iter.second.shortFlag.empty()) {
      result << "|-" << iter.second.shortFlag;
    }
    if (!iter.second.argName.empty()) {
      result << " <" << iter.second.argName << ">";
    }
    result << "]";
  }
  for (auto def : positionalDefs) {
    result << " <" << def.argName << ">";
  }

  for (auto iter : defs) {
    result << std::endl << std::setw(columnWidth) << iter.first;
    if (!iter.second.shortFlag.empty()) {
      result << " (-" << iter.second.shortFlag << ") ";
    } else {
      result << "      ";
    }
    result << iter.second.description;
  }

  for (auto def : positionalDefs) {
    result << std::endl << std::setw(columnWidth) << def.argName << "      " << def.description;
  }

  return result.str();
}


#ifndef MONPITOR_COMMANDARGS_H
#define MONPITOR_COMMANDARGS_H

#include <string>
#include <map>
#include <vector>

class CommandArgs {
public:
  struct FlagDef {
    std::string longFlag;
    std::string shortFlag;
    std::string argName;
    std::string description;
  };

  CommandArgs(std::initializer_list<FlagDef> defs);

  // returns error message, or empty for OK
  std::string parse(int argc, char** argv);

  const std::vector<std::string>& positional() const;
  bool hasKey(const std::string& longFlag) const;
  std::string getString(const std::string& longFlag, std::string defaultValue = std::string()) const;
  int getInt(const std::string& longFlag, int defaultValue = 0) const;
  double getFloat(const std::string& longFlag, double defaultValue = 0) const;

  std::string usageText(const std::string& programName) const;

private:
  std::map<std::string, FlagDef> defs;
  std::map<std::string, std::string> shortFlags;
  std::vector<FlagDef> positionalDefs;
  std::vector<std::string> _positional;
  std::map<std::string, std::string> flags;
};

#endif

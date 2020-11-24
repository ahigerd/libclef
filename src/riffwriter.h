#ifndef S2W_RIFFWRITER_H
#define S2W_RIFFWRITER_H

#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

class RiffWriter
{
public:
  RiffWriter(uint32_t sampleRate, bool stereo, uint32_t sizeInBytes = 0);
  ~RiffWriter();

  bool open(const std::string& filename);
  void write(const std::vector<char>& data);
  void write(const std::vector<int16_t>& data);
  void close();

private:
  std::ofstream file;
  uint32_t sampleRate, size;
  bool stereo, rewriteSize;
};

#endif

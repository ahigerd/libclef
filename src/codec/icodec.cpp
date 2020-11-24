#include "icodec.h"
#include <fstream>

ICodec::~ICodec() {}

SampleData* ICodec::decodeFile(const std::string& filename, uint64_t sampleID)
{
  std::vector<uint8_t> buffer;
  std::vector<char> block(1024);
  std::ifstream file(filename.c_str());
  while (file.good()) {
    file.read(block.data(), 1024);
    int bytesRead = file.gcount();
    buffer.insert(buffer.end(), block.begin(), block.begin() + bytesRead);
  }
  return decode(buffer, sampleID);
}

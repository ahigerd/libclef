#include "icodec.h"
#include "clefcontext.h"
#include <fstream>

ICodec::ICodec(ClefContext* ctx)
: ctx(ctx)
{
  // initializers only
}

ICodec::~ICodec() {}

ClefContext* ICodec::context() const
{
  return ctx;
}

SampleData* ICodec::decodeFile(const std::string& filename, uint64_t sampleID)
{
  std::vector<uint8_t> buffer;
  std::vector<char> block(1024);
  auto file(ctx->openFile(filename));
  while (file->good()) {
    file->read(block.data(), 1024);
    int bytesRead = file->gcount();
    buffer.insert(buffer.end(), block.begin(), block.begin() + bytesRead);
  }
  return decode(buffer, sampleID);
}

SampleData* ICodec::decode(const std::vector<uint8_t>& buffer, uint64_t sampleID)
{
  return decodeRange(buffer.begin(), buffer.end(), sampleID);
}

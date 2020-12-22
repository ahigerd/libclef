#include "riffcodec.h"
#include "pcmcodec.h"
#include "msadpcmcodec.h"
#include "utility.h"
#include <memory>
#include <exception>

SampleData* RiffCodec::decodeRange(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end, uint64_t sampleID)
{
  const uint8_t* buffer = &start[0];
  uint32_t magic = parseIntBE<uint32_t>(buffer, 0);
  if (magic != 'RIFF') {
    throw std::runtime_error("not a RIFF file");
  }
  uint32_t size = parseInt<uint32_t>(buffer, 4) + 8;
  if (size > end - start) {
    throw std::runtime_error("RIFF data is truncated");
  }
  magic = parseIntBE<uint32_t>(buffer, 8);
  if (magic != 'WAVE') {
    throw std::runtime_error("not a RIFF WAVE file");
  }
  uint32_t offset = 12;
  std::vector<uint8_t> data;
  int sampleBits = 0, channels = 0, format = 0, sampleRate, blockAlign;
  while (offset < size) {
    if (offset + 8 > size) {
      throw std::runtime_error("RIFF data is invalid");
    }
    magic = parseIntBE<uint32_t>(buffer, offset);
    uint32_t chunkSize = parseInt<uint32_t>(buffer, offset + 4);
    if (offset + 8 + chunkSize > size) {
      throw std::runtime_error("RIFF chunk data is invalid");
    }
    if (magic == 'fmt ') {
      if (chunkSize < 16) {
        throw std::runtime_error("RIFF fmt data is invalid");
      }
      format = parseInt<uint16_t>(buffer, offset + 8);
      channels = parseInt<uint16_t>(buffer, offset + 10);
      sampleRate = parseInt<uint32_t>(buffer, offset + 12);
      blockAlign = parseInt<uint16_t>(buffer, offset + 20);
      sampleBits = parseInt<uint16_t>(buffer, offset + 22);
    } else if (magic == 'data') {
      data.insert(
        data.end(),
        buffer + (offset + 8),
        buffer + (offset + chunkSize)
      );
    }
    offset += 8 + chunkSize;
  }
  std::unique_ptr<ICodec> codec;
  if (format == 1) {
    codec.reset(new PcmCodec(sampleBits, channels, false));
  } else if (format == 2) {
    codec.reset(new MsAdpcmCodec(blockAlign, channels));
  } else {
    throw std::runtime_error("Unsupported RIFF format");
  }
  SampleData* sampleData = codec->decode(data, sampleID);
  sampleData->sampleRate = sampleRate;
  return sampleData;
}

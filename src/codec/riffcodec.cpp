#include "riffcodec.h"
#include "pcmcodec.h"
#include "msadpcmcodec.h"
#include "utility.h"
#include <memory>
#include <stdexcept>

WaveFormatEx::WaveFormatEx() : format(0xFFFF)
{
  // initializers only
}

WaveFormatEx::WaveFormatEx(std::vector<uint8_t>::const_iterator start, std::vector<uint8_t>::const_iterator end)
{
  if (end - start < 12) {
    throw std::runtime_error("WAVEFORMATEX is invalid");
  }
  format = parseInt<uint16_t>(start, 0);
  channels = parseInt<uint16_t>(start, 2);
  sampleRate = parseInt<uint32_t>(start, 4);
  byteRate = parseInt<uint32_t>(start, 8);
  blockAlign = parseInt<uint16_t>(start, 12);
  sampleBits = parseInt<uint16_t>(start, 14);
  if (format != 1 && start + 18 < end) {
    int exLen = parseInt<uint16_t>(start, 16);
    if (exLen > 0 && start + 18 + exLen <= end) {
      exData = std::vector<uint8_t>(start + 18, start + 18 + exLen);
    }
  }
}

RiffCodec::RiffCodec(ClefContext* ctx) : ICodec(ctx)
{
  // initializers only
}

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
  WaveFormatEx fmt;
  while (offset < size) {
    if (offset + 8 > size) {
      throw std::runtime_error("RIFF data is invalid");
    }
    magic = parseIntBE<uint32_t>(buffer, offset);
    uint32_t chunkSize = parseInt<uint32_t>(buffer, offset + 4);
    if (offset + 8 + chunkSize > size) {
      throw std::runtime_error("RIFF chunk data is invalid");
    }
    offset += 8;
    if (magic == 'fmt ') {
      fmt = WaveFormatEx(start + offset, start + offset + chunkSize);
    } else if (magic == 'data') {
      data.insert(
        data.end(),
        buffer + offset,
        buffer + (offset + chunkSize)
      );
    }
    offset += chunkSize;
  }
  std::unique_ptr<ICodec> codec;
  if (fmt.format == 1) {
    codec.reset(new PcmCodec(context(), fmt.sampleBits, fmt.channels, false));
  } else if (fmt.format == 2) {
    codec.reset(new MsAdpcmCodec(context(), fmt.blockAlign, fmt.channels));
  } else {
    throw std::runtime_error("Unsupported RIFF format");
  }
  SampleData* sampleData = codec->decode(data, sampleID);
  sampleData->sampleRate = fmt.sampleRate;
  return sampleData;
}

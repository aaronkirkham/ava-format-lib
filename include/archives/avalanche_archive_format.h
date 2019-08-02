#pragma once

#include <cstdint>
#include <vector>

namespace ava::AvalancheArchiveFormat
{
static constexpr uint32_t AAF_MAGIC               = 0x464141;   // "AAF"
static constexpr uint32_t AAF_CHUNK_MAGIC         = 0x4D415745; // "EWAM"
static constexpr uint32_t AAF_MAX_CHUNK_DATA_SIZE = 0x2000000;

#pragma pack(push, 1)
struct AafFileHeader {
    uint32_t m_Magic                  = AAF_MAGIC;
    uint32_t m_Version                = 1;
    char     m_Magic2[28]             = {};
    uint32_t m_TotalUncompressedSize  = 0;
    uint32_t m_UncompressedBufferSize = 0;
    uint32_t m_ChunkCount             = 0;
};

struct AafFileChunk {
    uint32_t m_CompressedSize   = 0;
    uint32_t m_UncompressedSize = 0;
    uint32_t m_DataSize         = 0;
    uint32_t m_Magic            = AAF_CHUNK_MAGIC;
};
#pragma pack(pop)

static_assert(sizeof(AafFileHeader) == 0x30, "AafFileHeader alignment is wrong!");
static_assert(sizeof(AafFileChunk) == 0x10, "AafFileChunk alignment is wrong!");

void Parse(const std::vector<uint8_t>& buffer, std::vector<uint8_t>* out_buffer);
}; // namespace ava::AvalancheArchiveFormat

#pragma once

#include <cstdint>
#include <vector>

namespace ava::AvalancheArchiveFormat
{
static constexpr uint32_t AAF_MAGIC               = 0x464141;   // "AAF"
static constexpr uint32_t AAF_CHUNK_MAGIC         = 0x4D415745; // "EWAM"
static constexpr uint32_t AAF_MAX_CHUNK_DATA_SIZE = 0x2000000;
static constexpr uint32_t AAF_PADDING_BYTE        = 0x30;
static constexpr uint32_t ZLIB_HEADER_SIZE        = sizeof(uint8_t) + sizeof(uint8_t); // CMF + FLG
static constexpr uint32_t ZLIB_CHECKSUM_SIZE      = sizeof(uint32_t);                  // ADLER32

#pragma pack(push, 1)
struct AafHeader {
    uint32_t m_Magic                    = AAF_MAGIC;
    uint32_t m_Version                  = 1;
    char     m_Magic2[28]               = {0};
    uint32_t m_TotalUnpackedSize        = 0;
    uint32_t m_RequiredUnpackBufferSize = 0;
    uint32_t m_NumChunks                = 0;

    AafHeader()
    {
        strncpy_s(m_Magic2, 29, "AVALANCHEARCHIVEFORMATISCOOL", 29);
    }
};

struct AafChunk {
    uint32_t m_CompressedSize   = 0;
    uint32_t m_DecompressedSize = 0;
    uint32_t m_ChunkSize        = 0;
    uint32_t m_Magic            = AAF_CHUNK_MAGIC;
};
#pragma pack(pop)

static_assert(sizeof(AafHeader) == 0x30, "AafHeader alignment is wrong!");
static_assert(sizeof(AafChunk) == 0x10, "AafChunk alignment is wrong!");

void Compress(const std::vector<uint8_t>& buffer, std::vector<uint8_t>* out_buffer);
void Decompress(const std::vector<uint8_t>& buffer, std::vector<uint8_t>* out_buffer);
}; // namespace ava::AvalancheArchiveFormat

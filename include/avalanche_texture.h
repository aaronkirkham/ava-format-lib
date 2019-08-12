#pragma once

#include <cstdint>
#include <vector>

namespace ava::AvalancheTexture
{
static constexpr uint32_t AVTX_MAGIC       = 0x58545641; // AVTX
static constexpr uint32_t AVTX_MAX_STREAMS = 8;

enum class TextureFlags : uint32_t {
    STREAMED    = (1 << 0),
    PLACEHOLDER = (1 << 1),
    TILED       = (1 << 2),
    SRGB        = (1 << 3),
    CUBE        = (1 << 6),
};

#pragma pack(push, 1)
struct AvtxStream {
    uint32_t m_Offset;
    uint32_t m_Size;
    uint16_t m_Alignment;
    bool     m_IsTile;
    bool     m_IsSource;
};

struct AvtxHeader {
    uint32_t   m_Magic     = AVTX_MAGIC;
    uint32_t   m_Version   = 1;
    char       _unknown[2] = {};
    uint8_t    m_Dimension;
    uint32_t   m_Format;
    uint16_t   m_Width;
    uint16_t   m_Height;
    uint16_t   m_Depth;
    uint16_t   m_Flags;
    uint8_t    m_Mips;
    uint8_t    m_MipsRedisent;
    char       pad[10]                     = {};
    AvtxStream m_Streams[AVTX_MAX_STREAMS] = {};
};
#pragma pack(pop)

static_assert(sizeof(AvtxStream) == 0xC, "AvtxStream alignment is wrong!");
static_assert(sizeof(AvtxHeader) == 0x83, "AvtxHeader alignment is wrong!");

void Parse(const std::vector<uint8_t>& buffer, std::vector<uint8_t>* out_buffer);

uint8_t  FindBestStream(const AvtxHeader& header, bool only_source = false);
uint32_t GetHighestRank(const AvtxHeader& header, uint8_t stream_index);
}; // namespace ava::AvalancheTexture

#pragma once

#include <cstdint>
#include <vector>

namespace ava::AvalancheTexture
{
static constexpr uint32_t AVTX_MAGIC       = 0x58545641; // AVTX
static constexpr uint32_t AVTX_MAX_STREAMS = 8;

enum ETextureFlag : uint32_t {
    E_AVATEXTURE_FLAG_STREAMED        = 0x1,
    E_AVATEXTURE_FLAG_PLACEMENT       = 0x2,
    E_AVATEXTURE_FLAG_TILED           = 0x4,
    E_AVATEXTURE_FLAG_SRGB            = 0x8,
    E_AVATEXTURE_FLAG_LOD_FROM_RENDER = 0x10,
    E_AVATEXTURE_FLAG_CUBE            = 0x40,
    E_AVATEXTURE_FLAG_WATCH           = 0x8000,
};

#pragma pack(push, 1)
struct AvtxStream {
    uint32_t m_Offset;
    uint32_t m_Size;
    uint16_t m_Alignment;
    bool     m_TileMode;
    bool     m_Source;
};

struct AvtxHeader {
    uint32_t   m_Magic   = AVTX_MAGIC;
    uint8_t    m_Version = 1;
    uint8_t    m_Platform;
    uint8_t    m_Tag;
    uint8_t    m_Dimension;
    uint32_t   m_Format;
    uint16_t   m_Width;
    uint16_t   m_Height;
    uint16_t   m_Depth;
    uint16_t   m_Flags;
    uint8_t    m_Mips;
    uint8_t    m_MipsRedisent;
    uint8_t    m_MipsCinematic;
    uint8_t    m_MipsBias;
    uint8_t    m_LodGroup;
    uint8_t    m_Pool;
    uint8_t    m_Reserved0[2];
    uint32_t   m_Reserved1;
    AvtxStream m_Streams[AVTX_MAX_STREAMS] = {};
};
#pragma pack(pop)

static_assert(sizeof(AvtxStream) == 0xC, "AvtxStream alignment is wrong!");
static_assert(sizeof(AvtxHeader) == 0x80, "AvtxHeader alignment is wrong!");

void Parse(const std::vector<uint8_t>& buffer, std::vector<uint8_t>* out_buffer);

uint8_t  FindBestStream(const AvtxHeader& header, bool only_source = false);
uint32_t GetHighestRank(const AvtxHeader& header, uint8_t stream_index);
}; // namespace ava::AvalancheTexture

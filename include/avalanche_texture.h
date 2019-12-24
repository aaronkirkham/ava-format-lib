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
    uint16_t m_Alignment = 16;
    bool     m_TileMode;
    uint8_t  m_Source;
};

struct AvtxHeader {
    uint32_t   m_Magic   = AVTX_MAGIC;
    uint8_t    m_Version = 1;
    uint8_t    m_Platform;
    uint8_t    m_Tag;
    uint8_t    m_Dimension = 2;
    uint32_t   m_Format;
    uint16_t   m_Width;
    uint16_t   m_Height;
    uint16_t   m_Depth = 1;
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

struct TextureEntry {
    uint16_t m_Width;
    uint16_t m_Height;
    uint16_t m_Depth;
    uint32_t m_Format;
    uint8_t  m_Source;
};

static_assert(sizeof(AvtxStream) == 0xC, "AvtxStream alignment is wrong!");
static_assert(sizeof(AvtxHeader) == 0x80, "AvtxHeader alignment is wrong!");

/**
 * Read the best ranked entry from the AVTX buffer
 *
 * @param buffer Input buffer containing a raw AVTX file buffer
 * @param out_entry Pointer to TextureEntry struct where the texture information will be written
 * @param out_buffer Pointer to a byte vector where the texture pixel data will be written
 * @param source_buffer (Optional) Input source buffer containing raw texture data
 */
void ReadBestEntry(const std::vector<uint8_t>& buffer, TextureEntry* out_entry, std::vector<uint8_t>* out_buffer,
                   const std::vector<uint8_t>& source_buffer = {});

/**
 * Read a specific entry from the AVTX buffer
 *
 * @param buffer Input buffer containing a raw AVTX file buffer
 * @param stream_index Stream index to read from the AVTX buffer (some streams may require source_buffer)
 * @param out_entry Pointer to TextureEntry struct where the texture information will be written
 * @param out_buffer Pointer to a byte vector where the texture pixel data will be written
 * @param source_buffer (Optional) Input source buffer containing raw texture data (only required if the
 * AvtxStream.m_Source is set)
 */
void ReadEntry(const std::vector<uint8_t>& buffer, const uint8_t stream_index, TextureEntry* out_entry,
               std::vector<uint8_t>* out_buffer, const std::vector<uint8_t>& source_buffer = {});

/**
 * Write a texture entry to the AVTX buffer
 *
 * @param buffer Pointer to a raw AVTX file buffer where the entry will be written
 * @param entry TextureEntry struct containing information to be written]
 * @param texture_buffer Raw texture buffer to write to the AVTX file buffer
 * @param source_buffer (Optional) Write to source buffer instead of AVTX file buffer (only required if entry.m_Source
 * is set)
 */
void WriteEntry(std::vector<uint8_t>* buffer, const TextureEntry& entry, const std::vector<uint8_t>& texture_buffer,
                std::vector<uint8_t>* source_buffer = nullptr);

uint8_t  FindBestStream(const AvtxHeader& header, uint8_t source = 0);
uint32_t GetRank(const AvtxHeader& header, uint8_t stream_index);
}; // namespace ava::AvalancheTexture

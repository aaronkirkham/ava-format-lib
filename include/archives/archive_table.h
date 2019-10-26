#pragma once

#include <cstdint>
#include <vector>

namespace ava::ArchiveTable
{
static constexpr uint32_t TAB_MAGIC = 0x424154; // "TAB"

enum class CompressionType : uint8_t {
    CompressionType_None  = 0,
    CompressionType_Zlib  = 1,
    CompressionType_Oodle = 4,
};

#pragma pack(push, 1)
struct TabHeader {
    uint32_t m_Magic                  = TAB_MAGIC;
    uint16_t m_Version                = 2;
    uint16_t m_Endian                 = 1;
    int32_t  m_Alignment              = 0x1000;
    uint32_t _unknown                 = 0;
    uint32_t m_MaxCompressedBlockSize = 0;
    uint32_t m_UncompressedBlockSize  = 0;
};

struct TabEntry {
    uint32_t        m_NameHash;
    uint32_t        m_Offset;
    uint32_t        m_Size;
    uint32_t        m_UncompressedSize;
    uint16_t        m_CompressedBlockIndex;
    CompressionType m_CompressionType;
    uint8_t         m_Flags;
};

struct TabCompressedBlock {
    uint32_t m_CompressedSize;
    uint32_t m_UncompressedSize;
};
#pragma pack(pop)

static_assert(sizeof(TabHeader) == 0x18, "TabHeader alignment is wrong!");
static_assert(sizeof(TabEntry) == 0x14, "TabEntry alignment is wrong!");
static_assert(sizeof(TabCompressedBlock) == 0x8, "TabCompressedBlock alignment is wrong!");

void ReadTab(const std::vector<uint8_t>& buffer, std::vector<TabEntry>* out_entries,
             std::vector<TabCompressedBlock>* out_compression_blocks = nullptr);
bool ReadTabEntry(const std::vector<uint8_t>& buffer, uint32_t name_hash, TabEntry* out_entry);
void ReadEntryBufferFromArchive(const std::vector<uint8_t>& archive_buffer, const TabEntry& entry,
                                const std::vector<TabCompressedBlock>* compression_blocks,
                                std::vector<uint8_t>*                  out_buffer);
}; // namespace ava::ArchiveTable

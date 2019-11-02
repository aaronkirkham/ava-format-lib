#pragma once

#include <cstdint>
#include <vector>

namespace ava::ArchiveTable
{
static constexpr uint32_t TAB_MAGIC = 0x424154; // "TAB"

enum ECompressLibrary : uint8_t {
    E_COMPRESS_LIBRARY_NONE  = 0x0,
    E_COMPRESS_LIBRARY_ZLIB  = 0x1,
    E_COMPRESS_LIBRARY_OODLE = 0x4,
};

enum EEntryFlags {
    E_ENTRY_FLAG_DECODE_NONE   = 0x0,
    E_ENTRY_FLAG_DECODE_BUFFER = 0x1,
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
    uint32_t         m_NameHash;
    uint32_t         m_Offset;
    uint32_t         m_Size;
    uint32_t         m_UncompressedSize;
    uint16_t         m_CompressedBlockIndex;
    ECompressLibrary m_Library;
    uint8_t          m_Flags;
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

void WriteEntry(const std::string& filename, const std::vector<uint8_t>& file_buffer,
                std::vector<uint8_t>* out_tab_buffer, std::vector<uint8_t>* out_arc_buffer,
                ECompressLibrary compression = E_COMPRESS_LIBRARY_NONE);
}; // namespace ava::ArchiveTable

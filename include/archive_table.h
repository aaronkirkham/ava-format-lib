#pragma once

#include <cstdint>
#include <filesystem>

namespace ava::ArchiveTable
{
static constexpr uint32_t TAB_MAGIC = 0x424154; // "TAB"

#pragma pack(push, 1)
struct TabFileHeader {
    uint32_t m_Magic     = TAB_MAGIC;
    uint16_t m_Version   = 2;
    uint16_t m_Endian    = 1;
    int32_t  m_Alignment = 0x1000;
#ifndef USE_LEGACY_ARCHIVE_TABLE
    uint32_t _unknown                 = 0;
    uint32_t m_MaxCompressedBlockSize = 0;
    uint32_t m_UncompressedBlockSize  = 0;
#endif
};

#ifndef USE_LEGACY_ARCHIVE_TABLE
enum class CompressionType : uint8_t {
    CompressionType_None  = 0,
    CompressionType_Zlib  = 1,
    CompressionType_Oodle = 4,
};
#endif

struct TabFileEntry {
    uint32_t m_NameHash;
    uint32_t m_Offset;
    uint32_t m_Size;
#ifndef USE_LEGACY_ARCHIVE_TABLE
    uint32_t        m_UncompressedSize;
    uint16_t        m_CompressedBlockIndex;
    CompressionType m_CompressionType;
    uint8_t         m_Flags;
#endif
};

#ifndef USE_LEGACY_ARCHIVE_TABLE
struct TabFileCompressedBlock {
    uint32_t m_CompressedSize;
    uint32_t m_UncompressedSize;
};
#endif
#pragma pack(pop)

#ifdef USE_LEGACY_ARCHIVE_TABLE
static_assert(sizeof(TabFileHeader) == 0xC, "TabFileHeader (legacy) alignment is wrong!");
static_assert(sizeof(TabFileEntry) == 0xC, "TabFileEntry (legacy) alignment is wrong!");
#else
static_assert(sizeof(TabFileHeader) == 0x18, "TabFileHeader alignment is wrong!");
static_assert(sizeof(TabFileEntry) == 0x14, "TabFileEntry alignment is wrong!");
static_assert(sizeof(TabFileCompressedBlock) == 0x8, "TabFileCompressedBlock alignment is wrong!");
#endif

void ReadTab(const std::filesystem::path& filename, std::vector<TabFileEntry>* out_entries
#ifndef USE_LEGACY_ARCHIVE_TABLE
             ,
             std::vector<TabFileCompressedBlock>* out_compressed_blocks
#endif
);

bool ReadTabEntry(const std::filesystem::path& filename, uint32_t name_hash, TabFileEntry* out_entry);
}; // namespace ava::ArchiveTable

#pragma once

#include "../error.h"

#include <cstdint>
#include <string>
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

/**
 * Parse a TAB file and extract file entries
 *
 * @param buffer Input buffer containing a raw TAB file buffer
 * @param out_entries Pointer to vector of TabEntry's where the entries will be written
 * @param out_compression_blocks (Optional) Pointer to vector of TabCompressedBlock where the compression entries will
 * be written
 */
Result Parse(const std::vector<uint8_t>& buffer, std::vector<TabEntry>* out_entries,
             std::vector<TabCompressedBlock>* out_compression_blocks = nullptr);

/**
 * Read a single entry from a TAB file buffer
 *
 * @param buffer Input buffer containing a raw TAB file buffer
 * @param name_hash Filename hash of the entry to read
 * @param out_entry Pointer to a TabEntry struct where the entry will be written
 */
Result ReadEntry(const std::vector<uint8_t>& buffer, const uint32_t name_hash, TabEntry* out_entry);

/**
 * Read an entry file buffer from an ARC file buffer
 *
 * @param archive_buffer Input buffer containing a raw ARC file buffer
 * @param entry Entry to read from the ARC file buffer
 * @param out_buffer Pointer to a byte vector where the entry file buffer will be written
 * @param compression_blocks (Optional) Vector of TabCompressedBlocks (required if entry.m_CompressedBlockIndex != 0)
 */
Result ReadEntryBuffer(const std::vector<uint8_t>& archive_buffer, const TabEntry& entry,
                       std::vector<uint8_t>*                  out_buffer,
                       const std::vector<TabCompressedBlock>& compression_blocks = {});

/**
 * Decompress an entries buffer, similiar to ReadEntryBuffer, but requires the input buffer to only be the entry buffer
 * and not the archive buffer. This means we don't have to keep a whole copy of an archive file in memory.
 *
 * @param buffer Input buffer containing a raw entry file buffer
 * @param entry Entry whose buffer this belongs to
 * @param out_buffer Pointer to a byte vector where the decompressed entry file buffer will be written
 * @param compression_blocks (Optional) Vector of TabCompressedBlocks (required if entry.m_CompressedBlockIndex != 0)
 */
Result DecompressEntryBuffer(const std::vector<uint8_t>& buffer, const TabEntry& entry,
                             std::vector<uint8_t>*                  out_buffer,
                             const std::vector<TabCompressedBlock>& compression_blocks = {});

/**
 * Return total size required for compressed buffer size
 *
 * @param entry Entry to get the total buffer size of
 * @param compression_blocks (Optional) Vector of TabCompressedBlocks (required if entry.m_CompressedBlockIndex != 0)
 */
uint32_t GetEntryRequiredBufferSize(const TabEntry&                        entry,
                                    const std::vector<TabCompressedBlock>& compression_blocks = {});

/**
 * Write a single entry to a TAB & ARC file buffer
 *
 * @param out_tab_buffer Pointer to a raw TAB file buffer where the entry will be written
 * @param out_arc_buffer Pointer to a raw ARC file buffer where the entry file buffer will be written
 * @param filename String containing the name of the entry to write
 * @param file_buffer Entry file buffer to write to the ARC file buffer
 * @param compression (Optional) Compression method to use to compress the entry file buffer
 */
Result WriteEntry(std::vector<uint8_t>* out_tab_buffer, std::vector<uint8_t>* out_arc_buffer,
                  const std::string& filename, const std::vector<uint8_t>& file_buffer,
                  ECompressLibrary compression = E_COMPRESS_LIBRARY_NONE);
}; // namespace ava::ArchiveTable

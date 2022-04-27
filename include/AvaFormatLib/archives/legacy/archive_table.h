#pragma once

#include "../../error.h"

#include <cstdint>
#include <vector>

namespace ava::legacy::ArchiveTable
{
static constexpr uint32_t TAB_MAGIC = 0x424154; // "TAB"

#pragma pack(push, 1)
struct TabHeader {
    uint32_t m_Magic     = TAB_MAGIC;
    uint16_t m_Version   = 2;
    uint16_t m_Endian    = 1;
    int32_t  m_Alignment = 0x1000;
};

struct TabEntry {
    uint32_t m_NameHash;
    uint32_t m_Offset;
    uint32_t m_Size;
};
#pragma pack(pop)

static_assert(sizeof(TabHeader) == 0xC, "TabHeader (legacy) alignment is wrong!");
static_assert(sizeof(TabEntry) == 0xC, "TabEntry (legacy) alignment is wrong!");

/**
 * Parse a legacy TAB file and extract file entries
 *
 * @param buffer Input buffer containing a raw legacy TAB file buffer
 * @param out_entries Pointer to vector of legacy TabEntry's where the entries will be written
 */
Result Parse(const std::vector<uint8_t>& buffer, std::vector<TabEntry>* out_entries);

/**
 * Read a single legacy entry from a TAB file buffer
 *
 * @param buffer Input buffer containing a raw legacy TAB file buffer
 * @param name_hash Filename hash of the entry to read
 * @param out_entry Pointer to a legacy TabEntry struct where the entry will be written
 */
Result ReadEntry(const std::vector<uint8_t>& buffer, uint32_t name_hash, TabEntry* out_entry);

/**
 * Read a legacy entry file buffer from an ARC file buffer
 *
 * @param archive_buffer Input buffer containing a raw ARC file buffer
 * @param entry Entry to read from the ARC file buffer
 * @param out_buffer Pointer to a byte vector where the entry file buffer will be written
 */
Result ReadEntryBuffer(const std::vector<uint8_t>& archive_buffer, const TabEntry& entry,
                       std::vector<uint8_t>* out_buffer);

/**
 * Write a single legacy entry to a legacy TAB & ARC file buffer
 *
 * @param out_tab_buffer Pointer to a raw legacy TAB file buffer where the entry will be written
 * @param out_arc_buffer Pointer to a raw ARC file buffer where the entry file buffer will be written
 * @param filename String containing the name of the legacy entry to write
 * @param file_buffer Entry file buffer to write to the ARC file buffer
 */
Result WriteEntry(std::vector<uint8_t>* out_tab_buffer, std::vector<uint8_t>* out_arc_buffer,
                  const std::string& filename, const std::vector<uint8_t>& file_buffer);
}; // namespace ava::legacy::ArchiveTable

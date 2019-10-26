#pragma once

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

void ReadTab(const std::vector<uint8_t>& buffer, std::vector<TabEntry>* out_entries);
bool ReadTabEntry(const std::vector<uint8_t>& buffer, uint32_t name_hash, TabEntry* out_entry);
void ReadEntryBufferFromArchive(const std::vector<uint8_t>& archive_buffer, const TabEntry& entry,
                                std::vector<uint8_t>* out_buffer);
}; // namespace ava::legacy::ArchiveTable

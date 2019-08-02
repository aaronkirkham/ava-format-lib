#pragma once

#include <cstdint>
#include <vector>

namespace ava::legacy::ArchiveTable
{
static constexpr uint32_t TAB_MAGIC = 0x424154; // "TAB"

#pragma pack(push, 1)
struct TabFileHeader {
    uint32_t m_Magic     = TAB_MAGIC;
    uint16_t m_Version   = 2;
    uint16_t m_Endian    = 1;
    int32_t  m_Alignment = 0x1000;
};

struct TabFileEntry {
    uint32_t m_NameHash;
    uint32_t m_Offset;
    uint32_t m_Size;
};
#pragma pack(pop)

static_assert(sizeof(TabFileHeader) == 0xC, "TabFileHeader (legacy) alignment is wrong!");
static_assert(sizeof(TabFileEntry) == 0xC, "TabFileEntry (legacy) alignment is wrong!");

void ReadTab(const std::vector<uint8_t>& buffer, std::vector<TabFileEntry>* out_entries);
}; // namespace ava::legacy::ArchiveTable

#pragma once

#include <cstdint>
#include <vector>

namespace ava::StreamArchive
{
static constexpr uint32_t SARC_MAGIC = 0x53415243; // "SARC"

#pragma pack(push, 1)
struct SarcFileHeader {
    uint32_t m_MagicLength = 4;
    uint32_t m_Magic       = SARC_MAGIC;
    uint32_t m_Version     = 0;
    uint32_t m_Size        = 0;
};
#pragma pack(pop)

struct ArchiveEntry_t {
    std::string m_Filename = "";
    uint32_t    m_Offset   = 0;
    uint32_t    m_Size     = 0;
};

static_assert(sizeof(SarcFileHeader) == 0x10, "SarcFileHeader alignment is wrong!");

void Parse(const std::vector<uint8_t>& buffer, std::vector<ArchiveEntry_t>* out_entries);
void ParseTOC(const std::vector<uint8_t>& buffer, std::vector<ArchiveEntry_t>* out_entries);
}; // namespace ava::StreamArchive

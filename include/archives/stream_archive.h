#pragma once

#include <cstdint>
#include <vector>

namespace ava::StreamArchive
{
static constexpr uint32_t SARC_MAGIC              = 0x43524153; // "SARC"
static constexpr uint8_t  SARC_ENTRY_PADDING_BYTE = 0x0;

#pragma pack(push, 1)
struct SarcHeader {
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

static_assert(sizeof(SarcHeader) == 0x10, "SarcHeader alignment is wrong!");

void Parse(const std::vector<uint8_t>& buffer, std::vector<ArchiveEntry_t>* out_entries);
void ParseTOC(const std::vector<uint8_t>& buffer, std::vector<ArchiveEntry_t>* out_entries);

void ReadEntry(const std::vector<uint8_t>& buffer, const ArchiveEntry_t& entry, std::vector<uint8_t>* out_buffer);
void ReadEntry(const std::vector<uint8_t>& buffer, const std::vector<ArchiveEntry_t>& entries,
               const std::string& filename, std::vector<uint8_t>* out_buffer);

void WriteEntry(std::vector<uint8_t>& buffer, std::vector<ArchiveEntry_t>* entries, const std::string& filename,
                const std::vector<uint8_t>& file_buffer);
}; // namespace ava::StreamArchive

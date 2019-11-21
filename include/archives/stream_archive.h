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

struct ArchiveEntry {
    std::string m_Filename = "";
    uint32_t    m_Offset   = 0;
    uint32_t    m_Size     = 0;
};

static_assert(sizeof(SarcHeader) == 0x10, "SarcHeader alignment is wrong!");

/**
 * Parse a SARC file and extract file entries
 *
 * @param buffer Input buffer containing a raw SARC file buffer
 * @param out_entries Pointer to vector of ArchiveEntry's where the entries will be written
 */
void Parse(const std::vector<uint8_t>& buffer, std::vector<ArchiveEntry>* out_entries);

/**
 * Parse a patched SARC file list, commonly used with the .TOC extension
 *
 * @param buffer Input buffer containing a raw TOC file buffer
 * @param out_entries Pointer to vector of ArchiveEntry's where the entries will be written
 */
void ParseTOC(const std::vector<uint8_t>& buffer, std::vector<ArchiveEntry>* out_entries);

/**
 * Parse a patched SARC file list and update current entries vector, commonly used with the .TOC extension
 *
 * @param buffer Input buffer containing a raw TOC file buffer
 * @param entries Pointer to vector of ArchiveEntry's where the entries will be modified/written
 * @param out_total_added (Optional) Pointer to a uint32_t where a total number of added entrys will be written
 * @param out_total_patched (Optional) Pointer to a uint32_t where a total number of patched entrys will be written
 */
void ParseTOC(const std::vector<uint8_t>& buffer, std::vector<ArchiveEntry>* entries,
              uint32_t* out_total_added, uint32_t* out_total_patched);

/**
 * Init an empty buffer with a SARC header
 *
 * @param buffer Pointer to an empty input buffer
 * @param version SARC version number
 */
void InitBuffer(std::vector<uint8_t>* buffer, const uint32_t version = 2);

/**
 * Read the buffer of an Archive Entry
 *
 * @param buffer Input buffer containing a raw SARC file buffer
 * @param entry Entry to read buffer of
 * @param out_buffer Pointer to char vector where the output entry buffer will be written
 */
void ReadEntry(const std::vector<uint8_t>& buffer, const ArchiveEntry& entry, std::vector<uint8_t>* out_buffer);

/**
 * Read the buffer of an Archive Entry by filename
 *
 * @param buffer Input buffer containing a raw SARC file buffer
 * @param entries Vector of entries to read from, returned from Parse
 * @param filename String containing the name of the entry to read
 * @param out_buffer Pointer to char vector where the output entry buffer will be written
 */
void ReadEntry(const std::vector<uint8_t>& buffer, const std::vector<ArchiveEntry>& entries,
               const std::string& filename, std::vector<uint8_t>* out_buffer);

/**
 * Write a file to a SARC buffer
 *
 * @param buffer Input buffer containing a raw SARC file buffer
 * @param entries Vector of current archive entries, returned from Parse
 * @param filename String containing the name of the entry to write
 * @param file_buffer Input buffer containing the raw data for the file to write to the SARC buffer
 */
void WriteEntry(std::vector<uint8_t>* buffer, std::vector<ArchiveEntry>* entries, const std::string& filename,
                const std::vector<uint8_t>& file_buffer);

/**
 * Write a TOC file
 *
 * @param buffer Pointer to byte vector where the TOC buffer will be written
 * @param entries Vector of archive entries to write to the TOC
 */
void WriteTOC(std::vector<uint8_t>* buffer, const std::vector<ArchiveEntry>& entries);
}; // namespace ava::StreamArchive

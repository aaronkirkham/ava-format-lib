#include "../include/archive_table.h"

#include <fstream>

namespace ava::ArchiveTable
{
void ReadTab(std::filesystem::path& filename, std::vector<TabFileEntry>* out_entries
#ifndef USE_LEGACY_ARCHIVE_TABLE
             ,
             std::vector<TabFileCompressedBlock>* out_compressed_blocks
#endif
)
{
    std::ifstream stream(filename, std::ios::binary);
    if (stream.fail()) {
        throw std::runtime_error("Failed to open input file stream!");
    }

    // read header
    TabFileHeader header;
    stream.read((char*)&header, sizeof(TabFileHeader));
    if (header.m_Magic != TAB_MAGIC) {
        throw std::runtime_error("Invalid header magic! (Input file isn't .TAB?)");
    }

#ifndef USE_LEGACY_ARCHIVE_TABLE
    // read compressed blocks
    uint32_t num_compressed_blocks = 0;
    stream.read((char*)&num_compressed_blocks, sizeof(uint32_t));
    out_compressed_blocks->resize(num_compressed_blocks);
    stream.read((char*)out_compressed_blocks->data(), sizeof(TabFileCompressedBlock) * num_compressed_blocks);
#endif

#ifdef USE_LEGACY_ARCHIVE_TABLE
    while (!stream.eof()) {
#else
    const auto length = std::filesystem::file_size(filename);
    while (static_cast<int32_t>(stream.tellg()) + 20 <= length) {
#endif
        TabFileEntry entry;
        stream.read((char*)&entry, sizeof(TabFileEntry));

#ifdef USE_LEGACY_ARCHIVE_TABLE
        if (stream.fail()) {
            break;
        }
#endif

        out_entries->emplace_back(std::move(entry));
    }

    stream.close();
}

bool ReadTabEntry(const std::filesystem::path& filename, uint32_t name_hash, TabFileEntry* out_entry)
{
    std::vector<TabFileEntry> entries;
    ReadTab(filename, &entries, nullptr);

    const auto it = std::find_if(entries.begin(), entries.end(),
                                 [name_hash](const TabFileEntry& entry) { return entry.m_NameHash == name_hash; });
    if (it != entries.end()) {
        *out_entry = (*it);
        return true;
    }

    return false;
}

}; // namespace ava::ArchiveTable

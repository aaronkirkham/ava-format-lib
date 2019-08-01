#include "../../include/legacy/archive_table.h"

#include <fstream>

namespace ava::legacy::ArchiveTable
{
void ReadTab(std::filesystem::path& filename, std::vector<TabFileEntry>* out_entries)
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

    // read entries
    while (!stream.eof()) {
        TabFileEntry entry;
        stream.read((char*)&entry, sizeof(TabFileEntry));

        if (stream.fail()) {
            break;
        }

        out_entries->emplace_back(std::move(entry));
    }

    stream.close();
}
}; // namespace ava::legacy::ArchiveTable

#include "../../../include/archives/legacy/archive_table.h"

#include "../../../include/util/byte_array_buffer.h"
#include "../../../include/util/hashlittle.h"

namespace ava::legacy::ArchiveTable
{
void ReadTab(const std::vector<uint8_t>& buffer, std::vector<TabEntry>* out_entries)
{
    if (buffer.empty()) {
        throw std::invalid_argument("TAB input buffer can't be empty!");
    }

    if (!out_entries) {
        throw std::invalid_argument("TAB output entries vector can't be nullptr!");
    }

    byte_array_buffer buf(buffer.data(), buffer.size());
    std::istream      stream(&buf);

    // read header
    TabHeader header;
    stream.read((char*)&header, sizeof(TabHeader));
    if (header.m_Magic != TAB_MAGIC) {
        throw std::runtime_error("Invalid header magic! (Input file isn't .TAB?)");
    }

    // read entries
    while (!stream.eof()) {
        TabEntry entry;
        stream.read((char*)&entry, sizeof(TabEntry));

        if (stream.fail()) {
            break;
        }

        out_entries->emplace_back(std::move(entry));
    }
}
}; // namespace ava::legacy::ArchiveTable

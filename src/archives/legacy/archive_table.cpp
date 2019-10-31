#include "../../../include/archives/legacy/archive_table.h"

#include "../../../include/util/byte_array_buffer.h"
#include "../../../include/util/hashlittle.h"

#include <algorithm>

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

    byte_array_buffer buf(buffer);
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

bool ReadTabEntry(const std::vector<uint8_t>& buffer, uint32_t name_hash, TabEntry* out_entry)
{
    std::vector<TabEntry> entries;
    ReadTab(buffer, &entries);

    const auto it = std::find_if(entries.begin(), entries.end(),
                                 [name_hash](const TabEntry& entry) { return entry.m_NameHash == name_hash; });
    if (it != entries.end()) {
        *out_entry = (*it);
        return true;
    }

    return false;
}

void ReadEntryBufferFromArchive(const std::vector<uint8_t>& archive_buffer, const TabEntry& entry,
                                std::vector<uint8_t>* out_buffer)
{
    if (archive_buffer.empty()) {
        throw std::invalid_argument("input buffer can't be empty!");
    }

    if (!out_buffer) {
        throw std::invalid_argument("output buffer vector can't be nullptr!");
    }

    // copy the buffer from the input buffer
    assert(entry.m_Size != 0);
    out_buffer->resize(entry.m_Size);
    std::memcpy(out_buffer->data(), archive_buffer.data() + entry.m_Offset, entry.m_Size);
}
}; // namespace ava::legacy::ArchiveTable

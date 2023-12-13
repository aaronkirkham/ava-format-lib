#include <legacy/archive_table.h>

#include <util/byte_array_buffer.h>
#include <util/byte_vector_stream.h>
#include <util/hashlittle.h>

#include <algorithm>

namespace ava::legacy::ArchiveTable
{
Result Parse(const std::vector<uint8_t>& buffer, std::vector<TabEntry>* out_entries)
{
    if (buffer.empty() || !out_entries) {
        // throw std::invalid_argument("TAB input buffer can't be empty!");
        return E_INVALID_ARGUMENT;
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    TabHeader header;
    stream.read((char*)&header, sizeof(TabHeader));
    if (header.m_Magic != TAB_MAGIC) {
        // throw std::runtime_error("Invalid header magic! (Input file isn't .TAB?)");
        return E_TAB_INVALID_MAGIC;
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

    return E_OK;
}

Result ReadEntry(const std::vector<uint8_t>& buffer, uint32_t name_hash, TabEntry* out_entry)
{
    std::vector<TabEntry> entries;
    Parse(buffer, &entries);

    const auto it = std::find_if(entries.begin(), entries.end(),
                                 [name_hash](const TabEntry& entry) { return entry.m_NameHash == name_hash; });
    if (it == entries.end()) {
        // throw std::runtime_error("entry was not found in archive table!");
        return E_TAB_UNKNOWN_ENTRY;
    }

    *out_entry = (*it);
    return E_OK;
}

Result ReadEntryBuffer(const std::vector<uint8_t>& archive_buffer, const TabEntry& entry,
                       std::vector<uint8_t>* out_buffer)
{
    if (archive_buffer.empty() || !out_buffer) {
        // throw std::invalid_argument("input buffer can't be empty!");
        return E_INVALID_ARGUMENT;
    }

    // copy the buffer from the input buffer
    assert(entry.m_Size != 0);
    out_buffer->resize(entry.m_Size);
    std::memcpy(out_buffer->data(), archive_buffer.data() + entry.m_Offset, entry.m_Size);
    return E_OK;
}

Result WriteEntry(std::vector<uint8_t>* out_tab_buffer, std::vector<uint8_t>* out_arc_buffer,
                  const std::string& filename, const std::vector<uint8_t>& file_buffer)
{
    if (!out_tab_buffer || !out_arc_buffer || filename.empty() || file_buffer.empty()) {
        // throw std::invalid_argument("pointers to output tab/arc buffers can not be nullptr!");
        return E_INVALID_ARGUMENT;
    }

    utils::ByteVectorStream buf(out_tab_buffer);

    if (out_tab_buffer->empty()) {
        // write tab header
        TabHeader header;
        buf.write(header);
    }

    TabEntry entry{};
    entry.m_NameHash = ava::hashlittle(filename.c_str());
    entry.m_Offset   = static_cast<uint32_t>(out_arc_buffer->size());
    entry.m_Size     = static_cast<uint32_t>(file_buffer.size());

    // write the tab entry
    buf.write(entry);

    // write the file buffer
    std::copy(file_buffer.begin(), file_buffer.end(), std::back_inserter(*out_arc_buffer));
    return E_OK;
}
}; // namespace ava::legacy::ArchiveTable

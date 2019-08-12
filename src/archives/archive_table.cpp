#include "../../include/archives/archive_table.h"

#include "../../include/util/byte_array_buffer.h"
#include "../../include/util/hashlittle.h"

#include <algorithm>
#include <assert.h>

namespace ava::ArchiveTable
{
void ReadTab(const std::vector<uint8_t>& buffer, std::vector<TabEntry>* out_entries,
             std::vector<TabCompressedBlock>* out_compressed_blocks)
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
    TabHeader header{};
    stream.read((char*)&header, sizeof(TabHeader));
    if (header.m_Magic != TAB_MAGIC) {
        throw std::runtime_error("Invalid TAB header magic! (Input file isn't .TAB?)");
    }

    // read compressed blocks
    uint32_t num_compressed_blocks = 0;
    stream.read((char*)&num_compressed_blocks, sizeof(uint32_t));

    if (out_compressed_blocks) {
        out_compressed_blocks->resize(num_compressed_blocks);
        stream.read((char*)out_compressed_blocks->data(), sizeof(TabCompressedBlock) * num_compressed_blocks);
    } else {
        stream.ignore(sizeof(TabCompressedBlock) * num_compressed_blocks);
    }

    // read entries
    while (static_cast<int32_t>(stream.tellg()) + 20 <= buffer.size()) {
        TabEntry entry;
        stream.read((char*)&entry, sizeof(TabEntry));
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

#if 0
void ReadBufferFromArchive(const std::vector<uint8_t>& buffer, uint32_t name_hash, std::vector<uint8_t>* out_buffer)
{
    auto tab_filename = filename;
    tab_filename.replace_extension(".tab");

    // ensure the tab file exists
    if (!std::filesystem::exists(tab_filename)) {
        throw std::runtime_error("Can't find TAB file!");
    }

    // read the tab entry
    TabEntry entry{};
    if (!ReadTabEntry(tab_filename, name_hash, &entry)) {
        throw std::runtime_error("Can't find entry in TAB file!");
    }

    // try load the archive file
    std::ifstream stream(filename, std::ios::binary);
    if (stream.fail()) {
        throw std::runtime_error("Failed to open input ARC stream!");
    }

    stream.seekg(entry.m_Offset);

    // buffer is not compressed
    if (entry.m_CompressionType == CompressionType::CompressionType_None) {
        assert(entry.m_Size != 0);

        // read the buffer from the arc stream
        out_buffer->resize(entry.m_Size);
        stream.read((char*)out_buffer->data(), entry.m_Size);
        return;
    }

    // decompress buffer
    switch (entry.m_CompressionType) {
        case CompressionType::CompressionType_Zlib: {
            throw std::runtime_error("Zlib Decompression not implemented!");
            break;
        }

        case CompressionType::CompressionType_Oodle: {
            throw std::runtime_error("Oodle Decompression not implemented!");
            break;
        }
    }
}
#endif
}; // namespace ava::ArchiveTable

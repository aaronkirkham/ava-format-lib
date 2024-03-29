#include <archives/archive_table.h>
#include <archives/oodle_helper.h>

#include <util/byte_array_buffer.h>
#include <util/byte_vector_stream.h>
#include <util/hashlittle.h>

#include <algorithm>
#include <assert.h>

namespace ava::ArchiveTable
{
Result Parse(const std::vector<uint8_t>& buffer, std::vector<TabEntry>* out_entries,
             std::vector<TabCompressedBlock>* out_compression_blocks)
{
    if (buffer.empty() || !out_entries) {
        return E_INVALID_ARGUMENT;
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    TabHeader header{};
    stream.read((char*)&header, sizeof(TabHeader));
    if (header.m_Magic != TAB_MAGIC) {
        // throw std::runtime_error("Invalid TAB header magic! (Input file isn't .TAB?)");
        return E_TAB_INVALID_MAGIC;
    }

    // read compressed blocks
    uint32_t num_compressed_blocks = 0;
    stream.read((char*)&num_compressed_blocks, sizeof(uint32_t));

    if (out_compression_blocks) {
        out_compression_blocks->resize(num_compressed_blocks);
        stream.read((char*)out_compression_blocks->data(), sizeof(TabCompressedBlock) * num_compressed_blocks);
    } else {
        stream.ignore(sizeof(TabCompressedBlock) * num_compressed_blocks);
    }

    // read entries
    while (static_cast<int32_t>(stream.tellg()) + sizeof(TabEntry) <= buffer.size()) {
        TabEntry entry;
        stream.read((char*)&entry, sizeof(TabEntry));
        out_entries->emplace_back(std::move(entry));
    }

    return E_OK;
}

Result ReadEntry(const std::vector<uint8_t>& buffer, const uint32_t name_hash, TabEntry* out_entry)
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
                       std::vector<uint8_t>* out_buffer, const std::vector<TabCompressedBlock>& compression_blocks)
{
    if (archive_buffer.empty() || !out_buffer) {
        // throw std::invalid_argument("input buffer can't be empty!");
        return E_INVALID_ARGUMENT;
    }

    if (entry.m_CompressedBlockIndex != 0 && compression_blocks.empty()) {
        // throw std::invalid_argument("entry uses compression blocks, but none were passed.");
        return E_TAB_INPUT_REQUIRES_COMPRESSION_BLOCKS;
    }

    // entry isn't using compression, read directly from the buffer
    if (entry.m_Library == E_COMPRESS_LIBRARY_NONE) {
        assert(entry.m_Size != 0);
        out_buffer->resize(entry.m_Size);

        // copy the buffer from the input buffer
        std::memcpy(out_buffer->data(), archive_buffer.data() + entry.m_Offset, entry.m_Size);
    }
    // entry is using compression, decompress the entry buffer
    else {
        // figure out how much space we need (if compression blocks are used, we will get the total size of all blocks)
        const uint32_t size = GetEntryRequiredBufferSize(entry, compression_blocks);

        // copy the compressed buffer from the archive buffer
        std::vector<uint8_t> compressed_buffer(entry.m_Size);
        std::memcpy(compressed_buffer.data(), archive_buffer.data() + entry.m_Offset, size);

        return DecompressEntryBuffer(compressed_buffer, entry, out_buffer, compression_blocks);
    }

    return E_OK;
}

Result DecompressEntryBuffer(const std::vector<uint8_t>& buffer, const TabEntry& entry,
                             std::vector<uint8_t>*                  out_buffer,
                             const std::vector<TabCompressedBlock>& compression_blocks)
{
    if (buffer.empty() || !out_buffer) {
        // throw std::invalid_argument("input buffer can't be empty!");
        return E_INVALID_ARGUMENT;
    }

    if (entry.m_CompressedBlockIndex != 0 && compression_blocks.empty()) {
        // throw std::invalid_argument("entry uses compression blocks, but none were passed.");
        return E_TAB_INPUT_REQUIRES_COMPRESSION_BLOCKS;
    }

    // read the entry buffer from the input buffer
    switch (entry.m_Library) {
        case E_COMPRESS_LIBRARY_NONE: {
            assert(entry.m_Size != 0);
            out_buffer->resize(entry.m_Size);

            // copy the buffer from the input buffer
            std::memcpy(out_buffer->data(), buffer.data(), entry.m_Size);
            break;
        }

        case E_COMPRESS_LIBRARY_ZLIB: {
#ifdef _DEBUG
            __debugbreak();
#endif
            // throw std::runtime_error("Zlib Decompression not implemented!");
            return E_NOT_IMPLEMENTED;
            break;
        }

        case E_COMPRESS_LIBRARY_OODLE: {
            // entry is not using compression blocks
            if (entry.m_CompressedBlockIndex == 0) {
                // copy the compressed buffer from the arc input buffer
                std::vector<uint8_t> compressed_data(entry.m_Size);
                std::memcpy(compressed_data.data(), buffer.data(), entry.m_Size);

                assert(entry.m_Size != entry.m_UncompressedSize);

                // uncompress the buffer
                out_buffer->resize(entry.m_UncompressedSize);
                const int64_t size = ava::Oodle::Decompress(compressed_data.data(), entry.m_Size, out_buffer->data(),
                                                            entry.m_UncompressedSize);

                // ensure the decompressed amount what we expected
                if (size != entry.m_UncompressedSize) {
#ifdef _DEBUG
                    __debugbreak();
#endif
                    out_buffer->clear();
                    return E_TAB_DECOMPRESS_BLOCK_FAILED;
                }
            }
            // entry is using compression blocks
            else {
                out_buffer->resize(entry.m_UncompressedSize);

                uint16_t current_block_index     = entry.m_CompressedBlockIndex;
                uint32_t archive_buffer_offset   = 0;
                uint32_t total_compressed_size   = entry.m_Size;
                uint32_t total_uncompressed_size = 0;

                while (total_compressed_size > 0) {
                    const TabCompressedBlock& block = compression_blocks.at(current_block_index);

                    // read the compressed block
                    std::vector<uint8_t> block_data(block.m_CompressedSize);
                    std::memcpy(block_data.data(), buffer.data() + archive_buffer_offset, block.m_CompressedSize);

                    // decompress the block data
                    const int64_t size =
                        ava::Oodle::Decompress(block_data.data(), block.m_CompressedSize,
                                               out_buffer->data() + total_uncompressed_size, block.m_UncompressedSize);
                    if (size != block.m_UncompressedSize) {
#ifdef _DEBUG
                        __debugbreak();
#endif
                        out_buffer->clear();
                        return E_TAB_DECOMPRESS_BLOCK_FAILED;
                    }

                    total_compressed_size -= block.m_CompressedSize;
                    total_uncompressed_size += block.m_UncompressedSize;
                    archive_buffer_offset += block.m_CompressedSize;
                    current_block_index++;
                }
            }

            break;
        }
    }

    return E_OK;
}

uint32_t GetEntryRequiredBufferSize(const TabEntry& entry, const std::vector<TabCompressedBlock>& compression_blocks)
{
    if (entry.m_Library != E_COMPRESS_LIBRARY_NONE || entry.m_CompressedBlockIndex == 0) {
        return entry.m_Size;
    }

    if (compression_blocks.empty()) {
        // TODO : should this be an error?
        return 0;
    }

    uint16_t current_block_index   = entry.m_CompressedBlockIndex;
    uint32_t total_compressed_size = entry.m_Size;
    uint32_t remaining             = entry.m_Size;

    while (remaining > 0) {
        const TabCompressedBlock& block = compression_blocks.at(current_block_index);

        remaining -= block.m_CompressedSize;
        total_compressed_size += block.m_CompressedSize;
        ++current_block_index;
    }

    return total_compressed_size;
}

Result WriteEntry(std::vector<uint8_t>* out_tab_buffer, std::vector<uint8_t>* out_arc_buffer,
                  const std::string& filename, const std::vector<uint8_t>& file_buffer, ECompressLibrary compression)
{
    if (!out_tab_buffer || !out_arc_buffer || filename.empty() || file_buffer.empty()) {
        return E_INVALID_ARGUMENT;
    }

    utils::ByteVectorStream buf(out_tab_buffer);

    if (out_tab_buffer->empty()) {
        // write tab header
        TabHeader header;
        buf.write(header);

        // write compressed blocks
        // @TODO: figure this out!
        uint32_t compressed_block_count = 0;
        // TabCompressedBlock compressed_block{0xFFFFFFFF, 0xFFFFFFFF};
        buf.write(compressed_block_count);
        // buf.write((char*)&compressed_block, sizeof(TabCompressedBlock), compressed_block_count);
    }

    TabEntry entry{};
    entry.m_NameHash             = ava::hashlittle(filename.c_str());
    entry.m_Offset               = static_cast<uint32_t>(out_arc_buffer->size());
    entry.m_Size                 = static_cast<uint32_t>(file_buffer.size());
    entry.m_UncompressedSize     = entry.m_Size;
    entry.m_CompressedBlockIndex = 0;
    entry.m_Library              = compression;
    entry.m_Flags                = E_ENTRY_FLAG_DECODE_NONE;

    switch (compression) {
        case E_COMPRESS_LIBRARY_NONE: {
            // TODO : memcpy
            std::copy(file_buffer.begin(), file_buffer.end(), std::back_inserter(*out_arc_buffer));
            break;
        }

        case E_COMPRESS_LIBRARY_ZLIB: {
#ifdef _DEBUG
            __debugbreak();
#endif
            // throw std::runtime_error("Zlib Crompression not implemented!");
            return E_NOT_IMPLEMENTED;
            break;
        }

        case E_COMPRESS_LIBRARY_OODLE: {
            // entry is not using compression blocks
            if (entry.m_CompressedBlockIndex == 0) {
                std::vector<uint8_t> compressed_data;
                const int64_t        size = ava::Oodle::Compress(&file_buffer, &compressed_data);

                // update entry
                entry.m_Size  = static_cast<uint32_t>(size);
                entry.m_Flags = E_ENTRY_FLAG_DECODE_BUFFER;

                if (size == 0) {
#ifdef _DEBUG
                    __debugbreak();
#endif
                    return E_TAB_COMPRESS_BLOCK_FAILED;
                }

                // write the compressed buffer to the arc buffer
                // TODO : memcpy
                std::copy(compressed_data.begin(), compressed_data.end(), std::back_inserter(*out_arc_buffer));
            } else {
#ifdef _DEBUG
                __debugbreak();
#endif
                return E_NOT_IMPLEMENTED;
                // throw std::runtime_error("CompressionType_Oodle: Compression blocks not implemented!");
            }

            break;
        }
    }

    // write the tab entry
    buf.write(entry);
    return E_OK;
}
}; // namespace ava::ArchiveTable

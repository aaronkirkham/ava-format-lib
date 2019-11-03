#include "../../include/archives/archive_table.h"

#include "../../include/archives/oodle_helper.h"

#include "../../include/util/byte_array_buffer.h"
#include "../../include/util/byte_vector_writer.h"
#include "../../include/util/hashlittle.h"

#include <algorithm>
#include <assert.h>

namespace ava::ArchiveTable
{
void Parse(const std::vector<uint8_t>& buffer, std::vector<TabEntry>* out_entries,
           std::vector<TabCompressedBlock>* out_compression_blocks)
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
    TabHeader header{};
    stream.read((char*)&header, sizeof(TabHeader));
    if (header.m_Magic != TAB_MAGIC) {
        throw std::runtime_error("Invalid TAB header magic! (Input file isn't .TAB?)");
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
}

void ReadEntry(const std::vector<uint8_t>& buffer, const uint32_t name_hash, TabEntry* out_entry)
{
    std::vector<TabEntry> entries;
    Parse(buffer, &entries);

    const auto it = std::find_if(entries.begin(), entries.end(),
                                 [name_hash](const TabEntry& entry) { return entry.m_NameHash == name_hash; });
    if (it == entries.end()) {
        throw std::runtime_error("entry was not found in archive table!");
    }

    *out_entry = (*it);
}

void ReadEntryBuffer(const std::vector<uint8_t>& archive_buffer, const TabEntry& entry,
                     const std ::vector<TabCompressedBlock>* compression_blocks, std::vector<uint8_t>* out_buffer)
{
    if (archive_buffer.empty()) {
        throw std::invalid_argument("input buffer can't be empty!");
    }

    if (!out_buffer) {
        throw std::invalid_argument("output buffer vector can't be nullptr!");
    }

    if (entry.m_CompressedBlockIndex != 0 && (!compression_blocks || compression_blocks->empty())) {
        throw std::invalid_argument("entry uses compression blocks, but none were passed.");
    }

    // read the entry buffer from the input buffer
    switch (entry.m_Library) {
        case E_COMPRESS_LIBRARY_NONE: {
            assert(entry.m_Size != 0);
            out_buffer->resize(entry.m_Size);

            // copy the buffer from the input buffer
            std::memcpy(out_buffer->data(), archive_buffer.data() + entry.m_Offset, entry.m_Size);
            break;
        }

        case E_COMPRESS_LIBRARY_ZLIB: {
#ifdef _DEBUG
            __debugbreak();
#endif
            throw std::runtime_error("Zlib Decompression not implemented!");
            break;
        }

        case E_COMPRESS_LIBRARY_OODLE: {
            // entry is not using compression blocks
            if (entry.m_CompressedBlockIndex == 0) {
                // copy the compressed buffer from the arc input buffer
                std::vector<uint8_t> compressed_data(entry.m_Size);
                std::memcpy(compressed_data.data(), archive_buffer.data() + entry.m_Offset, entry.m_Size);

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
                    throw std::runtime_error("CompressionType_Oodle: Failed to decompress the buffer.");
                }
            }
            // entry is using compression blocks
            else {
                out_buffer->resize(entry.m_UncompressedSize);

                uint16_t current_block_index     = entry.m_CompressedBlockIndex;
                uint32_t archive_buffer_offset   = entry.m_Offset;
                uint32_t total_compressed_size   = entry.m_Size;
                uint32_t total_uncompressed_size = 0;

                while (total_compressed_size > 0) {
                    const TabCompressedBlock& block = compression_blocks->at(current_block_index);

                    // read the compressed block
                    std::vector<uint8_t> block_data(block.m_CompressedSize);
                    std::memcpy(block_data.data(), archive_buffer.data() + archive_buffer_offset,
                                block.m_CompressedSize);

                    // decompress the block data
                    const int64_t size =
                        ava::Oodle::Decompress(block_data.data(), block.m_CompressedSize,
                                               out_buffer->data() + total_uncompressed_size, block.m_UncompressedSize);
                    if (size != block.m_UncompressedSize) {
#ifdef _DEBUG
                        __debugbreak();
#endif
                        out_buffer->clear();
                        throw std::runtime_error("CompressionType_Oodle: Failed to decompress block buffer");
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
}

void WriteEntry(const std::string& filename, const std::vector<uint8_t>& file_buffer,
                std::vector<uint8_t>* out_tab_buffer, std::vector<uint8_t>* out_arc_buffer,
                ECompressLibrary compression)
{
    if (filename.empty()) {
        throw std::invalid_argument("filename string can not be empty!");
    }

    if (file_buffer.empty()) {
        throw std::invalid_argument("input file buffer can not be empty!");
    }

    if (!out_tab_buffer || !out_arc_buffer) {
        throw std::invalid_argument("pointers to output tab/arc buffers can not be nullptr!");
    }

    byte_vector_writer buf(out_tab_buffer);

    if (out_tab_buffer->empty()) {
        // write tab header
        TabHeader header;
        buf.write((char*)&header, sizeof(TabHeader));

        // write compressed blocks
        // @TODO: figure this out!
        uint32_t compressed_block_count = 0;
        // TabCompressedBlock compressed_block{0xFFFFFFFF, 0xFFFFFFFF};
        buf.write((char*)&compressed_block_count, sizeof(uint32_t));
        // buf.write((char*)&compressed_block, sizeof(TabCompressedBlock), compressed_block_count);
    }

    TabEntry entry{};
    entry.m_NameHash             = hashlittle(filename.c_str());
    entry.m_Offset               = static_cast<uint32_t>(out_arc_buffer->size());
    entry.m_Size                 = static_cast<uint32_t>(file_buffer.size());
    entry.m_UncompressedSize     = entry.m_Size;
    entry.m_CompressedBlockIndex = 0;
    entry.m_Library              = compression;
    entry.m_Flags                = E_ENTRY_FLAG_DECODE_NONE;

    switch (compression) {
        case E_COMPRESS_LIBRARY_NONE: {
            std::copy(file_buffer.begin(), file_buffer.end(), std::back_inserter(*out_arc_buffer));
            break;
        }

        case E_COMPRESS_LIBRARY_ZLIB: {
#ifdef _DEBUG
            __debugbreak();
#endif
            throw std::runtime_error("Zlib Crompression not implemented!");
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
                    throw std::runtime_error("CompressionType_Oodle: Failed to compress the buffer.");
                }

                // write the compressed buffer to the arc buffer
                std::copy(compressed_data.begin(), compressed_data.end(), std::back_inserter(*out_arc_buffer));
            } else {
#ifdef _DEBUG
                __debugbreak();
#endif
                throw std::runtime_error("CompressionType_Oodle: Compression blocks not implemented!");
            }

            break;
        }
    }

    // write the tab entry
    buf.write((char*)&entry, sizeof(TabEntry));
}
}; // namespace ava::ArchiveTable

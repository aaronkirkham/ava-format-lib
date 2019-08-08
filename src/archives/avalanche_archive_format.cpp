#include "../../include/archives/avalanche_archive_format.h"

#include "../../include/util/byte_array_buffer.h"
#include "../../include/util/hashlittle.h"

namespace ava::AvalancheArchiveFormat
{
/**
 * Decompress Avalanche Archive Format
 *
 * @param buffer Input buffer containing a raw AAF file buffer
 * @param out_buffer Pointer to char vector where the decompressed buffer will be written
 */
void Parse(const std::vector<uint8_t>& buffer, std::vector<uint8_t>* out_buffer)
{
    if (buffer.empty()) {
        throw std::invalid_argument("AAF input buffer can't be empty!");
    }

    if (!out_buffer) {
        throw std::invalid_argument("AAF output buffer can't be nullptr!");
    }

    byte_array_buffer buf(buffer.data(), buffer.size());
    std::istream      stream(&buf);

    // read header
    AafFileHeader header;
    stream.read((char*)&header, sizeof(AafFileHeader));
    if (header.m_Magic != AAF_MAGIC) {
        throw std::runtime_error("Invalid AAF header magic!");
    }

    out_buffer->reserve(header.m_TotalUncompressedSize);

    // read all the chunks
    for (uint32_t i = 0; i < header.m_ChunkCount; ++i) {
        const auto start_pos = stream.tellg();

        AafFileChunk chunk;
        stream.read((char*)&chunk, sizeof(AafFileChunk));
        if (chunk.m_Magic != AAF_CHUNK_MAGIC) {
            throw std::runtime_error("Invalid AAF chunk magic!");
        }

        // read the chunk data
        std::vector<uint8_t> chunk_data(chunk.m_CompressedSize);
        stream.read((char*)chunk_data.data(), chunk.m_CompressedSize);

#if 0
        // decompress the chunk
        auto                 compressed_size   = chunk.m_CompressedSize;
        auto                 decompressed_size = chunk.m_UncompressedSize;
        std::vector<uint8_t> decompressed_chunk_data(decompressed_size);
        const auto           result =
            uncompress(decompressed_chunk_data.data(), &decompressed_size, chunk_data.data(), compressed_size);

        if (result != Z_OK || decompressed_size != chunk.m_UncompressedSize) {
#ifdef _DEBUG
            __debugbreak();
#endif
            throw std::runtime_error("Failed to decompress an AAF chunk!");
        }

        out_buffer->insert(out_buffer->end(), decompressed_chunk_data.begin(), decompressed_chunk_data.end());
        stream.seekg((uint64_t)start_pos + chunk.m_DataSize);
#endif
    }
}
}; // namespace ava::AvalancheArchiveFormat

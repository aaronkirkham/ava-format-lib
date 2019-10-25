#include "../../include/archives/avalanche_archive_format.h"

#include "../../include/util/byte_array_buffer.h"
#include "../../include/util/byte_vector_writer.h"
#include "../../include/util/hashlittle.h"
#include "../../include/util/math.h"

#include <zlib/zlib.h>

namespace ava::AvalancheArchiveFormat
{
/**
 * Compress Avalanche Archive Format
 *
 * @param buffer Input buffer containing a raw file buffer
 * @param out_buffer Pointer to char vector where the compressed buffer will be written
 */
void Compress(const std::vector<uint8_t>& buffer, std::vector<uint8_t>* out_buffer)
{
    if (buffer.empty()) {
        throw std::invalid_argument("AAF compress input buffer can't be empty!");
    }

    if (!out_buffer) {
        throw std::invalid_argument("AAF compress output buffer can't be nullptr!");
    }

    const uint32_t buffer_size         = static_cast<uint32_t>(buffer.size());
    const uint32_t num_chunks          = (1 + (buffer_size / AAF_MAX_CHUNK_DATA_SIZE));
    const bool     has_multiple_chunks = (num_chunks > 1);

    AafHeader header;
    header.m_TotalUncompressedSize  = buffer_size;
    header.m_UncompressedBufferSize = (has_multiple_chunks ? AAF_MAX_CHUNK_DATA_SIZE : buffer_size);
    header.m_ChunkCount             = num_chunks;

    byte_vector_writer buf(out_buffer);
    buf.write(&header, sizeof(AafHeader));

    // create the chunks
    std::vector<AafChunk> chunks;
    uint32_t              last_chunk_offset = 0;
    for (uint32_t i = 0; i < num_chunks; ++i) {
        AafChunk             chunk;
        std::vector<uint8_t> chunk_data;

        // split the data if we have more than 1 chunk
        if (has_multiple_chunks) {
            // calculate the current chunk size
            const uint32_t block_size = (buffer_size - last_chunk_offset);
            const uint32_t chunk_size =
                (block_size > AAF_MAX_CHUNK_DATA_SIZE ? (AAF_MAX_CHUNK_DATA_SIZE % block_size) : block_size);

            // copy the current chunk data
            chunk.m_UncompressedSize = chunk_size;
            chunk_data               = std::vector<uint8_t>(buffer.begin() + last_chunk_offset,
                                              buffer.begin() + last_chunk_offset + chunk_size);
            last_chunk_offset += chunk_size;
        }
        // we only have a single chunk, the chunk will contain the whole buffer
        else {
            chunk.m_UncompressedSize = buffer_size;
        }

        // compress the current chunk data
        uLong                decompressed_size = static_cast<uLong>(chunk.m_UncompressedSize);
        uLong                compressed_size   = compressBound(decompressed_size);
        std::vector<uint8_t> compressed_block(compressed_size);

        const auto result = compress(compressed_block.data(), &compressed_size,
                                     (has_multiple_chunks ? chunk_data.data() : buffer.data()), decompressed_size);
        if (result != Z_OK) {
#ifdef _DEBUG
            __debugbreak();
#endif
            throw std::runtime_error("Failed to compress an AAF chunk!");
        }

        // NOTE: we remove 6 bytes as zlib will automatically write 2 bytes at the front (CMF & FLG) and 4 bytes at the
        // back (ADLER32) of our compressed buffer, which we do not want.
        chunk.m_CompressedSize = (compressed_size - 6);

        // calculate the padding and data size
        const uint32_t padding =
            ava::math::align_distance((buf.tellp() + sizeof(AafChunk) + chunk.m_CompressedSize), 16);
        chunk.m_DataSize = (sizeof(AafChunk) + chunk.m_CompressedSize + padding);

        // write chunk and compressed data
        // @NOTE: +2 to skip CMF & FLG
        buf.write((char*)&chunk, sizeof(AafChunk));
        buf.write(compressed_block.data() + 2, chunk.m_CompressedSize);

        // write block padding
        for (uint32_t i = 0; i < padding; ++i) {
            buf.write((char*)&AAF_PADDING_BYTE, 1); // @TODO: improve this, each write will call vector::resize
        }
    }
}

/**
 * Decompress Avalanche Archive Format
 *
 * @param buffer Input buffer containing a raw AAF file buffer
 * @param out_buffer Pointer to char vector where the decompressed buffer will be written
 */
void Decompress(const std::vector<uint8_t>& buffer, std::vector<uint8_t>* out_buffer)
{
    if (buffer.empty()) {
        throw std::invalid_argument("AAF decompress input buffer can't be empty!");
    }

    if (!out_buffer) {
        throw std::invalid_argument("AAF decompress output buffer can't be nullptr!");
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    AafHeader header;
    stream.read((char*)&header, sizeof(AafHeader));
    if (header.m_Magic != AAF_MAGIC) {
        throw std::runtime_error("Invalid AAF header magic!");
    }

    out_buffer->reserve(header.m_TotalUncompressedSize);

    // read all the chunks
    for (uint32_t i = 0; i < header.m_ChunkCount; ++i) {
        const auto start_pos = stream.tellg();

        AafChunk chunk;
        stream.read((char*)&chunk, sizeof(AafChunk));
        if (chunk.m_Magic != AAF_CHUNK_MAGIC) {
            throw std::runtime_error("Invalid AAF chunk magic!");
        }

        // read the chunk data
        std::vector<uint8_t> chunk_data(chunk.m_CompressedSize);
        stream.read((char*)chunk_data.data(), chunk.m_CompressedSize);

        // decompress the chunk
        auto                 compressed_size   = static_cast<uLong>(chunk.m_CompressedSize);
        auto                 decompressed_size = static_cast<uLong>(chunk.m_UncompressedSize);
        std::vector<uint8_t> decompressed_chunk_data(decompressed_size);

        const auto result =
            uncompress(decompressed_chunk_data.data(), &decompressed_size, chunk_data.data(), compressed_size);
        if (result != Z_OK || decompressed_size != chunk.m_UncompressedSize) {
#ifdef _DEBUG
            __debugbreak();
#endif
            throw std::runtime_error("Failed to decompress an AAF chunk!");
        }

        out_buffer->insert(out_buffer->end(), decompressed_chunk_data.begin(), decompressed_chunk_data.end());
        stream.seekg((uint64_t)start_pos + chunk.m_DataSize);
    }
}
}; // namespace ava::AvalancheArchiveFormat

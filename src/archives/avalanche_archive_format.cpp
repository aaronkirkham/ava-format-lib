#include <archives/avalanche_archive_format.h>

#include <util/byte_array_buffer.h>
#include <util/byte_vector_stream.h>
#include <util/hashlittle.h>
#include <util/math.h>
#include <util/zlib.h>

namespace ava::AvalancheArchiveFormat
{
bool IsCompressed(const std::vector<uint8_t>& buffer)
{
    return *(uint32_t*)buffer.data() == AAF_MAGIC;
}

Result Compress(const std::vector<uint8_t>& buffer, std::vector<uint8_t>* out_buffer)
{
    if (buffer.empty() || !out_buffer) {
        // throw std::invalid_argument("AAF compress input buffer can't be empty!");
        return E_INVALID_ARGUMENT;
    }

    const uint32_t buffer_size         = static_cast<uint32_t>(buffer.size());
    const uint32_t num_chunks          = (1 + (buffer_size / AAF_MAX_CHUNK_DATA_SIZE));
    const bool     has_multiple_chunks = (num_chunks > 1);

    AafHeader header;
    header.m_TotalUnpackedSize        = buffer_size;
    header.m_RequiredUnpackBufferSize = (has_multiple_chunks ? AAF_MAX_CHUNK_DATA_SIZE : buffer_size);
    header.m_NumChunks                = num_chunks;

    utils::ByteVectorStream buf(out_buffer);
    buf.write(header);

    // create the chunks
    uint32_t last_chunk_offset = 0;
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
            chunk.m_DecompressedSize = chunk_size;
            chunk_data               = std::vector<uint8_t>(buffer.begin() + last_chunk_offset,
                                              buffer.begin() + last_chunk_offset + chunk_size);
            last_chunk_offset += chunk_size;
        }
        // we only have a single chunk, the chunk will contain the whole buffer
        else {
            chunk.m_DecompressedSize = buffer_size;
        }

        // compress the current chunk data
        uint32_t             compressed_size = compressBound(chunk.m_DecompressedSize);
        std::vector<uint8_t> compressed_block(compressed_size);
        const int32_t        result = ava::zlib::Compress((has_multiple_chunks ? chunk_data.data() : buffer.data()),
                                                          chunk.m_DecompressedSize, compressed_block.data(), &compressed_size);
        if (result != Z_OK) {
#ifdef _DEBUG
            __debugbreak();
#endif
            // throw std::runtime_error("Failed to compress an AAF chunk!");
            return E_AAF_COMPRESS_CHUNK_FAILED;
        }

        // update the chunk compressed size
        chunk.m_CompressedSize = compressed_size;

        // calculate the padding and data size
        const uint32_t padding =
            ava::math::align_distance((buf.tellp() + sizeof(AafChunk) + chunk.m_CompressedSize), 16);
        chunk.m_ChunkSize = (sizeof(AafChunk) + chunk.m_CompressedSize + padding);

        // write chunk and compressed data
        buf.write(chunk);
        buf.write(compressed_block.data(), chunk.m_CompressedSize);

        // write block padding
        buf.write((char*)&AAF_PADDING_BYTE, 1, padding);
    }

    return E_OK;
}

Result Decompress(const std::vector<uint8_t>& buffer, std::vector<uint8_t>* out_buffer)
{
    if (buffer.empty() || !out_buffer) {
        // throw std::invalid_argument("AAF decompress input buffer can't be empty!");
        return E_INVALID_ARGUMENT;
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    AafHeader header{};
    stream.read((char*)&header, sizeof(AafHeader));
    if (header.m_Magic != AAF_MAGIC) {
        // throw std::runtime_error("Invalid AAF header magic!");
        return E_AAF_INVALID_MAGIC;
    }

    out_buffer->reserve(header.m_TotalUnpackedSize);

    // read all the chunks
    uint32_t buffer_offset = 0;
    for (uint32_t i = 0; i < header.m_NumChunks; ++i) {
        const auto start_pos = stream.tellg();

        AafChunk chunk;
        stream.read((char*)&chunk, sizeof(AafChunk));
        if (chunk.m_Magic != AAF_CHUNK_MAGIC) {
            // throw std::runtime_error("Invalid AAF chunk magic!");
            return E_AAF_INVALID_CHUNK_MAGIC;
        }

        // read the chunk data
        std::vector<uint8_t> chunk_data(chunk.m_CompressedSize);
        stream.read((char*)chunk_data.data(), chunk.m_CompressedSize);

        // decompress the chunk
        uint32_t             compressed_size   = chunk.m_CompressedSize;
        uint32_t             decompressed_size = chunk.m_DecompressedSize;
        std::vector<uint8_t> decompressed_chunk_data(decompressed_size);

        const int32_t result = ava::zlib::Decompress(chunk_data.data(), &compressed_size,
                                                     decompressed_chunk_data.data(), &decompressed_size);
        if (result != Z_OK || decompressed_size != chunk.m_DecompressedSize) {
#ifdef _DEBUG
            __debugbreak();
#endif
            // throw std::runtime_error("Failed to decompress an AAF chunk!");
            return E_AAF_DECOMPRESS_CHUNK_FAILED;
        }

        out_buffer->insert(out_buffer->end(), decompressed_chunk_data.begin(), decompressed_chunk_data.end());
        stream.seekg((uint64_t)start_pos + chunk.m_ChunkSize);
    }

    return E_OK;
}
}; // namespace ava::AvalancheArchiveFormat

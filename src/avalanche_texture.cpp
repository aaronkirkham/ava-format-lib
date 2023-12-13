#include <avalanche_texture.h>

#include <util/byte_array_buffer.h>
#include <util/byte_vector_stream.h>
#include <util/hashlittle.h>

namespace ava::AvalancheTexture
{
Result ReadBestEntry(const std::vector<uint8_t>& buffer, TextureEntry* out_entry, std::vector<uint8_t>* out_buffer,
                     const std::vector<uint8_t>& source_buffer)
{
    if (buffer.empty() || !out_entry || !out_buffer) {
        // throw std::invalid_argument("AVTX input buffer can't be empty!");
        return E_INVALID_ARGUMENT;
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    AvtxHeader header{};
    stream.read((char*)&header, sizeof(AvtxHeader));
    if (header.m_Magic != AVTX_MAGIC) {
        // throw std::runtime_error("Invalid AVTX header magic!");
        return E_AVTX_INVALID_MAGIC;
    }

    if (header.m_Version != 1) {
        // throw std::runtime_error("Invalid AVTX version!");
        return E_AVTX_UNKNOWN_VERSION;
    }

    // find the best stream to use
    const uint8_t     stream_index = FindBestStream(header, !source_buffer.empty());
    const uint32_t    rank         = GetRank(header, stream_index);
    const AvtxStream& avtx_stream  = header.m_Streams[stream_index];

    TextureEntry entry{};
    entry.m_Width  = (header.m_Width >> rank);
    entry.m_Height = (header.m_Height >> rank);
    entry.m_Depth  = (header.m_Depth >> rank);
    entry.m_Format = header.m_Format;
    entry.m_Source = avtx_stream.m_Source;
    *out_entry     = entry;

    out_buffer->resize(avtx_stream.m_Size);

    // copy the pixel data
    const auto start = (avtx_stream.m_Source ? source_buffer.data() : buffer.data()) + avtx_stream.m_Offset;
    std::memcpy(out_buffer->data(), start, avtx_stream.m_Size);
    return E_OK;
}

Result ReadEntry(const std::vector<uint8_t>& buffer, const uint8_t stream_index, TextureEntry* out_entry,
                 std::vector<uint8_t>* out_buffer, const std::vector<uint8_t>& source_buffer)
{
    if (buffer.empty() || stream_index >= AVTX_MAX_STREAMS || !out_entry || !out_buffer) {
        // throw std::invalid_argument("AVTX input buffer can't be empty!");
        return E_INVALID_ARGUMENT;
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    AvtxHeader header{};
    stream.read((char*)&header, sizeof(AvtxHeader));
    if (header.m_Magic != AVTX_MAGIC) {
        // throw std::runtime_error("Invalid AVTX header magic!");
        return E_AVTX_INVALID_MAGIC;
    }

    if (header.m_Version != 1) {
        // throw std::runtime_error("Invalid AVTX version!");
        return E_AVTX_UNKNOWN_VERSION;
    }

    // the requested stream was from source, and we provided no source buffer
    const AvtxStream& avtx_stream = header.m_Streams[stream_index];
    if (avtx_stream.m_Source && source_buffer.empty()) {
        // throw std::runtime_error("AVTX source buffer required!");
        return E_AVTX_SOURCE_BUFFER_NEEDED;
    }

    const uint32_t rank = GetRank(header, stream_index);

    TextureEntry entry{};
    entry.m_Width  = (header.m_Width >> rank);
    entry.m_Height = (header.m_Height >> rank);
    entry.m_Depth  = (header.m_Depth >> rank);
    entry.m_Format = header.m_Format;
    entry.m_Source = avtx_stream.m_Source;
    *out_entry     = entry;

    out_buffer->resize(avtx_stream.m_Size);

    // copy the pixel data
    const auto start = (avtx_stream.m_Source ? source_buffer.data() : buffer.data()) + avtx_stream.m_Offset;
    std::memcpy(out_buffer->data(), start, avtx_stream.m_Size);
    return E_OK;
}

Result WriteEntry(std::vector<uint8_t>* buffer, const TextureEntry& entry, const std::vector<uint8_t>& texture_buffer,
                  std::vector<uint8_t>* source_buffer)
{
    if (!buffer || texture_buffer.empty() || (entry.m_Source && !source_buffer)) {
        // throw std::invalid_argument("pointer to output AVTX buffer can not be nullptr!");
        return E_INVALID_ARGUMENT;
    }

    // @TODO
    return E_OK;
}

uint8_t FindBestStream(const AvtxHeader& header, uint8_t source)
{
    uint8_t biggest      = 0;
    uint8_t stream_index = 0;

    for (uint32_t i = 0; i < AVTX_MAX_STREAMS; ++i) {
        const AvtxStream& stream = header.m_Streams[i];
        if (stream.m_Size == 0) {
            continue;
        }

        if ((stream.m_Source == source) || (!stream.m_Source && stream.m_Size > biggest)) {
            biggest      = stream.m_Size;
            stream_index = i;
        }
    }

    assert(stream_index < AVTX_MAX_STREAMS);
    return stream_index;
}

uint32_t GetRank(const AvtxHeader& header, uint8_t stream_index)
{
    return (header.m_Mips - (header.m_MipsRedisent + stream_index));
}
}; // namespace ava::AvalancheTexture

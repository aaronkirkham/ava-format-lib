#include "../include/avalanche_texture.h"

#include "../include/util/byte_array_buffer.h"
#include "../include/util/hashlittle.h"

namespace ava::AvalancheTexture
{
void ReadBestEntry(const std::vector<uint8_t>& buffer, TextureEntry* out_entry, std::vector<uint8_t>* out_buffer,
                   const std::vector<uint8_t>& source_buffer)
{
    if (buffer.empty()) {
        throw std::invalid_argument("AVTX input buffer can't be empty!");
    }

    if (!out_entry) {
        throw std::invalid_argument("AVTX output entry can't be nullptr!");
    }

    if (!out_buffer) {
        throw std::invalid_argument("AVTX output buffer can't be nullptr!");
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    AvtxHeader header{};
    stream.read((char*)&header, sizeof(AvtxHeader));
    if (header.m_Magic != AVTX_MAGIC) {
        throw std::runtime_error("Invalid AVTX header magic!");
    }

    if (header.m_Version != 1) {
        throw std::runtime_error("Invalid AVTX version!");
    }

    // find the best stream to use
    const uint8_t stream_index = FindBestStream(header, !source_buffer.empty());
    assert(stream_index < AVTX_MAX_STREAMS);
    const uint32_t    rank        = GetRank(header, stream_index);
    const AvtxStream& avtx_stream = header.m_Streams[stream_index];

    TextureEntry entry{};
    entry.m_Width  = (header.m_Width >> rank);
    entry.m_Height = (header.m_Height >> rank);
    entry.m_Depth  = (header.m_Depth >> rank);
    entry.m_Format = header.m_Format;
    *out_entry     = entry;

    // copy the pixel data
    const auto start = (avtx_stream.m_Source ? source_buffer.begin() : buffer.begin()) + avtx_stream.m_Offset;
    std::copy(start, start + avtx_stream.m_Size, std::back_inserter(*out_buffer));
}

void ReadEntry(const std::vector<uint8_t>& buffer, const uint8_t stream_index, TextureEntry* out_entry,
               std::vector<uint8_t>* out_buffer, const std::vector<uint8_t>& source_buffer)
{
    if (buffer.empty()) {
        throw std::invalid_argument("AVTX input buffer can't be empty!");
    }

    if (stream_index >= AVTX_MAX_STREAMS) {
        throw std::invalid_argument("AVTX invalid stream index");
    }

    if (!out_entry) {
        throw std::invalid_argument("AVTX output entry can't be nullptr!");
    }

    if (!out_buffer) {
        throw std::invalid_argument("AVTX output buffer can't be nullptr!");
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    AvtxHeader header{};
    stream.read((char*)&header, sizeof(AvtxHeader));
    if (header.m_Magic != AVTX_MAGIC) {
        throw std::runtime_error("Invalid AVTX header magic!");
    }

    if (header.m_Version != 1) {
        throw std::runtime_error("Invalid AVTX version!");
    }

    // the requested stream was from source, and we provided no source buffer
    const AvtxStream& avtx_stream = header.m_Streams[stream_index];
    if (avtx_stream.m_Source && source_buffer.empty()) {
        throw std::runtime_error("AVTX source buffer required!");
    }

    const uint32_t rank = GetRank(header, stream_index);

    TextureEntry entry{};
    entry.m_Width  = (header.m_Width >> rank);
    entry.m_Height = (header.m_Height >> rank);
    entry.m_Depth  = (header.m_Depth >> rank);
    entry.m_Format = header.m_Format;
    *out_entry     = entry;

    // copy the pixel data
    const auto start = (avtx_stream.m_Source ? source_buffer.begin() : buffer.begin()) + avtx_stream.m_Offset;
    std::copy(start, start + avtx_stream.m_Size, std::back_inserter(*out_buffer));
}

uint8_t FindBestStream(const AvtxHeader& header, bool only_source)
{
    uint8_t biggest      = 0;
    uint8_t stream_index = 0;

    for (uint32_t i = 0; i < AVTX_MAX_STREAMS; ++i) {
        const AvtxStream& stream = header.m_Streams[i];
        if (stream.m_Size == 0) {
            continue;
        }

        if ((stream.m_Source == only_source) || (!stream.m_Source && stream.m_Size > biggest)) {
            biggest      = stream.m_Size;
            stream_index = i;
        }
    }

    return stream_index;
}

uint32_t GetRank(const AvtxHeader& header, uint8_t stream_index)
{
    return (header.m_Mips - (header.m_MipsRedisent + stream_index));
}
}; // namespace ava::AvalancheTexture

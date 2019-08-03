#include "../include/avalanche_texture.h"

#include "../include/util/byte_array_buffer.h"
#include "../include/util/hashlittle.h"

namespace ava::AvalancheTexture
{
/**
 * Convert a DDSC buffer to DDS
 *
 * @param buffer Input buffer containing a raw DDSC file buffer
 * @param out_buffer Pointer to char vector where the output DDS buffer will be written
 */
void Parse(const std::vector<uint8_t>& buffer, std::vector<uint8_t>* out_buffer)
{
    byte_array_buffer buf(buffer.data(), buffer.size());
    std::istream      stream(&buf);

    // read header
    AvtxFileHeader header;
    stream.read((char*)&header, sizeof(AvtxFileHeader));
    if (header.m_Magic != AVTX_MAGIC) {
        throw std::runtime_error("Invalid AVTX header magic!");
    }

    // find the best stream to use
    // @TODO: allow override
    const uint8_t  stream_index = FindBestStream(header);
    const uint32_t rank         = GetHighestRank(header, stream_index);

#ifdef _DEBUG
    __debugbreak();
#endif
}

uint8_t FindBestStream(const AvtxFileHeader& header, bool only_source)
{
    uint8_t biggest      = 0;
    uint8_t stream_index = 0;

    for (uint32_t i = 0; i < AVTX_MAX_STREAMS; ++i) {
        const AvtxFileStream& stream = header.m_Streams[i];
        if (stream.m_Size == 0) {
            continue;
        }

        if ((stream.m_IsSource == only_source) || (!stream.m_IsSource && stream.m_Size > biggest)) {
            biggest      = stream.m_Size;
            stream_index = i;
        }
    }

    return stream_index;
}

uint32_t GetHighestRank(const AvtxFileHeader& header, uint8_t stream_index)
{
    uint32_t rank = 0;
    for (uint32_t i = 0; i < AVTX_MAX_STREAMS; ++i) {
        if (i == stream_index) {
            continue;
        }

        if (header.m_Streams[i].m_Size > header.m_Streams[stream_index].m_Size) {
            ++rank;
        }
    }

    return rank;
}
}; // namespace ava::AvalancheTexture

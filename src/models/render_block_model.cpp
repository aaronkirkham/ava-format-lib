#include <models/render_block_model.h>

#include <algorithm>
#include <util/byte_array_buffer.h>

namespace ava::RenderBlockModel
{
std::vector<std::pair<uint64_t, uint64_t>> find_render_blocks(const std::vector<uint8_t>& buffer, uint64_t pos)
{
    std::vector<std::pair<uint64_t, uint64_t>> results;

    uint64_t start = pos;
    uint64_t _pos  = pos;
    while (_pos < buffer.size()) {
        if (buffer[_pos] == 0xEF && buffer[_pos + 1] == 0xCD && buffer[_pos + 2] == 0xAB && buffer[_pos + 3] == 0x89) {
            auto end = _pos;
            results.push_back(std::make_pair(start, end));
            start = (end + sizeof(uint32_t));
        }

        ++_pos;
    }

    return results;
}

Result Parse(const std::vector<uint8_t>& buffer, std::vector<RenderBlockData>* out_render_blocks)
{
    if (buffer.empty() || !out_render_blocks) {
        // throw std::invalid_argument("RBMDL input buffer can't be empty!");
        return E_INVALID_ARGUMENT;
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    RbmHeader header;
    stream.read((char*)&header, sizeof(RbmHeader));
    if (strncmp((char*)&header.m_Magic, "RBMDL", header.m_MagicLength) != 0) {
        // throw std::runtime_error("Invalid RBMDL header magic!");
        return E_RBMDL_INVALID_MAGIC;
    }

    // ensure correct version
    if (header.m_VersionMajor != 1 && header.m_VersionMinor != 16) {
        // throw std::runtime_error("Unknown RBMDL version! (Expecting 1.16.x)");
        return E_RBMDL_UNKNOWN_VERSION;
    }

    // copy out the render block data
    auto results = find_render_blocks(buffer, stream.tellg());
    out_render_blocks->reserve(results.size());
    for (auto& result : results) {
        uint32_t hash      = *(uint32_t*)(buffer.data() + result.first);
        uint64_t data_size = (uint64_t)((result.second - result.first) - sizeof(uint32_t));

        std::vector<uint8_t> render_block_buffer(data_size);
        std::memcpy(render_block_buffer.data(), (buffer.data() + result.first + sizeof(uint32_t)), data_size);

        out_render_blocks->emplace_back(std::make_pair(hash, std::move(render_block_buffer)));
    }

    return E_OK;
}
}; // namespace ava::RenderBlockModel

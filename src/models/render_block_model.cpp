#include <models/render_block_model.h>

#include <util/byte_array_buffer.h>

namespace ava::RenderBlockModel
{
Result Parse(const std::vector<uint8_t>& buffer, RBMHashHandler rbm_hash_handler)
{
    if (buffer.empty() || !rbm_hash_handler) {
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

    //
    if (header.m_VersionMajor != 1 && header.m_VersionMinor != 16) {
        // throw std::runtime_error("Unknown RBMDL version! (Expecting 1.16.x)");
        return E_RBMDL_UNKNOWN_VERSION;
    }

    // read render blocks
    for (decltype(header.m_NumberOfBlocks) i = 0; i < header.m_NumberOfBlocks; ++i) {
        uint32_t hash;
        stream.read((char*)&hash, sizeof(uint32_t));

        const uint32_t block_size = 0; // @TODO
        if (block_size != 0) {
            std::vector<uint8_t> block_buffer(block_size);
            stream.read((char*)block_buffer.data(), block_size);
            rbm_hash_handler(hash, block_buffer);
        }

        uint32_t checksum;
        stream.read((char*)&checksum, sizeof(uint32_t));
        if (checksum != RBM_BLOCK_CHECKSUM) {
            // throw std::runtime_error("Invalid RBMDL block checksum!");
            return E_RBMDL_BAD_CHECKSUM;
        }
    }

    return E_OK;
}
}; // namespace ava::RenderBlockModel

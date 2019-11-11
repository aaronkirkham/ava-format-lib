#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace ava::ResourceBundle
{
#pragma pack(push, 1)
struct ResourceEntry {
    uint32_t path_hash;
    uint32_t extension_hash;
    uint32_t file_size;
};
#pragma pack(pop)

static_assert(sizeof(ResourceEntry) == 0xC, "ResourceEntry alignment is wrong.");

/**
 * Read a single entry from a ResourceBundle file buffer
 *
 * @param buffer Input buffer containing a raw ResourceBundle file buffer
 * @param name_hash Filename hash of the entry to read
 * @param out_buffer Pointer to byte vector where the file buffer will be written
 */
void ReadEntry(const std::vector<uint8_t>& buffer, const uint32_t name_hash, std::vector<uint8_t>* out_buffer);

/**
 * Write a single entry to a ResourceBundle file buffer
 *
 * @param out_buffer Pointer to byte vector where the single entry will be written
 * @param filename Filename which is being written to the ResourceBundle file buffer
 * @param file_buffer Raw file buffer to write to the ResourceBundle file buffer
 */
void WriteEntry(std::vector<uint8_t>* out_buffer, const std::filesystem::path& filename,
                const std::vector<uint8_t>& file_buffer);
}; // namespace ava::ResourceBundle

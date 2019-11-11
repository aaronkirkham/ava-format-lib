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

void ReadEntry(const std::vector<uint8_t>& buffer, const uint32_t name_hash, std::vector<uint8_t>* out_buffer);
void WriteEntry(const std::filesystem::path& filename, const std::vector<uint8_t>& file_buffer,
                std::vector<uint8_t>* out_buffer);
}; // namespace ava::ResourceBundle

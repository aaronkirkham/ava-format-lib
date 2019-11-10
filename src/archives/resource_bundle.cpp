#include "../../include/archives/resource_bundle.h"

#include "../../include/util/byte_array_buffer.h"
#include "../../include/util/byte_vector_writer.h"
#include "../../include/util/hashlittle.h"

namespace ava::ResourceBundle
{
void ReadEntry(const std::vector<uint8_t>& buffer, const uint32_t name_hash, std::vector<uint8_t>* out_buffer)
{
    if (buffer.empty()) {
        throw std::invalid_argument("ResourceBundle input buffer can't be empty!");
    }

    if (!out_buffer) {
        throw std::invalid_argument("ResourceBundle output buffer can't be nullptr!");
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read all entries
    while (static_cast<int32_t>(stream.tellg()) + sizeof(ResourceEntry) <= buffer.size()) {
        ResourceEntry entry;
        stream.read((char*)&entry, sizeof(ResourceEntry));

#ifdef _DEBUG
        __debugbreak();
#endif
        if (entry.path_hash == name_hash) {
            // @TODO
        }
    }
}

void WriteEntry(const std::filesystem::path& filename, const std::vector<uint8_t>& file_buffer,
                std::vector<uint8_t>* out_buffer)
{
    if (filename.empty()) {
        throw std::invalid_argument("filename string can not be empty!");
    }

    if (file_buffer.empty()) {
        throw std::invalid_argument("input file buffer can not be empty!");
    }

    if (!out_buffer) {
        throw std::invalid_argument("output buffer can not be nullptr!");
    }

    byte_vector_writer buf(out_buffer);

    ResourceEntry entry{};
    entry.path_hash      = hashlittle(filename_str.stem().string().c_str());
    entry.extension_hash = hashlittle(filename.extension().string().c_str());
    entry.file_size      = file_buffer.size();

    buf.write((char*)&entry, sizeof(ResourceEntry));
    std::copy(file_buffer.begin(), file_buffer.end(), std::back_inserter(*out_buffer));
}
}; // namespace ava::ResourceBundle

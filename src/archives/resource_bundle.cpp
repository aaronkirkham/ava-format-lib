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

        // copy the file buffer
        if (entry.m_NameHash == name_hash) {
            out_buffer->resize(entry.m_Size);
            stream.read((char*)out_buffer->data(), entry.m_Size);
            break;
        }

        // skip the current file buffer and get to the next entry
        stream.ignore(entry.m_Size);
    }
}

void WriteEntry(std::vector<uint8_t>* out_buffer, const std::filesystem::path& filename,
                const std::vector<uint8_t>& file_buffer)
{
    if (!out_buffer) {
        throw std::invalid_argument("output buffer can not be nullptr!");
    }

    if (filename.empty()) {
        throw std::invalid_argument("filename string can not be empty!");
    }

    if (file_buffer.empty()) {
        throw std::invalid_argument("input file buffer can not be empty!");
    }

    byte_vector_writer buf(out_buffer);

    ResourceEntry entry{};
    entry.m_NameHash      = ava::hashlittle(filename.string().c_str());
    entry.m_ExtensionHash = ava::hashlittle(filename.extension().string().c_str());
    entry.m_Size          = static_cast<uint32_t>(file_buffer.size());

    buf.write((char*)&entry, sizeof(ResourceEntry));
    std::copy(file_buffer.begin(), file_buffer.end(), std::back_inserter(*out_buffer));
}
}; // namespace ava::ResourceBundle

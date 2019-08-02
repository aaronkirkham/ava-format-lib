#include "../../include/archives/stream_archive.h"

#include "../../include/util/byte_array_buffer.h"
#include "../../include/util/hashlittle.h"

#include <map>
#include <string>

namespace ava::StreamArchive
{
/**
 * Parse a SARC file and extract file entries
 *
 * @param buffer Input buffer containing a raw SARC file buffer
 * @param out_entries Pointer to vector of ArchiveEntry_t's where the entries will be written
 */
void Parse(const std::vector<uint8_t>& buffer, std::vector<ArchiveEntry_t>* out_entries)
{
    assert(buffer.size() != 0);
    assert(out_entries != nullptr);

    byte_array_buffer buf(buffer.data(), buffer.size());
    std::istream      stream(&buf);

    // read header
    SarcFileHeader header{};
    stream.read((char*)&header, sizeof(SarcFileHeader));
    if (header.m_Magic != SARC_MAGIC) {
        throw std::runtime_error("Invalid SARC header magic!");
    }

    // parse the version
    switch (header.m_Version) {
        // Just Cause 3 and earlier..
        case 2: {
            // read all entries
            const auto start_pos = stream.tellg();
            while (true) {
                uint32_t length = 0;
                stream.read((char*)&length, sizeof(uint32_t));

                auto filename = std::unique_ptr<char[]>(new char[length + 1]);
                stream.read(filename.get(), length);
                filename[length] = '\0';

                ArchiveEntry_t entry{};
                entry.m_Filename = filename.get();
                stream.read((char*)&entry.m_Offset, sizeof(entry.m_Offset));
                stream.read((char*)&entry.m_Size, sizeof(entry.m_Size));
                out_entries->emplace_back(std::move(entry));

                if (header.m_Size - (stream.tellg() - start_pos) <= 15) {
                    break;
                }
            }

            break;
        }

        // Just Cause 4 and later..
        case 3: {
            uint32_t strings_length = 0;
            stream.read((char*)&strings_length, sizeof(uint32_t));

            std::map<uint32_t, std::string> filenames;

            const uint64_t data_offset = ((uint64_t)stream.tellg() + strings_length);
            while ((uint64_t)stream.tellg() < data_offset) {
                std::string filename;
                std::getline(stream, filename, '\0');

                const uint32_t name_hash = hashlittle(filename.c_str());
                filenames[name_hash]     = std::move(filename);
            }

            assert(stream.tellg() == data_offset);

            // read all entries
            while (stream.tellg() < header.m_Size) {
                uint32_t file_offset       = 0;
                uint32_t uncompressed_size = 0;
                uint32_t name_hash         = 0;

                stream.ignore(sizeof(uint32_t)); // UNUSED: name offset
                stream.read((char*)&file_offset, sizeof(uint32_t));
                stream.read((char*)&uncompressed_size, sizeof(uint32_t));
                stream.read((char*)&name_hash, sizeof(uint32_t));
                stream.ignore(sizeof(uint32_t)); // UNUSED: extension hash

                ArchiveEntry_t entry{};
                entry.m_Filename = filenames[name_hash];
                entry.m_Offset   = file_offset;
                entry.m_Size     = uncompressed_size;
                out_entries->emplace_back(std::move(entry));
            }

            break;
        }

        default: {
            throw std::runtime_error("Unknown SARC version!");
        }
    }
}

/**
 * Parse a patched SARC file list, commonly used with the .TOC extension
 *
 * @param buffer Input buffer containing a raw TOC file buffer
 * @param out_entries Pointer to vector of ArchiveEntry_t's where the entries will be written
 */
void ParseTOC(const std::vector<uint8_t>& buffer, std::vector<ArchiveEntry_t>* out_entries)
{
    assert(buffer.size() != 0);
    assert(out_entries != nullptr);

    byte_array_buffer buf(buffer.data(), buffer.size());
    std::istream      stream(&buf);

    // read entries
    while (static_cast<size_t>(stream.tellg()) < buffer.size()) {
        uint32_t length;
        stream.read((char*)&length, sizeof(uint32_t));

        // read the filename string
        const auto filename = std::unique_ptr<char[]>(new char[length + 1]);
        stream.read(filename.get(), length);
        filename[length] = '\0';

        // read archive entry
        ArchiveEntry_t entry{};
        entry.m_Filename = filename.get();
        stream.read((char*)&entry.m_Offset, sizeof(entry.m_Offset));
        stream.read((char*)&entry.m_Size, sizeof(entry.m_Size));
        out_entries->emplace_back(std::move(entry));
    }
}
}; // namespace ava::StreamArchive

#include "../../include/archives/stream_archive.h"

#include "../../include/util/byte_array_buffer.h"
#include "../../include/util/byte_vector_writer.h"
#include "../../include/util/hashlittle.h"
#include "../../include/util/math.h"

#include <algorithm>
#include <map>
#include <numeric>
#include <string>

// TEMP
#include <fstream>

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
    if (buffer.empty()) {
        throw std::invalid_argument("SARC input buffer can't be empty!");
    }

    if (!out_entries) {
        throw std::invalid_argument("SARC output entries vector can't be nullptr!");
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    SarcHeader header{};
    stream.read((char*)&header, sizeof(SarcHeader));
    if (header.m_Magic != SARC_MAGIC) {
        throw std::runtime_error("Invalid SARC header magic!");
    }

    // parse the version
    switch (header.m_Version) {
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
    if (buffer.empty()) {
        throw std::invalid_argument("SARC TOC input buffer can't be empty!");
    }

    if (!out_entries) {
        throw std::invalid_argument("SARC TOC output entries vector can't be nullptr!");
    }

    byte_array_buffer buf(buffer);
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

/**
 * Read the buffer of an Archive Entry
 *
 * @param buffer Input buffer containing a raw SARC file buffer
 * @param entry Entry to read buffer of
 * @param out_buffer Pointer to char vector where the output entry buffer will be written
 */
void ReadEntry(const std::vector<uint8_t>& buffer, const ArchiveEntry_t& entry, std::vector<uint8_t>* out_buffer)
{
    if (buffer.empty()) {
        throw std::runtime_error("input buffer can't be empty!");
    }

    if (!out_buffer) {
        throw std::runtime_error("output buffer can't be nullptr!");
    }

    // @TODO: Fix reading entries which are patched (offset == 0).

    assert(entry.m_Offset != 0 && entry.m_Offset != -1);
    assert((entry.m_Offset + entry.m_Size) <= buffer.size());

    const auto start = buffer.begin() + entry.m_Offset;
    std::copy(start, start + entry.m_Size, std::back_inserter(*out_buffer));
}

/**
 * Read the buffer of an Archive Entry by filename
 *
 * @param buffer Input buffer containing a raw SARC file buffer
 * @param entries Vector of entries to read from, returned from Parse
 * @param filename String containing the name of the entry to read
 * @param out_buffer Pointer to char vector where the output entry buffer will be written
 */
void ReadEntry(const std::vector<uint8_t>& buffer, const std::vector<ArchiveEntry_t>& entries,
               const std::string& filename, std::vector<uint8_t>* out_buffer)
{
    const auto it = std::find_if(entries.begin(), entries.end(),
                                 [filename](const ArchiveEntry_t& entry) { return entry.m_Filename == filename; });
    if (it != entries.end()) {
        const ArchiveEntry_t& entry = (*it);

        assert(entry.m_Offset != 0 && entry.m_Offset != -1);
        assert((entry.m_Offset + entry.m_Size) <= buffer.size());

        const auto start = buffer.begin() + entry.m_Offset;
        std::copy(start, start + entry.m_Size, std::back_inserter(*out_buffer));
    }
}

/**
 * Write a file to a SARC buffer
 *
 * @param buffer Input buffer containing a raw SARC file buffer
 * @param entries Vector of current archive entries, returned from Parse
 * @param filename String containing the name of the entry to write
 * @param file_buffer Input buffer containing the raw data for the file to write to the SARC buffer
 */
void WriteEntry(std::vector<uint8_t>& buffer, std::vector<ArchiveEntry_t>* entries, const std::string& filename,
                const std::vector<uint8_t>& file_buffer)
{
    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    SarcHeader header{};
    stream.read((char*)&header, sizeof(SarcHeader));
    if (header.m_Magic != SARC_MAGIC) {
        throw std::runtime_error("Invalid SARC header magic!");
    }

    std::vector<uint8_t> temp_buffer;
    byte_vector_writer   tbuf(&temp_buffer);

    ArchiveEntry_t* entry = nullptr;
    const auto      it    = std::find_if(entries->begin(), entries->end(),
                                 [filename](const ArchiveEntry_t& item) { return item.m_Filename == filename; });

    // file currently does not exist in the archive, add a new entry
    if (it == entries->end()) {
        ArchiveEntry_t e{};
        e.m_Filename = filename;
        e.m_Offset   = 1; // @NOTE: offset will be updated later, just make sure this is >0
        e.m_Size     = static_cast<uint32_t>(file_buffer.size());
        entries->emplace_back(std::move(e));

        entry = &entries->back();
    }
    // file already exists in the archive, update the size
    else {
        entry         = &(*it);
        entry->m_Size = static_cast<uint32_t>(file_buffer.size());
    }

    switch (header.m_Version) {
        case 2: {
            // calculate new header and data size
            uint32_t header_size = 0;
            uint32_t data_size   = 0;
            for (const auto& entry : *entries) {
                const uint32_t length            = ava::math::aligned_string_len(entry.m_Filename);
                const uint32_t entry_header_size = sizeof(uint32_t) + length + sizeof(uint32_t) + sizeof(uint32_t);
                const uint32_t entry_data_size   = entry.m_Offset != 0 ? entry.m_Size : 0;

                header_size += entry_header_size;
                data_size += ava::math::align(entry_data_size);
            }

            header.m_Size = ava::math::align(header_size, 16);

            // allocate enough space for everything
            temp_buffer.resize(sizeof(SarcHeader) + header.m_Size + data_size);
            tbuf.write((char*)&header, sizeof(SarcHeader));

            // update all entries
            uint32_t current_data_offset = (sizeof(SarcHeader) + header.m_Size);
            for (auto& entry : *entries) {
                uint32_t       padding = 0;
                const uint32_t length  = ava::math::aligned_string_len(entry.m_Filename, sizeof(uint32_t), &padding);
                const uint32_t data_offset = entry.m_Offset != 0 ? current_data_offset : 0;

                tbuf.write((char*)&length, sizeof(uint32_t));
                tbuf.write((char*)entry.m_Filename.c_str(), entry.m_Filename.length());
                tbuf.write((char*)&SARC_ENTRY_PADDING_BYTE, sizeof(uint8_t), padding);
                tbuf.write((char*)&data_offset, sizeof(uint32_t));
                tbuf.write((char*)&entry.m_Size, sizeof(uint32_t));

                // don't update offsets of patched files
                if (data_offset != 0) {
                    // if the current entry is the file we added, copy the buffer from the input file buffer
                    if (entry.m_Filename == filename) {
                        std::memcpy(&temp_buffer[data_offset], file_buffer.data(), file_buffer.size());
                    } else {
                        std::memcpy(&temp_buffer[data_offset], &buffer[entry.m_Offset], entry.m_Size);
                    }

                    // update the entry data offset
                    entry.m_Offset = data_offset;
                }

                // update the current data offset
                current_data_offset = ava::math::align(current_data_offset + (data_offset != 0 ? entry.m_Size : 0));
            }

            buffer = std::move(temp_buffer);
            break;
        }

        case 3: {
            // calculate strings length
            const uint32_t strings_length = std::accumulate(
                entries->begin(), entries->end(), 0, [](uint32_t accumulator, const ArchiveEntry_t& entry) {
                    return accumulator + (uint32_t)entry.m_Filename.length() + 1;
                });

            tbuf.write((char*)&strings_length, sizeof(uint32_t));

            // write strings
            for (const auto& entry : *entries) {
                tbuf.write((char*)entry.m_Filename.c_str(), entry.m_Filename.length() + 1);
            }

            // @TODO: finish me!

#ifdef _DEBUG
            std::ofstream s("c:/users/aaron/desktop/tmp.bin", std::ios::binary);
            s.write((char*)temp_buffer.data(), temp_buffer.size());
            s.close();

            __debugbreak();
#endif
            break;
        }
    }
}
}; // namespace ava::StreamArchive

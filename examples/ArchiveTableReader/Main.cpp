#include <AvaFormatLib.h>

#include <filesystem>
#include <fstream>

void ReadFileToBuffer(const std::filesystem::path& filename, std::vector<uint8_t>* buffer)
{
    const auto size = std::filesystem::file_size(filename);
    buffer->resize(size);

    std::ifstream stream(filename, std::ios::binary);
    stream.read((char*)buffer->data(), size);
    stream.close();
}

int main(int argc, char* argv[])
{
    // TAB test
    {
        std::vector<uint8_t> tab_buffer;
        ReadFileToBuffer("D:/Steam/steamapps/common/Just Cause 4/archives_win64/main_patch/game0.tab", &tab_buffer);

        std::vector<ava::ArchiveTable::TabFileEntry> entries;
        ava::ArchiveTable::ReadTab(tab_buffer, &entries);

        __debugbreak();
    }

    // SARC test
    {
        std::vector<uint8_t> aaf_buffer;
        ReadFileToBuffer("C:/users/aaron/desktop/grapplinghookwire.ee", &aaf_buffer);

        std::vector<uint8_t>                            sarc_buffer;
        std::vector<ava::StreamArchive::ArchiveEntry_t> entries;

        ava::AvalancheArchiveFormat::Parse(aaf_buffer, &sarc_buffer);
        ava::StreamArchive::Parse(sarc_buffer, &entries);

        __debugbreak();
    }

    // TOC test
    {
        std::vector<uint8_t> buffer;
        ReadFileToBuffer("C:/users/aaron/desktop/grapplinghookwire.ee.toc", &buffer);

        std::vector<ava::StreamArchive::ArchiveEntry_t> entries;
        ava::StreamArchive::ParseTOC(buffer, &entries);

        __debugbreak();
    }
    return 1;
}

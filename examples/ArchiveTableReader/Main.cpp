#include <archive_table.h>

void main()
{
    std::filesystem::path filename = "D:/Steam/steamapps/common/Just Cause 4/archives_win64/main_patch/game0.tab";
    // ava::ArchiveTable::ReadBufferFromArchive(filename, name_hash, &buffer);

    std::vector<ava::ArchiveTable::TabFileEntry> entries;
    ava::ArchiveTable::ReadTab(filename, &entries);

    __debugbreak();
}

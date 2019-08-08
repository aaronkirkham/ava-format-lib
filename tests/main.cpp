#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <AvaFormatLib.h>

#include <assert.h>
#include <filesystem>
#include <fstream>

using FileBuffer = std::vector<uint8_t>;
void ReadFile(const std::filesystem::path& filename, FileBuffer* buffer)
{
    const auto size = std::filesystem::file_size(filename);
    buffer->resize(size);

    std::ifstream stream(filename, std::ios::binary);
    stream.read((char*)buffer->data(), size);
    stream.close();
}

TEST_CASE("Archive TAB Test", "[AvaFormatLib]")
{
    FileBuffer                                   buffer;
    std::vector<ava::ArchiveTable::TabFileEntry> entries;

    ReadFile("D:/Steam/steamapps/common/Just Cause 4/archives_win64/main_patch/game0.tab", &buffer);
    REQUIRE_FALSE(buffer.empty());

    // empty input buffer should throw
    REQUIRE_THROWS_AS(ava::ArchiveTable::ReadTab({}, nullptr), std::invalid_argument);

    // should have entries
    REQUIRE_NOTHROW(ava::ArchiveTable::ReadTab(buffer, &entries));
    REQUIRE_FALSE(entries.empty());
}

TEST_CASE("Archive AAF Test", "[AvaFormatLib]")
{
    FileBuffer buffer;
    FileBuffer out_buffer;

    ReadFile("C:/users/aaron/desktop/grapplinghookwire.ee", &buffer);
    REQUIRE_FALSE(buffer.empty());

    // empty input buffer should throw
    REQUIRE_THROWS_AS(ava::AvalancheArchiveFormat::Parse({}, nullptr), std::invalid_argument);

    // should have output buffer
    REQUIRE_NOTHROW(ava::AvalancheArchiveFormat::Parse(buffer, &out_buffer));
    REQUIRE_FALSE(out_buffer.empty());
}

TEST_CASE("Archive SARC TOC Test", "[AvaFormatLib]")
{
    FileBuffer                                      buffer;
    std::vector<ava::StreamArchive::ArchiveEntry_t> entries;

    ReadFile("C:/users/aaron/desktop/grapplinghookwire.ee.toc", &buffer);
    REQUIRE_FALSE(buffer.empty());

    // empty input buffer should throw
    REQUIRE_THROWS_AS(ava::StreamArchive::ParseTOC(buffer, nullptr), std::invalid_argument);

    // should have entries
    REQUIRE_NOTHROW(ava::StreamArchive::ParseTOC(buffer, &entries));
    REQUIRE_FALSE(entries.empty());
}

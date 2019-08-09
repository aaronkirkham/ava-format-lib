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

struct AISpring {
    float Speed;
    float Constant;
    float Damping;
};

struct SniperTweaks {
    AISpring AimSpringXZ;
    AISpring AimSpringY;
    AISpring VelocityPredictionXZ;
    float    PerfectAimTimeBeforeShooting;
    float    InitialPredictAheadDistance;
    float    FinalPredictAheadDistance;
    float    PredictFadeOutTime;
    float    ShootIfHoveringForThisLong;
    float    MinTimeBeforeShooting;
    float    InitialRandomAimDistance;
};

struct MountedWeaponTweaks {
    int TimeBetweenCheckingTheSameWeaponTwice;
};

struct WeaponTweaks {
    SniperTweaks        Sniper;
    MountedWeaponTweaks MountedWeapon;
};

TEST_CASE("ADF Test", "[AvaFormatLib]")
{
    FileBuffer buffer;
    ReadFile("C:/users/aaron/desktop/adf/weapons.aisystunec", &buffer);
    REQUIRE_FALSE(buffer.empty());

    ava::AvalancheDataFormat::AvalancheDataFormat adf(buffer);

    // root instance
    ava::AvalancheDataFormat::SInstanceInfo instance_info;
    REQUIRE_NOTHROW(adf.GetInstance(0, &instance_info));
    REQUIRE(instance_info.m_NameHash == 0xd9066df1);

    // read instance
    WeaponTweaks* weapon_tweaks = nullptr;
    REQUIRE_NOTHROW(adf.ReadInstance(instance_info, (void**)&weapon_tweaks));
    REQUIRE(weapon_tweaks != nullptr);
    REQUIRE(weapon_tweaks->Sniper.InitialRandomAimDistance == 1.5f);
}

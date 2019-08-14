#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <AvaFormatLib.h>

#include <filesystem>
#include <fstream>

using FileBuffer = std::vector<uint8_t>;
bool ReadFile(const std::filesystem::path& filename, FileBuffer* buffer)
{
    try {
        const auto size = std::filesystem::file_size(filename);
        buffer->resize(size);

        std::ifstream stream(filename, std::ios::binary);
        stream.read((char*)buffer->data(), size);
        stream.close();
    } catch (...) {
        return false;
    }

    return !buffer->empty();
}

TEST_CASE("Archive Table Format", "[AvaFormatLib][TAB]")
{
    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/game5.tab", &buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        std::vector<ava::ArchiveTable::TabEntry> entries;
        REQUIRE_THROWS_AS(ava::ArchiveTable::ReadTab({}, &entries), std::invalid_argument);
        REQUIRE_THROWS_AS(ava::ArchiveTable::ReadTab(buffer, nullptr), std::invalid_argument);
    }

    SECTION("file was parsed and entries vector has results")
    {
        std::vector<ava::ArchiveTable::TabEntry> entries;
        REQUIRE_NOTHROW(ava::ArchiveTable::ReadTab(buffer, &entries));
        REQUIRE_FALSE(entries.empty());
        REQUIRE(entries[0].m_NameHash == 0xbeea6bb0);
    }
}

TEST_CASE("Avalanche Archive Format", "[AvaFormatLib][AAF]")
{
    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/grapplinghookwire.ee", &buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        FileBuffer out_buffer;
        REQUIRE_THROWS_AS(ava::AvalancheArchiveFormat::Parse({}, &out_buffer), std::invalid_argument);
        REQUIRE_THROWS_AS(ava::AvalancheArchiveFormat::Parse(buffer, nullptr), std::invalid_argument);
    }

    /*SECTION("has valid output buffer")
    {
        FileBuffer out_buffer;
        REQUIRE_NOTHROW(ava::AvalancheArchiveFormat::Parse(buffer, &out_buffer));
        REQUIRE_FALSE(out_buffer.empty());
    }*/
}

TEST_CASE("Stream Archive", "[AvaFormatLib][SARC]")
{
    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/paratrooper_drop.ee", &buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        std::vector<ava::StreamArchive::ArchiveEntry_t> entries;
        REQUIRE_THROWS_AS(ava::StreamArchive::Parse({}, &entries), std::invalid_argument);
        REQUIRE_THROWS_AS(ava::StreamArchive::Parse(buffer, nullptr), std::invalid_argument);
    }

    SECTION("file was parsed and entries vector has results")
    {
        std::vector<ava::StreamArchive::ArchiveEntry_t> entries;
        REQUIRE_NOTHROW(ava::StreamArchive::Parse(buffer, &entries));
        REQUIRE(entries.size() == 3);
        REQUIRE(entries[2].m_Filename
                == "editor/entities/spawners/combatant_spawnrules/spawn_modules/paratrooper_drop.epe");
    }
}

TEST_CASE("Stream Archive TOC", "[AvaFormatLib][TOC]")
{
    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/grapplinghookwire.ee.toc", &buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        std::vector<ava::StreamArchive::ArchiveEntry_t> entries;
        REQUIRE_THROWS_AS(ava::StreamArchive::ParseTOC({}, &entries), std::invalid_argument);
        REQUIRE_THROWS_AS(ava::StreamArchive::ParseTOC(buffer, nullptr), std::invalid_argument);
    }

    SECTION("file was parsed and entries vector has results")
    {
        std::vector<ava::StreamArchive::ArchiveEntry_t> entries;
        REQUIRE_NOTHROW(ava::StreamArchive::ParseTOC(buffer, &entries));
        REQUIRE_FALSE(entries.empty());
        REQUIRE(entries[9].m_Filename == "effects/textures/t_smoke_blast_alpha_dif.ddsc");
    }
}

TEST_CASE("Avalanche Data Format", "[AvaFormatLib][ADF]")
{
    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/weapons.aisystunec", &buffer));

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

    SECTION("invalid input argument throws std::invalid_argument")
    {
        REQUIRE_THROWS_AS(
            []() {
                // invalid input buffer
                ava::AvalancheDataFormat::AvalancheDataFormat adf({});
            }(),
            std::invalid_argument);
    }

    SECTION("can get root instance")
    {
        ava::AvalancheDataFormat::AvalancheDataFormat adf(buffer);

        ava::AvalancheDataFormat::SInstanceInfo instance_info{};
        REQUIRE_NOTHROW(adf.GetInstance(0, &instance_info));
        REQUIRE(instance_info.m_NameHash == 0xd9066df1);
    }

    SECTION("can read root instance")
    {
        ava::AvalancheDataFormat::AvalancheDataFormat adf(buffer);

        // read instance
        WeaponTweaks* weapon_tweaks = nullptr;
        REQUIRE_NOTHROW(adf.ReadInstance(0xd9066df1, 0x8dfb5000, (void**)&weapon_tweaks));
        REQUIRE(weapon_tweaks != nullptr);
        REQUIRE(weapon_tweaks->Sniper.InitialRandomAimDistance == 1.5f);
    }
}

TEST_CASE("Render Block Model", "[AvaFormatLib][RBMDL]")
{
    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/model.rbm", &buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        static auto hash_handler = [](uint32_t hash, const std::vector<uint8_t>& buffer) {};
        REQUIRE_THROWS_AS(ava::RenderBlockModel::Parse({}, hash_handler), std::invalid_argument);
        REQUIRE_THROWS_AS(ava::RenderBlockModel::Parse(buffer, nullptr), std::invalid_argument);
    }

    SECTION("file was parsed")
    {
        static auto hash_handler = [](uint32_t hash, const std::vector<uint8_t>& buffer) { REQUIRE_FALSE(true); };
        REQUIRE_NOTHROW(ava::RenderBlockModel::Parse(buffer, hash_handler));
    }
}

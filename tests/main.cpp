#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <AvaFormatLib.h>

#include "archives/legacy/archive_table.h"

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
    using namespace ava::ArchiveTable;

    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/game5.tab", &buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        std::vector<TabEntry> entries;
        REQUIRE_THROWS_AS(ReadTab({}, &entries), std::invalid_argument);
        REQUIRE_THROWS_AS(ReadTab(buffer, nullptr), std::invalid_argument);
    }

    SECTION("file was parsed and entries vector has results")
    {
        std::vector<TabEntry> entries;
        REQUIRE_NOTHROW(ReadTab(buffer, &entries));
        REQUIRE_FALSE(entries.empty());
        REQUIRE(entries[0].m_NameHash == 0xbeea6bb0);
    }
}

TEST_CASE("Archive Table Format (LEGACY)", "[AvaFormatLib][TAB]")
{
    using namespace ava::legacy::ArchiveTable;

    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/game67.tab", &buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        std::vector<TabEntry> entries;
        REQUIRE_THROWS_AS(ReadTab({}, &entries), std::invalid_argument);
        REQUIRE_THROWS_AS(ReadTab(buffer, nullptr), std::invalid_argument);
    }

    SECTION("file was parsed and entries vector has results")
    {
        std::vector<TabEntry> entries;
        REQUIRE_NOTHROW(ReadTab(buffer, &entries));
        REQUIRE_FALSE(entries.empty());
        REQUIRE(entries[0].m_NameHash == 0x966db7bc);
    }
}

TEST_CASE("Avalanche Archive Format", "[AvaFormatLib][AAF]")
{
    using namespace ava::AvalancheArchiveFormat;

    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/grapplinghookwire.ee", &buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        FileBuffer out_buffer;
        REQUIRE_THROWS_AS(Parse({}, &out_buffer), std::invalid_argument);
        REQUIRE_THROWS_AS(Parse(buffer, nullptr), std::invalid_argument);
    }

    SECTION("has valid output buffer")
    {
        FileBuffer out_buffer;
        REQUIRE_NOTHROW(Parse(buffer, &out_buffer));
        REQUIRE_FALSE(out_buffer.empty());
    }
}

TEST_CASE("Stream Archive", "[AvaFormatLib][SARC]")
{
    using namespace ava::StreamArchive;

    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/paratrooper_drop.ee", &buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        std::vector<ArchiveEntry_t> entries;
        REQUIRE_THROWS_AS(Parse({}, &entries), std::invalid_argument);
        REQUIRE_THROWS_AS(Parse(buffer, nullptr), std::invalid_argument);
    }

    SECTION("file was parsed and entries vector has results")
    {
        std::vector<ArchiveEntry_t> entries;
        REQUIRE_NOTHROW(Parse(buffer, &entries));
        REQUIRE(entries.size() == 3);
        REQUIRE(entries[2].m_Filename
                == "editor/entities/spawners/combatant_spawnrules/spawn_modules/paratrooper_drop.epe");
    }
}

TEST_CASE("Stream Archive TOC", "[AvaFormatLib][TOC]")
{
    using namespace ava::StreamArchive;

    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/grapplinghookwire.ee.toc", &buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        std::vector<ArchiveEntry_t> entries;
        REQUIRE_THROWS_AS(ParseTOC({}, &entries), std::invalid_argument);
        REQUIRE_THROWS_AS(ParseTOC(buffer, nullptr), std::invalid_argument);
    }

    SECTION("file was parsed and entries vector has results")
    {
        std::vector<ArchiveEntry_t> entries;
        REQUIRE_NOTHROW(ParseTOC(buffer, &entries));
        REQUIRE_FALSE(entries.empty());
        REQUIRE(entries[9].m_Filename == "effects/textures/t_smoke_blast_alpha_dif.ddsc");
    }
}

TEST_CASE("Runtime Property Container", "[AvaFormatLib][RTPC]")
{
    using namespace ava::RuntimePropertyContainer;

    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/random_encounter_bombs_away.epe", &buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        REQUIRE_THROWS_AS(
            []() {
                // invalid input buffer
                RuntimeContainer rtpc({});
            }(),
            std::invalid_argument);
    }

    SECTION("123")
    {
        RuntimeContainer rtpc(buffer);
    }
}

TEST_CASE("Avalanche Data Format", "[AvaFormatLib][ADF]")
{
    using namespace ava::AvalancheDataFormat;

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
                ADF adf({});
            }(),
            std::invalid_argument);
    }

    SECTION("can get root instance")
    {
        ADF           adf(buffer);
        SInstanceInfo instance_info{};

        REQUIRE_NOTHROW(adf.GetInstance(0, &instance_info));
        REQUIRE(instance_info.m_NameHash == 0xd9066df1);
    }

    SECTION("can read root instance")
    {
        ADF adf(buffer);

        // read instance
        WeaponTweaks* weapon_tweaks = nullptr;
        REQUIRE_NOTHROW(adf.ReadInstance(0xd9066df1, 0x8dfb5000, (void**)&weapon_tweaks));
        REQUIRE(weapon_tweaks != nullptr);
        REQUIRE(weapon_tweaks->Sniper.InitialRandomAimDistance == 1.5f);

        std::free(weapon_tweaks);
    }
}

#if 0
TEST_CASE("Render Block Model", "[AvaFormatLib][RBMDL]")
{
    using namespace ava::RenderBlockModel;

    FileBuffer buffer;
    REQUIRE(ReadFile("../tests/data/model.rbm", &buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        static auto hash_handler = [](uint32_t hash, const std::vector<uint8_t>& buffer) {};
        REQUIRE_THROWS_AS(Parse({}, hash_handler), std::invalid_argument);
        REQUIRE_THROWS_AS(Parse(buffer, nullptr), std::invalid_argument);
    }

    SECTION("file was parsed")
    {
        static auto hash_handler = [](uint32_t hash, const std::vector<uint8_t>& buffer) { REQUIRE_FALSE(true); };
        REQUIRE_NOTHROW(Parse(buffer, hash_handler));
    }
}
#endif

TEST_CASE("Avalanche Model Format", "[AvaFormatLib][AMF]")
{
    using namespace ava::AvalancheDataFormat;
    using namespace ava::AvalancheModelFormat;

    FileBuffer modelc_buffer;
    REQUIRE(ReadFile("../tests/data/cow.modelc", &modelc_buffer));

    FileBuffer meshc_buffer;
    REQUIRE(ReadFile("../tests/data/cow.meshc", &meshc_buffer));

    SECTION("invalid input argument throws std::invalid_argument")
    {
        ADF*             adf         = nullptr;
        SAmfModel*       amf_model   = nullptr;
        SAmfMeshHeader*  mesh_header = nullptr;
        SAmfMeshBuffers* mesh_buffer = nullptr;
        REQUIRE_THROWS_AS(ParseModelc({}, &adf, &amf_model), std::invalid_argument);
        REQUIRE_THROWS_AS(ParseMeshc({}, &adf, &mesh_header, &mesh_buffer), std::invalid_argument);
    }

    SECTION("MODELC file was parsed")
    {
        ADF*       adf       = nullptr;
        SAmfModel* amf_model = nullptr;
        REQUIRE_NOTHROW(ParseModelc(modelc_buffer, &adf, &amf_model));

        REQUIRE(adf != nullptr);
        REQUIRE(amf_model != nullptr);

        REQUIRE(amf_model->m_Mesh == 0xc316a3df);

        std::free(adf);
        std::free(amf_model);
    }

    SECTION("MESHC file was parsed")
    {
        ADF*             adf         = nullptr;
        SAmfMeshHeader*  mesh_header = nullptr;
        SAmfMeshBuffers* mesh_buffer = nullptr;
        REQUIRE_NOTHROW(ParseMeshc(meshc_buffer, &adf, &mesh_header, &mesh_buffer));

        REQUIRE(adf != nullptr);
        REQUIRE(mesh_header != nullptr);
        REQUIRE(mesh_buffer != nullptr);

        REQUIRE(mesh_header->m_LodGroups.m_Count == 5);
        REQUIRE(mesh_header->m_HighLodPath == 0x778851d8);
        REQUIRE(mesh_buffer->m_VertexBuffers.m_Count == 1);

        std::free(adf);
        std::free(mesh_header);
        std::free(mesh_buffer);
    }
}

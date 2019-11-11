#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <AvaFormatLib.h>

#include "archives/legacy/archive_table.h"

#include <filesystem>
#include <fstream>

using FileBuffer = std::vector<uint8_t>;
bool ReadTestFile(const std::filesystem::path& filename, FileBuffer* buffer)
{
    std::filesystem::path name = (IsDebuggerPresent() ? "../" : "");
    name                       = name / "tests" / "data" / filename;

    try {
        const auto size = std::filesystem::file_size(name);
        buffer->resize(size);

        std::ifstream stream(name, std::ios::binary);
        stream.read((char*)buffer->data(), size);
        stream.close();
    } catch (...) {
        return false;
    }

    return !buffer->empty();
}

template <typename T> std::size_t VectorFastHash(std::vector<T> const& vec)
{
    std::size_t seed = vec.size();
    for (auto& i : vec) {
        seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

bool FilesAreTheSame(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b)
{
    return VectorFastHash(a) == VectorFastHash(b);
}

TEST_CASE("Archive Table Format", "[AvaFormatLib][TAB]")
{
    using namespace ava::ArchiveTable;

    FileBuffer tab_buffer, arc_buffer;
    ReadTestFile("test0.tab", &tab_buffer);
    ReadTestFile("test0.arc", &arc_buffer);

    FileBuffer hello_buffer, world_buffer;
    ReadTestFile("hello.bin", &hello_buffer);
    ReadTestFile("world.bin", &world_buffer);

    ava::Oodle::LoadLib("D:/Steam/steamapps/common/Just Cause 4/oo2core_7_win64.dll");

    SECTION("invalid input argument throws std::invalid_argument")
    {
        std::vector<TabEntry> entries;
        REQUIRE_THROWS_AS(Parse({}, &entries), std::invalid_argument);
        REQUIRE_THROWS_AS(Parse(tab_buffer, nullptr), std::invalid_argument);
    }

    SECTION("file was parsed and entries vector has results")
    {
        std::vector<TabEntry> entries;
        REQUIRE_NOTHROW(Parse(tab_buffer, &entries));
        REQUIRE_FALSE(entries.empty());
        REQUIRE(entries[0].m_NameHash == hashlittle("hello.bin"));
    }

    SECTION("can read entries")
    {
        TabEntry entry{};
        REQUIRE_NOTHROW(ReadEntry(tab_buffer, hashlittle("world.bin"), &entry));

        std::vector<uint8_t> file_buffer;
        REQUIRE_NOTHROW(ReadEntryBuffer(arc_buffer, entry, nullptr, &file_buffer));
        REQUIRE(FilesAreTheSame(file_buffer, world_buffer));
    }

    SECTION("can write entries")
    {
        FileBuffer t_buffer, a_buffer;
        REQUIRE_NOTHROW(WriteEntry("hello.bin", hello_buffer, &t_buffer, &a_buffer));
        REQUIRE_NOTHROW(WriteEntry("world.bin", world_buffer, &t_buffer, &a_buffer, E_COMPRESS_LIBRARY_OODLE));
        REQUIRE(FilesAreTheSame(t_buffer, tab_buffer));
        REQUIRE(FilesAreTheSame(a_buffer, arc_buffer));
    }

    ava::Oodle::UnloadLib();
}

TEST_CASE("Archive Table Format (LEGACY)", "[AvaFormatLib][TAB]")
{
    using namespace ava::legacy::ArchiveTable;

    FileBuffer tab_buffer, arc_buffer;
    ReadTestFile("test0_legacy.tab", &tab_buffer);
    ReadTestFile("test0_legacy.arc", &arc_buffer);

    FileBuffer hello_buffer, world_buffer;
    ReadTestFile("hello.bin", &hello_buffer);
    ReadTestFile("world.bin", &world_buffer);

    SECTION("invalid input argument throws std::invalid_argument")
    {
        std::vector<TabEntry> entries;
        REQUIRE_THROWS_AS(Parse({}, &entries), std::invalid_argument);
        REQUIRE_THROWS_AS(Parse(tab_buffer, nullptr), std::invalid_argument);
    }

    SECTION("file was parsed and entries vector has results")
    {
        std::vector<TabEntry> entries;
        REQUIRE_NOTHROW(Parse(tab_buffer, &entries));
        REQUIRE_FALSE(entries.empty());
        REQUIRE(entries[0].m_NameHash == hashlittle("hello.bin"));
    }

    SECTION("can read entries")
    {
        TabEntry entry{};
        REQUIRE_NOTHROW(ReadEntry(tab_buffer, hashlittle("world.bin"), &entry));

        std::vector<uint8_t> file_buffer;
        REQUIRE_NOTHROW(ReadEntryBuffer(arc_buffer, entry, &file_buffer));
        REQUIRE(FilesAreTheSame(file_buffer, world_buffer));
    }

    SECTION("can write entries")
    {
        FileBuffer t_buffer, a_buffer;
        REQUIRE_NOTHROW(WriteEntry("hello.bin", hello_buffer, &t_buffer, &a_buffer));
        REQUIRE_NOTHROW(WriteEntry("world.bin", world_buffer, &t_buffer, &a_buffer));
        REQUIRE(FilesAreTheSame(t_buffer, tab_buffer));
        REQUIRE(FilesAreTheSame(a_buffer, arc_buffer));
    }
}

TEST_CASE("Avalanche Archive Format", "[AvaFormatLib][AAF]")
{
    using namespace ava::AvalancheArchiveFormat;

    FileBuffer buffer;
    ReadTestFile("grapplinghookwire.ee", &buffer);

    SECTION("invalid input argument throws std::invalid_argument")
    {
        FileBuffer out_buffer;
        REQUIRE_THROWS_AS(Decompress({}, &out_buffer), std::invalid_argument);
        REQUIRE_THROWS_AS(Decompress(buffer, nullptr), std::invalid_argument);
        REQUIRE_THROWS_AS(Compress({}, &out_buffer), std::invalid_argument);
        REQUIRE_THROWS_AS(Compress(buffer, nullptr), std::invalid_argument);
    }

    SECTION("decompression has valid output buffer")
    {
        FileBuffer decompressed_buffer;
        REQUIRE_NOTHROW(Decompress(buffer, &decompressed_buffer));
        REQUIRE_FALSE(decompressed_buffer.empty());

        SECTION("decompressed output buffer is valid SARC format")
        {
            std::vector<ava::StreamArchive::ArchiveEntry_t> entries;
            REQUIRE_NOTHROW(ava::StreamArchive::Parse(decompressed_buffer, &entries));
            REQUIRE(entries.size() == 81);
            REQUIRE(entries[80].m_Filename == "editor/entities/gameobjects/grapplinghookwire.epe");
        }

        SECTION("compressed buffer matches original file buffer")
        {
            FileBuffer recompressed_buffer;
            REQUIRE_NOTHROW(Compress(decompressed_buffer, &recompressed_buffer));
            REQUIRE(recompressed_buffer.size() == buffer.size());
            REQUIRE(FilesAreTheSame(recompressed_buffer, buffer));
        }
    }
}

TEST_CASE("Stream Archive", "[AvaFormatLib][SARC]")
{
    using namespace ava::StreamArchive;

    SECTION("invalid input argument throws std::invalid_argument")
    {
        std::vector<ArchiveEntry_t> entries;
        REQUIRE_THROWS_AS(Parse({}, &entries), std::invalid_argument);
        REQUIRE_THROWS_AS(Parse({1}, nullptr), std::invalid_argument);
    }

    SECTION("v2")
    {
        FileBuffer buffer;
        ReadTestFile("wgst002_skin_flame.sarc", &buffer);

        std::vector<ArchiveEntry_t> entries;
        REQUIRE_NOTHROW(Parse(buffer, &entries));

        SECTION("file was parsed and entries vector has results")
        {
            REQUIRE_FALSE(entries.empty());
            REQUIRE(entries.at(10).m_Filename == "models/jc_aero/wingsuit/wingsuit_dlc_body.lod");
        }

        SECTION("can read entries")
        {
            std::vector<uint8_t> out_buffer;
            ReadEntry(buffer, entries, "editor/entities/dlc/wingsuit_skins/wgst002_skin_flame.epe", &out_buffer);
            REQUIRE_FALSE(out_buffer.empty());
            REQUIRE((out_buffer[0] == 'R' && out_buffer[1] == 'T' && out_buffer[2] == 'P' && out_buffer[3] == 'C'));
        }

        SECTION("can write entries")
        {
            FileBuffer world_buffer;
            ReadTestFile("world.bin", &world_buffer);

            REQUIRE_NOTHROW(WriteEntry(&buffer, &entries, "world.bin", world_buffer));
            REQUIRE(entries.back().m_Filename == "world.bin");
        }
    }

    SECTION("v3")
    {
        FileBuffer buffer;
        ReadTestFile("paratrooper_drop.ee", &buffer);

        std::vector<ArchiveEntry_t> entries;
        REQUIRE_NOTHROW(Parse(buffer, &entries));

        SECTION("file was parsed and entries vector has results")
        {
            REQUIRE_FALSE(entries.empty());
            REQUIRE(entries.at(2).m_Filename
                    == "editor/entities/spawners/combatant_spawnrules/spawn_modules/paratrooper_drop.epe");
        }

        SECTION("can read entries")
        {
            std::vector<uint8_t> out_buffer;
            /*ReadEntry(buffer, entries,
                      "editor/entities/spawners/combatant_spawnrules/spawn_modules/paratrooper_drop.epe",
               &out_buffer);*/
            ReadEntry(buffer, entries, "ai/bts/01_micro_behaviors/vehicle/airplane_fly_on_spline.btc", &out_buffer);
            REQUIRE_FALSE(out_buffer.empty());
        }

#if 0
		SECTION("can write entries")
        {
            FileBuffer world_buffer;
            ReadTestFile("world.bin", &world_buffer);

            REQUIRE_NOTHROW(WriteEntry(buffer, &entries, "world.bin", world_buffer));
            REQUIRE(entries.back().m_Filename == "world.bin");
        }
#endif
    }
}

TEST_CASE("Stream Archive TOC", "[AvaFormatLib][TOC]")
{
    using namespace ava::StreamArchive;

    FileBuffer buffer;
    ReadTestFile("grapplinghookwire.ee.toc", &buffer);

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

TEST_CASE("Resource Bundle", "[AvaFormatLib][ResourceBundle]")
{
    using namespace ava::ResourceBundle;

    FileBuffer buffer;
    ReadTestFile("test1.resourcebundle", &buffer);

    FileBuffer hello_buffer, world_buffer;
    ReadTestFile("hello.bin", &hello_buffer);
    ReadTestFile("world.bin", &world_buffer);

    SECTION("invalid input argument throws std::invalid_argument")
    {
        std::vector<uint8_t> out_buffer;
        REQUIRE_THROWS_AS(ReadEntry({}, 0x0, &out_buffer), std::invalid_argument);
        REQUIRE_THROWS_AS(ReadEntry(buffer, 0x0, nullptr), std::invalid_argument);
    }

    SECTION("can read entries")
    {
        std::vector<uint8_t> out_buffer;
        REQUIRE_NOTHROW(ReadEntry(buffer, 0x6ca6d4b9, &out_buffer));

        REQUIRE(FilesAreTheSame(out_buffer, world_buffer));
    }

    SECTION("can write entries")
    {
        std::vector<uint8_t> rb_buffer;
        REQUIRE_NOTHROW(WriteEntry("hello.bin", hello_buffer, &rb_buffer));
        REQUIRE_NOTHROW(WriteEntry("world.bin", world_buffer, &rb_buffer));

        REQUIRE(FilesAreTheSame(rb_buffer, buffer));
    }
}

TEST_CASE("Runtime Property Container", "[AvaFormatLib][RTPC]")
{
    using namespace ava::RuntimePropertyContainer;

    FileBuffer buffer;
    ReadTestFile("random_encounter_bombs_away.epe", &buffer);

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
    ReadTestFile("weapons.aisystunec", &buffer);

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
    ReadTestFile("model.rbm", &buffer);

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
    ReadTestFile("cow.modelc", &modelc_buffer);

    FileBuffer meshc_buffer;
    ReadTestFile("cow.meshc", &meshc_buffer);

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

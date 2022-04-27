#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <AvaFormatLib.h>
#include <archives/legacy/archive_table.h>
#include <error.h>

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
    ReadTestFile("hello.bin", &hello_buffer); // uncompressed
    ReadTestFile("world.bin", &world_buffer); // compressed

    auto oodleLoadResult = ava::Oodle::LoadLib("D:/Steam/steamapps/common/Just Cause 4/oo2core_7_win64.dll");

    SECTION("handles invalid input arguments")
    {
        std::vector<TabEntry> entries;
        REQUIRE(Parse({}, &entries) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(Parse(tab_buffer, nullptr) == ava::Result::E_INVALID_ARGUMENT);
    }

    SECTION("file was parsed and entries vector has results")
    {
        std::vector<TabEntry> entries;
        REQUIRE(AVA_FL_SUCCEEDED(Parse(tab_buffer, &entries)));
        REQUIRE_FALSE(entries.empty());
        REQUIRE(entries[0].m_NameHash == ava::hashlittle("hello.bin"));
        REQUIRE(entries[1].m_NameHash == ava::hashlittle("world.bin"));
    }

    SECTION("can read uncompressed entries")
    {
        TabEntry entry{};
        REQUIRE(AVA_FL_SUCCEEDED(ReadEntry(tab_buffer, ava::hashlittle("hello.bin"), &entry)));

        REQUIRE(entry.m_Library == E_COMPRESS_LIBRARY_NONE);

        std::vector<uint8_t> file_buffer;
        REQUIRE(AVA_FL_SUCCEEDED(ReadEntryBuffer(arc_buffer, entry, &file_buffer)));
        REQUIRE(FilesAreTheSame(file_buffer, hello_buffer));
    }

    SECTION("can write uncompressed entries")
    {
        // TODO

        // FileBuffer new_tab_buffer, new_arc_buffer;
        // REQUIRE(AVA_FL_SUCCEEDED(WriteEntry(&new_tab_buffer, &new_arc_buffer, "hello.bin", hello_buffer)));
        // REQUIRE(FilesAreTheSame(new_tab_buffer, tab_buffer));
    }

    if (AVA_FL_SUCCEEDED(oodleLoadResult)) {
        SECTION("can read compressed entries")
        {
            TabEntry entry{};
            REQUIRE(AVA_FL_SUCCEEDED(ReadEntry(tab_buffer, ava::hashlittle("world.bin"), &entry)));

            REQUIRE(entry.m_Library == E_COMPRESS_LIBRARY_OODLE);

            std::vector<uint8_t> file_buffer;
            REQUIRE(AVA_FL_SUCCEEDED(ReadEntryBuffer(arc_buffer, entry, &file_buffer)));
            REQUIRE(FilesAreTheSame(file_buffer, world_buffer));
        }

        SECTION("can write compressed entries")
        {
            // TODO
        }
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

    SECTION("handles invalid input arguments")
    {
        std::vector<TabEntry> entries;
        REQUIRE(Parse({}, &entries) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(Parse(tab_buffer, nullptr) == ava::Result::E_INVALID_ARGUMENT);
    }

    SECTION("file was parsed and entries vector has results")
    {
        std::vector<TabEntry> entries;
        REQUIRE(AVA_FL_SUCCEEDED(Parse(tab_buffer, &entries)));
        REQUIRE_FALSE(entries.empty());
        REQUIRE(entries[0].m_NameHash == ava::hashlittle("hello.bin"));
    }

    SECTION("can read entries")
    {
        TabEntry entry{};
        REQUIRE(AVA_FL_SUCCEEDED(ReadEntry(tab_buffer, ava::hashlittle("world.bin"), &entry)));

        std::vector<uint8_t> file_buffer;
        REQUIRE(AVA_FL_SUCCEEDED(ReadEntryBuffer(arc_buffer, entry, &file_buffer)));
        REQUIRE(FilesAreTheSame(file_buffer, world_buffer));
    }

    SECTION("can write entries")
    {
        FileBuffer t_buffer, a_buffer;
        REQUIRE(AVA_FL_SUCCEEDED(WriteEntry(&t_buffer, &a_buffer, "hello.bin", hello_buffer)));
        REQUIRE(AVA_FL_SUCCEEDED(WriteEntry(&t_buffer, &a_buffer, "world.bin", world_buffer)));
        REQUIRE(FilesAreTheSame(t_buffer, tab_buffer));
        REQUIRE(FilesAreTheSame(a_buffer, arc_buffer));
    }
}

TEST_CASE("Avalanche Archive Format", "[AvaFormatLib][AAF]")
{
    using namespace ava::AvalancheArchiveFormat;

    FileBuffer buffer;
    ReadTestFile("grapplinghookwire.ee", &buffer);

    SECTION("handles invalid input arguments")
    {
        FileBuffer out_buffer;
        REQUIRE(Decompress({}, &out_buffer) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(Decompress(buffer, nullptr) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(Compress({}, &out_buffer) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(Compress(buffer, nullptr) == ava::Result::E_INVALID_ARGUMENT);
    }

    SECTION("decompression has valid output buffer")
    {
        FileBuffer decompressed_buffer;
        REQUIRE(AVA_FL_SUCCEEDED(Decompress(buffer, &decompressed_buffer)));
        REQUIRE_FALSE(decompressed_buffer.empty());

        SECTION("decompressed output buffer is valid SARC format")
        {
            std::vector<ava::StreamArchive::ArchiveEntry> entries;
            REQUIRE(AVA_FL_SUCCEEDED(ava::StreamArchive::Parse(decompressed_buffer, &entries)));
            REQUIRE(entries.size() == 81);
            REQUIRE(entries[80].m_Filename == "editor/entities/gameobjects/grapplinghookwire.epe");
        }

        SECTION("compressed buffer matches original file buffer")
        {
            FileBuffer recompressed_buffer;
            REQUIRE(AVA_FL_SUCCEEDED(Compress(decompressed_buffer, &recompressed_buffer)));
            REQUIRE(recompressed_buffer.size() == buffer.size());
            REQUIRE(FilesAreTheSame(recompressed_buffer, buffer));
        }
    }
}

TEST_CASE("Stream Archive", "[AvaFormatLib][SARC]")
{
    using namespace ava::StreamArchive;

    SECTION("handles invalid input arguments")
    {
        std::vector<ArchiveEntry> entries;
        REQUIRE(Parse({}, &entries) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(Parse({1}, nullptr) == ava::Result::E_INVALID_ARGUMENT);
    }

    SECTION("v2")
    {
        FileBuffer buffer;
        ReadTestFile("wgst002_skin_flame.sarc", &buffer);

        std::vector<ArchiveEntry> entries;
        REQUIRE(AVA_FL_SUCCEEDED(Parse(buffer, &entries)));

        SECTION("file was parsed and entries vector has results")
        {
            REQUIRE_FALSE(entries.empty());
            REQUIRE(entries.at(10).m_Filename == "models/jc_aero/wingsuit/wingsuit_dlc_body.lod");
        }

        SECTION("can read entries")
        {
            std::vector<uint8_t> out_buffer;
            REQUIRE(AVA_FL_SUCCEEDED(
                ReadEntry(buffer, entries, "editor/entities/dlc/wingsuit_skins/wgst002_skin_flame.epe", &out_buffer)));
            REQUIRE_FALSE(out_buffer.empty());
            REQUIRE((out_buffer[0] == 'R' && out_buffer[1] == 'T' && out_buffer[2] == 'P' && out_buffer[3] == 'C'));
        }

        SECTION("can write entries")
        {
            FileBuffer world_buffer;
            ReadTestFile("world.bin", &world_buffer);

            REQUIRE(AVA_FL_SUCCEEDED(WriteEntry(&buffer, &entries, "world.bin", world_buffer)));
            REQUIRE(entries.back().m_Filename == "world.bin");
        }
    }

    SECTION("v3")
    {
        FileBuffer buffer;
        ReadTestFile("paratrooper_drop.ee", &buffer);

        std::vector<ArchiveEntry> entries;
        REQUIRE(AVA_FL_SUCCEEDED(Parse(buffer, &entries)));

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
            REQUIRE(AVA_FL_SUCCEEDED(ReadEntry(
                buffer, entries, "ai/bts/01_micro_behaviors/vehicle/airplane_fly_on_spline.btc", &out_buffer)));
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

    SECTION("handles invalid input arguments")
    {
        std::vector<ArchiveEntry> entries;
        REQUIRE(ParseTOC({}, &entries) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(ParseTOC(buffer, nullptr) == ava::Result::E_INVALID_ARGUMENT);
    }

    SECTION("file was parsed and entries vector has results")
    {
        std::vector<ArchiveEntry> entries;
        REQUIRE(AVA_FL_SUCCEEDED(ParseTOC(buffer, &entries)));
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

    SECTION("handles invalid input arguments")
    {
        std::vector<uint8_t> out_buffer;
        REQUIRE(ReadEntry({}, 0x0, &out_buffer) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(ReadEntry(buffer, 0x0, nullptr) == ava::Result::E_INVALID_ARGUMENT);
    }

    SECTION("can read entries")
    {
        std::vector<uint8_t> out_buffer;
        REQUIRE(AVA_FL_SUCCEEDED(ReadEntry(buffer, ava::hashlittle("world.bin"), &out_buffer)));
        REQUIRE(FilesAreTheSame(out_buffer, world_buffer));
    }

    SECTION("can write entries")
    {
        std::vector<uint8_t> rb_buffer;
        REQUIRE(AVA_FL_SUCCEEDED(WriteEntry(&rb_buffer, "hello.bin", hello_buffer)));
        REQUIRE(AVA_FL_SUCCEEDED(WriteEntry(&rb_buffer, "world.bin", world_buffer)));
        REQUIRE(FilesAreTheSame(rb_buffer, buffer));
    }
}

#if 0
TEST_CASE("Runtime Property Container", "[AvaFormatLib][RTPC]")
{
    using namespace ava::RuntimePropertyContainer;

    FileBuffer buffer;
    ReadTestFile("random_encounter_bombs_away.epe", &buffer);

    // SECTION("handles invalid input arguments")
    // {
    //     REQUIRE_THROWS_AS(
    //         []() {
    //             // invalid input buffer
    //             RuntimeContainer rtpc({});
    //         }(),
    //         std::invalid_argument);
    // }

    SECTION("123")
    {
        RuntimeContainer rtpc(buffer);
    }
}
#endif

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

    // SECTION("handles invalid input arguments")
    // {
    //     REQUIRE_THROWS_AS(
    //         [] {
    //             // invalid input buffer
    //             std::vector<uint8_t> buffer;
    //             ADF                  adf(buffer);
    //         }(),
    //         std::invalid_argument);
    // }

    SECTION("can get root instance")
    {
        ADF           adf(buffer);
        SInstanceInfo instance_info{};

        REQUIRE(adf.GetInstance(0, &instance_info));
        REQUIRE(instance_info.m_NameHash == 0xd9066df1);
    }

    SECTION("can read root instance")
    {
        ADF adf(buffer);

        // read instance
        WeaponTweaks* weapon_tweaks = nullptr;
        REQUIRE(adf.ReadInstance(0xd9066df1, 0x8dfb5000, (void**)&weapon_tweaks));
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

    SECTION("handles invalid input arguments")
    {
        static auto hash_handler = [](uint32_t hash, const std::vector<uint8_t>& buffer) {};
        REQUIRE(Parse({}, hash_handler) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(Parse(buffer, nullptr) == ava::Result::E_INVALID_ARGUMENT);
    }

    SECTION("file was parsed")
    {
        static auto hash_handler = [](uint32_t hash, const std::vector<uint8_t>& buffer) { REQUIRE_FALSE(true); };
        REQUIRE(AVA_FL_SUCCEEDED(Parse(buffer, hash_handler)));
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

    SECTION("handles invalid input arguments")
    {
        ADF*             adf         = nullptr;
        SAmfModel*       amf_model   = nullptr;
        SAmfMeshHeader*  mesh_header = nullptr;
        SAmfMeshBuffers* mesh_buffer = nullptr;
        REQUIRE(ParseModelc({}, &adf, &amf_model) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(ParseMeshc({}, &adf, &mesh_header, &mesh_buffer) == ava::Result::E_INVALID_ARGUMENT);
    }

    SECTION("MODELC file was parsed")
    {
        ADF*       adf       = nullptr;
        SAmfModel* amf_model = nullptr;
        REQUIRE(AVA_FL_SUCCEEDED(ParseModelc(modelc_buffer, &adf, &amf_model)));

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
        REQUIRE(AVA_FL_SUCCEEDED(ParseMeshc(meshc_buffer, &adf, &mesh_header, &mesh_buffer)));

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

TEST_CASE("Avalanche Texture", "[AvaFormatLib][AVTX]")
{
    using namespace ava::AvalancheTexture;

    FileBuffer buffer;
    ReadTestFile("test2.ddsc", &buffer);

    SECTION("handles invalid input arguments")
    {
        TextureEntry         out_entry{};
        std::vector<uint8_t> out_buffer;
        REQUIRE(ReadBestEntry({}, &out_entry, &out_buffer) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(ReadBestEntry(buffer, nullptr, &out_buffer) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(ReadBestEntry(buffer, &out_entry, nullptr) == ava::Result::E_INVALID_ARGUMENT);

        REQUIRE(ReadEntry({}, 0, &out_entry, &out_buffer) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(ReadEntry(buffer, 0, nullptr, &out_buffer) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(ReadEntry(buffer, 0, &out_entry, nullptr) == ava::Result::E_INVALID_ARGUMENT);
        REQUIRE(ReadEntry(buffer, AVTX_MAX_STREAMS, &out_entry, &out_buffer) == ava::Result::E_INVALID_ARGUMENT);
    }

#if 0
	SECTION("source stream throws if source buffer is empty")
	{
		// @TODO
		// ReadEntry(buffer, stream_index, out_entry, &out_buffer, {});
	}
#endif

    SECTION("can read texture stream entry")
    {
        TextureEntry         entry{};
        std::vector<uint8_t> out_buffer;
        REQUIRE(AVA_FL_SUCCEEDED(ReadBestEntry(buffer, &entry, &out_buffer)));
        REQUIRE(entry.m_Width == 300);
        REQUIRE(entry.m_Height == 300);
        REQUIRE_FALSE(out_buffer.empty());
    }

#if 0
	SECTION("can write texture stream entry")
	{
		// @TODO
		// WriteEntry(buffer, entry, texture_buffer, nullptr);
	}
#endif
}

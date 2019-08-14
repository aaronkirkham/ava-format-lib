#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace ava::RenderBlockModel
{
static constexpr uint32_t RBM_BLOCK_CHECKSUM = 0x89ABCDEF;

enum RenderBlockType : uint32_t {
    RB_2DTEX1                    = 0x45DBC85F,
    RB_2DTEX2                    = 0x8663465,
    RB_3DTEXT                    = 0x474ECF38,
    RB_AOBOX                     = 0x67BCF1D0,
    RB_ADDFOGVOLUME              = 0xCF84413C,
    RB_ATMOSPHERICSCATTERING     = 0x90D7C43,
    RB_BARK                      = 0x898323B2,
    RB_BAVARIUMSHIELD            = 0xA5D24CCD,
    RB_BEAM                      = 0x1B5E8315,
    RB_BILLBOARD                 = 0x389265A9,
    RB_BOX                       = 0x416C4035,
    RB_BUILDINGJC3               = 0x35BF53D5,
    RB_BULLET                    = 0x91571CF0,
    RB_CARLIGHT                  = 0xDB948BF1,
    RB_CARPAINTMM                = 0x483304D6,
    RB_CHARACTER                 = 0x9D6E332A,
    RB_CHARACTERSKIN             = 0x626F5E3B,
    RB_CIRRUSCLOUDS              = 0x3449988B,
    RB_CLOUDS                    = 0xA399123E,
    RB_DECALDEFORMABLE           = 0xDF9E9916,
    RB_DECALSIMPLE               = 0x2BAFDD6C,
    RB_DECALSKINNED              = 0x8A42C0C1,
    RB_DECALSKINNEDDESTRUCTION   = 0x4684BC,
    RB_DEFERREDLIGHTING          = 0x4598D520,
    RB_DEPTHDOWNSAMPLE           = 0xF9EF9DE4,
    RB_ENVIRONMENTREFLECTION     = 0x7287FA5F,
    RB_FXMESHFIRE                = 0x6FF3659,
    RB_FLAG                      = 0xD885FCDB,
    RB_FOGGRADIENT               = 0x1637FB2A,
    RB_FOGVOLUME                 = 0xB0E85383,
    RB_FOILAGE                   = 0x3C8DE6D3,
    RB_FONT                      = 0x7E8F98CE,
    RB_FULLSCREENVIDEO           = 0xAE7E6231,
    RB_GIONLY                    = 0x91C2D583,
    RB_GENERAL                   = 0xA7583B2B,
    RB_GENERALJC3                = 0x2EE0F4A9,
    RB_GENERALMASKEDJC3          = 0x8F2335EC,
    RB_GENERALMKIII              = 0x2CEC5AD5,
    RB_HALO                      = 0x65D9B5B2,
    RB_LANDMARK                  = 0x3B630E6D,
    RB_LAYERED                   = 0xC7021EE3,
    RB_LIGHTGLOW                 = 0xD2033C51,
    RB_LIGHTS                    = 0xDB48F55A,
    RB_LINE                      = 0xC07B6866,
    RB_LRCLOUDSCOMPOSE           = 0xE13A427D,
    RB_MATERIALTUNE              = 0xC2457E44,
    RB_MESHPARTICLE              = 0xBAE64FF8,
    RB_OCCLUDER                  = 0x2A44553C,
    RB_OCCULSION                 = 0xBD231E4,
    RB_OPEN                      = 0x5B13AA49,
    RB_OUTLINEEFFECTBLUR         = 0xDAF5CDB0,
    RB_PARTICLECOMPOSE           = 0x6B535F86,
    RB_POSTEFFECTS               = 0x58DE8D87,
    RB_PROP                      = 0x4894ECD,
    RB_RAINOCCLUDER              = 0x7450659E,
    RB_REFLECTION                = 0xA981F55F,
    RB_ROADJUNCTION              = 0x566DCE92,
    RB_SSAO                      = 0xC2C03635,
    RB_SSDECAL                   = 0x4FE3AE77,
    RB_SCENECAPTURE              = 0x3EB17238,
    RB_SCREENSPACEREFLECTION     = 0xDA1EB637,
    RB_SCREENSPACESUBSURFACESKIN = 0xB65AC9D7,
    RB_SINGLE                    = 0x9D1EE307,
    RB_SKIDMARKS                 = 0x4D3A7D2F,
    RB_SKYBOX                    = 0xC24EFB87,
    RB_SKYGRADIENT               = 0x64076188,
    RB_SKYMODEL                  = 0x2D95C25E,
    RB_SOFTCLOUDS                = 0xB308E2F4,
    RB_SPHERICALHARMONICPROBE    = 0x359F6B2C,
    RB_SPLINEROAD                = 0xEDABAD,
    RB_SPOTLIGHTCONE             = 0x33E6BCA3,
    RB_TERRAINPATCH              = 0x815DF732,
    RB_TRAIL                     = 0x858E7014,
    RB_TRIANGLE                  = 0xEB96E782,
    RB_UNDERWATERFOGGRADIENT     = 0x69DE065B,
    RB_VEGINTRECENTER            = 0xD89EF9,
    RB_VEGINTERACTIONVOLUME      = 0x6D87DBFC,
    RB_VEGSAMPLING               = 0x77B4C335,
    RB_VEGSYSTEMPOSTDRAW         = 0x20DA8E9D,
    RB_VEGUPDATE                 = 0x1AF1F8AC,
    RB_WATERDISPLACEMENTOVERRIDE = 0xF99C72A1,
    RB_WATERMARK                 = 0x90FE086C,
    RB_WATERREFLECTIONPLANE      = 0x910EDC80,
    RB_WEATHER                   = 0x89215D85,
    RB_WINDOW                    = 0x5B2003F6,
};

#pragma pack(push, 1)
struct RbmHeader {
    uint32_t m_MagicLength       = 5;
    uint8_t  m_Magic[5]          = {'R', 'B', 'M', 'D', 'L'};
    uint32_t m_VersionMajor      = 1;
    uint32_t m_VersionMinor      = 16;
    uint32_t m_VersionRevision   = 0;
    float    m_BoundingBoxMin[3] = {0};
    float    m_BoundingBoxMax[3] = {0};
    uint32_t m_NumberOfBlocks    = 0;
    uint32_t m_Flags             = 0;
};
#pragma pack(pop)

static_assert(sizeof(RbmHeader) == 0x35, "RbmHeader alignment is wrong!");

using RBMHashHandler = std::function<void(uint32_t, const std::vector<uint8_t>& buffer)>;
void Parse(const std::vector<uint8_t>& buffer, RBMHashHandler rbm_hash_handler);
}; // namespace ava::RenderBlockModel

#pragma once

#include <cstdint>
#include <vector>

#include "../avalanche_data_format.h"

namespace ava::AvalancheModelFormat
{
enum EAmfUsage {
    AmfUsage_Unspecified       = 0x0,
    AmfUsage_Position          = 0x1,
    AmfUsage_TextureCoordinate = 0x2,
    AmfUsage_Normal            = 0x3,
    AmfUsage_Tangent           = 0x4,
    AmfUsage_BiTangent         = 0x5,
    AmfUsage_TangentSpace      = 0x6,
    AmfUsage_BoneIndex         = 0x7,
    AmfUsage_BoneWeight        = 0x8,
    AmfUsage_Color             = 0x9,
    AmfUsage_WireRadius        = 0xA,
};

enum EAmfFormat {
    AmfFormat_R32G32B32A32_FLOAT          = 0x0,
    AmfFormat_R32G32B32A32_UINT           = 0x1,
    AmfFormat_R32G32B32A32_SINT           = 0x2,
    AmfFormat_R32G32B32_FLOAT             = 0x3,
    AmfFormat_R32G32B32_UINT              = 0x4,
    AmfFormat_R32G32B32_SINT              = 0x5,
    AmfFormat_R16G16B16A16_FLOAT          = 0x6,
    AmfFormat_R16G16B16A16_UNORM          = 0x7,
    AmfFormat_R16G16B16A16_UINT           = 0x8,
    AmfFormat_R16G16B16A16_SNORM          = 0x9,
    AmfFormat_R16G16B16A16_SINT           = 0xA,
    AmfFormat_R16G16B16_FLOAT             = 0xB,
    AmfFormat_R16G16B16_UNORM             = 0xC,
    AmfFormat_R16G16B16_UINT              = 0xD,
    AmfFormat_R16G16B16_SNORM             = 0xE,
    AmfFormat_R16G16B16_SINT              = 0xF,
    AmfFormat_R32G32_FLOAT                = 0x10,
    AmfFormat_R32G32_UINT                 = 0x11,
    AmfFormat_R32G32_SINT                 = 0x12,
    AmfFormat_R10G10B10A2_UNORM           = 0x13,
    AmfFormat_R10G10B10A2_UINT            = 0x14,
    AmfFormat_R11G11B10_FLOAT             = 0x15,
    AmfFormat_R8G8B8A8_UNORM              = 0x16,
    AmfFormat_R8G8B8A8_UNORM_SRGB         = 0x17,
    AmfFormat_R8G8B8A8_UINT               = 0x18,
    AmfFormat_R8G8B8A8_SNORM              = 0x19,
    AmfFormat_R8G8B8A8_SINT               = 0x1A,
    AmfFormat_R16G16_FLOAT                = 0x1B,
    AmfFormat_R16G16_UNORM                = 0x1C,
    AmfFormat_R16G16_UINT                 = 0x1D,
    AmfFormat_R16G16_SNORM                = 0x1E,
    AmfFormat_R16G16_SINT                 = 0x1F,
    AmfFormat_R32_FLOAT                   = 0x20,
    AmfFormat_R32_UINT                    = 0x21,
    AmfFormat_R32_SINT                    = 0x22,
    AmfFormat_R8G8_UNORM                  = 0x23,
    AmfFormat_R8G8_UINT                   = 0x24,
    AmfFormat_R8G8_SNORM                  = 0x25,
    AmfFormat_R8G8_SINT                   = 0x26,
    AmfFormat_R16_FLOAT                   = 0x27,
    AmfFormat_R16_UNORM                   = 0x28,
    AmfFormat_R16_UINT                    = 0x29,
    AmfFormat_R16_SNORM                   = 0x2A,
    AmfFormat_R16_SINT                    = 0x2B,
    AmfFormat_R8_UNORM                    = 0x2C,
    AmfFormat_R8_UINT                     = 0x2D,
    AmfFormat_R8_SNORM                    = 0x2E,
    AmfFormat_R8_SINT                     = 0x2F,
    AmfFormat_R32_UNIT_VEC_AS_FLOAT       = 0x30,
    AmfFormat_R32_R8G8B8A8_UNORM_AS_FLOAT = 0x31,
    AmfFormat_R8G8B8A8_TANGENT_SPACE      = 0x32,
};

#pragma pack(push, 8)
struct SAmfMaterial {
    uint32_t                                      m_Name;
    uint32_t                                      m_RenderBlockId;
    ava::AvalancheDataFormat::SAdfDeferredPtr     m_Attributes;
    ava::AvalancheDataFormat::SAdfArray<uint32_t> m_Textures;
};

struct SAmfModel {
    uint32_t                                          m_Mesh;
    ava::AvalancheDataFormat::SAdfArray<uint8_t>      m_LodSlots;
    uint32_t                                          m_MemoryTag;
    float                                             m_LodFactor;
    ava::AvalancheDataFormat::SAdfArray<SAmfMaterial> m_Materials;
};
#pragma pack(pop)

static_assert(sizeof(SAmfMaterial) == 0x28, "SAmfMaterial alignment is wrong!");
static_assert(sizeof(SAmfModel) == 0x30, "SAmfModel alignment is wrong!");

#pragma pack(push, 8)
struct SAmfBoundingBox {
    float m_Min[3];
    float m_Max[3];
};

struct SAmfSubMesh {
    uint32_t        m_SubMeshId;
    uint32_t        m_IndexCount;
    uint32_t        m_IndexStreamOffset;
    SAmfBoundingBox m_BoundingBox;
};

struct SAmfStreamAttribute {
    EAmfUsage  m_Usage;
    EAmfFormat m_Format;
    uint8_t    m_StreamIndex;
    uint8_t    m_StreamOffset;
    uint8_t    m_StreamStride;
    uint8_t    m_PackingData[8];
};

struct SAmfMesh {
    uint32_t                                                 m_MeshTypeId;
    uint32_t                                                 m_IndexCount;
    uint32_t                                                 m_VertexCount;
    int8_t                                                   m_IndexBufferIndex;
    int8_t                                                   m_IndexBufferStride;
    uint32_t                                                 m_IndexBufferOffset;
    ava::AvalancheDataFormat::SAdfArray<int8_t>              m_VertexBufferIndices;
    ava::AvalancheDataFormat::SAdfArray<int8_t>              m_VertexStreamStrides;
    ava::AvalancheDataFormat::SAdfArray<int32_t>             m_VertexStreamOffsets;
    float                                                    m_TextureDensities[3];
    ava::AvalancheDataFormat::SAdfDeferredPtr                m_MeshProperties;
    ava::AvalancheDataFormat::SAdfArray<int16_t>             m_BoneIndexLookup;
    ava::AvalancheDataFormat::SAdfArray<SAmfSubMesh>         m_SubMeshes;
    ava::AvalancheDataFormat::SAdfArray<SAmfStreamAttribute> m_StreamAttributes;
};

struct SAmfLodGroup {
    uint32_t                                      m_LODIndex;
    ava::AvalancheDataFormat::SAdfArray<SAmfMesh> m_Meshes;
};

struct SAmfMeshHeader {
    SAmfBoundingBox                                   m_BoundingBox;
    uint32_t                                          m_MemoryTag;
    ava::AvalancheDataFormat::SAdfArray<SAmfLodGroup> m_LodGroups;
    uint32_t                                          m_HighLodPath;
};
#pragma pack(pop)

static_assert(sizeof(SAmfLodGroup) == 0x18, "SAmfLodGroup alignment is wrong!");
static_assert(sizeof(SAmfMeshHeader) == 0x38, "SAmfMeshHeader alignment is wrong!");

#pragma pack(push, 8)
struct SAmfBuffer {
    ava::AvalancheDataFormat::SAdfArray<int8_t> m_Data;
    int8_t                                      m_CreateSRV : 1;
};

struct SAmfMeshBuffers {
    uint32_t                                        m_MemoryTag;
    ava::AvalancheDataFormat::SAdfArray<SAmfBuffer> m_IndexBuffers;
    ava::AvalancheDataFormat::SAdfArray<SAmfBuffer> m_VertexBuffers;
};
#pragma pack(pop)

static_assert(sizeof(SAmfBuffer) == 0x18, "SAmfBuffer alignment is wrong!");
static_assert(sizeof(SAmfMeshBuffers) == 0x28, "SAmfMeshBuffers alignment is wrong!");

void ParseModelc(const std::vector<uint8_t>& buffer, ava::AvalancheDataFormat::ADF** out_adf, SAmfModel** out_model);
void ParseMeshc(const std::vector<uint8_t>& buffer, ava::AvalancheDataFormat::ADF** out_adf,
                SAmfMeshHeader** out_mesh_header, SAmfMeshBuffers** out_mesh_buffer);
void ParseHrmeshc(const std::vector<uint8_t>& buffer, ava::AvalancheDataFormat::ADF** out_adf,
                  SAmfMeshBuffers** out_mesh_buffer);
}; // namespace ava::AvalancheModelFormat

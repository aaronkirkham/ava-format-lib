#pragma once

#include "avalanche_data_format.h"
#include "error.h"

#include <cstdint>
#include <vector>

namespace ava::ShaderBundle
{
#pragma pack(push, 8)
struct SShader {
    uint32_t           m_NameHash;
    const char*        m_Name;
    uint32_t           m_DataHash;
    SAdfArray<uint8_t> m_BinaryData;
};

struct SShaderLibrary {
    const char*        m_Name;
    const char*        m_BuildTime;
    SAdfArray<SShader> m_VertexShaders;
    SAdfArray<SShader> m_FragmentShaders;
    SAdfArray<SShader> m_ComputeShaders;
    SAdfArray<SShader> m_GeometryShaders;
    SAdfArray<SShader> m_HullShaders;
    SAdfArray<SShader> m_DomainShaders;
};

static_assert(sizeof(SShader) == 0x28, "SShader alignment is wrong!");
static_assert(sizeof(SShaderLibrary) == 0x70, "SShaderLibrary alignment is wrong!");
#pragma pack(pop)

/**
 * Parse a .shader_bundle ADF buffer
 *
 * @param buffer Input buffer containing a raw shader_bundle ADF file buffer
 * @param out_adf Pointer to an ADF struct where the ADF instance will be stored (You're responsible for calling delete
 * on this pointer the memory once you're done with it)
 * @param out_shader_library Pointer to a SShaderLibrary struct where the data will be stored once parsed (You're
 * responsible for calling free() on this pointer once you're done with it)
 */
Result Parse(const std::vector<uint8_t>& buffer, AvalancheDataFormat::ADF** out_adf,
             SShaderLibrary** out_shader_library);
} // namespace ava::ShaderBundle
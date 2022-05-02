#include <shader_bundle.h>

namespace ava::ShaderBundle
{
Result Parse(const std::vector<uint8_t>& buffer, AvalancheDataFormat::ADF** out_adf, SShaderLibrary** out_shader_bundle)
{
    if (buffer.empty()) {
        return E_INVALID_ARGUMENT;
    }

    const auto adf = new AvalancheDataFormat::ADF(buffer);
    *out_adf       = adf;

    // read the shader bundle data
    AvalancheDataFormat::SInstanceInfo instance_info{};
    adf->GetInstance(0, &instance_info);
    adf->ReadInstance(instance_info.m_NameHash, 0xF2923B32, (void**)out_shader_bundle);
    return E_OK;
}
} // namespace ava::ShaderBundle
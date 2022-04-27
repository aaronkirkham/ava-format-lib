#include <models/avalanche_model_format.h>

#include <memory>

namespace ava::AvalancheModelFormat
{
Result ParseModelc(const std::vector<uint8_t>& buffer, ava::AvalancheDataFormat::ADF** out_adf, SAmfModel** out_model)
{
    if (buffer.empty()) {
        // throw std::invalid_argument("MODELC input buffer can't be empty!");
        return E_INVALID_ARGUMENT;
    }

    const auto adf = new ava::AvalancheDataFormat::ADF(buffer);
    *out_adf       = adf;

    adf->ReadInstance(0, (void**)out_model);
    return E_OK;
}

Result ParseMeshc(const std::vector<uint8_t>& buffer, ava::AvalancheDataFormat::ADF** out_adf,
                  SAmfMeshHeader** out_mesh_header, SAmfMeshBuffers** out_mesh_buffer)
{
    if (buffer.empty()) {
        // throw std::invalid_argument("MESHC input buffer can't be empty!");
        return E_INVALID_ARGUMENT;
    }

    const auto adf = new ava::AvalancheDataFormat::ADF(buffer);
    *out_adf       = adf;

    adf->ReadInstance(0, (void**)out_mesh_header);
    adf->ReadInstance(1, (void**)out_mesh_buffer);
    return E_OK;
}

Result ParseHrmeshc(const std::vector<uint8_t>& buffer, ava::AvalancheDataFormat::ADF** out_adf,
                    SAmfMeshBuffers** out_mesh_buffer)
{
    if (buffer.empty()) {
        // throw std::invalid_argument("HRMESHC input buffer can't be empty!");
        return E_INVALID_ARGUMENT;
    }

    const auto adf = new ava::AvalancheDataFormat::ADF(buffer);
    *out_adf       = adf;

    adf->ReadInstance(0, (void**)out_mesh_buffer);
    return E_OK;
}
}; // namespace ava::AvalancheModelFormat

#include "../../include/models/avalanche_model_format.h"

#include <memory>

namespace ava::AvalancheModelFormat
{
void ParseModelc(const std::vector<uint8_t>& buffer, ava::AvalancheDataFormat::ADF** out_adf, SAmfModel** out_model)
{
    if (buffer.empty()) {
        throw std::invalid_argument("MODELC input buffer can't be empty!");
    }

    const auto adf = new ava::AvalancheDataFormat::ADF(buffer);
    *out_adf       = adf;

    adf->ReadInstance(0, (void**)out_model);
}

void ParseMeshc(const std::vector<uint8_t>& buffer, ava::AvalancheDataFormat::ADF** out_adf,
                SAmfMeshHeader** out_mesh_header, SAmfMeshBuffers** out_mesh_buffer)
{
    if (buffer.empty()) {
        throw std::invalid_argument("MESHC input buffer can't be empty!");
    }

    const auto adf = new ava::AvalancheDataFormat::ADF(buffer);
    *out_adf       = adf;

    adf->ReadInstance(0, (void**)out_mesh_header);
    adf->ReadInstance(1, (void**)out_mesh_buffer);
}

void ParseHrmeshc(const std::vector<uint8_t>& buffer, ava::AvalancheDataFormat::ADF** out_adf,
                  SAmfMeshBuffers** out_mesh_buffer)
{
    if (buffer.empty()) {
        throw std::invalid_argument("HRMESHC input buffer can't be empty!");
    }

    const auto adf = new ava::AvalancheDataFormat::ADF(buffer);
    *out_adf       = adf;

    adf->ReadInstance(0, (void**)out_mesh_buffer);
}
}; // namespace ava::AvalancheModelFormat

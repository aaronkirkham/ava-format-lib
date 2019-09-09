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

    ava::AvalancheDataFormat::SInstanceInfo instance{};
    adf->GetInstance(0, &instance);
    adf->ReadInstance(instance, (void**)out_model);
}

void ParseMeshc(const std::vector<uint8_t>& buffer, ava::AvalancheDataFormat::ADF** out_adf,
                SAmfMeshHeader** out_mesh_header, SAmfMeshBuffers** out_mesh_buffer)
{
    if (buffer.empty()) {
        throw std::invalid_argument("MESHC input buffer can't be empty!");
    }

    const auto adf = new ava::AvalancheDataFormat::ADF(buffer);
    *out_adf       = adf;

    ava::AvalancheDataFormat::SInstanceInfo header_instance{};
    adf->GetInstance(0, &header_instance);
    adf->ReadInstance(header_instance, (void**)out_mesh_header);

    ava::AvalancheDataFormat::SInstanceInfo mesh_instance{};
    adf->GetInstance(1, &mesh_instance);
    adf->ReadInstance(mesh_instance, (void**)out_mesh_buffer);
}
}; // namespace ava::AvalancheModelFormat

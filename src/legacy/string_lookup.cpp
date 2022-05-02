#include <legacy/string_lookup.h>

namespace ava::legacy::StringLookup
{
Result Parse(const std::vector<uint8_t>& buffer, AvalancheDataFormat::ADF** out_adf, SStringLookup** out_string_lookup)
{
    if (buffer.empty()) {
        return E_INVALID_ARGUMENT;
    }

    const auto adf = new AvalancheDataFormat::ADF(buffer);
    *out_adf       = adf;

    adf->ReadInstance(0, (void**)out_string_lookup);
    return E_OK;
}
} // namespace ava::legacy::StringLookup
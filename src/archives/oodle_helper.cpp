#include "../include/archives/oodle_helper.h"

#include <Windows.h>
#include <assert.h>

namespace ava::Oodle
{
void LoadLib(const std::filesystem::path& oodle_dll_path)
{
    if (oodle_handle) {
        return;
    }

    if (!std::filesystem::exists(oodle_dll_path)) {
        throw std::invalid_argument("The Oodle DLL path specified does not exist.");
    }

    auto handle = LoadLibrary(oodle_dll_path.string().c_str());
    if (!handle) {
        throw std::runtime_error("Failed to load Oodle DLL.");
    }

    LoadLib(handle);
    we_looded_oodle = true;
}

void LoadLib(void* handle)
{
    if (oodle_handle || !handle) {
        return;
    }

    oodle_handle = handle;

    // load functions
    OodleLZ_Compress   = (OodleLZ_Compress_t)GetProcAddress((HMODULE)oodle_handle, "OodleLZ_Compress");
    OodleLZ_Decompress = (OodleLZ_Decompress_t)GetProcAddress((HMODULE)oodle_handle, "OodleLZ_Decompress");
    if (!OodleLZ_Compress || !OodleLZ_Decompress) {
        UnloadLib();
        throw std::runtime_error("Failed to find required functions inside Oodle DLL.");
    }
}

void UnloadLib()
{
    if (we_looded_oodle) {
        FreeLibrary((HMODULE)oodle_handle);
        we_looded_oodle = false;
    }

    oodle_handle       = nullptr;
    OodleLZ_Compress   = nullptr;
    OodleLZ_Decompress = nullptr;
}

int64_t Compress(const void* data, const int64_t data_size, const void* out_data)
{
    assert(oodle_handle);
    assert(OodleLZ_Compress);
    return OodleLZ_Compress(OodleLZCompresor_Kraken, data, data_size, out_data, OodleLZCompressionLevel_None, 0, 0, 0,
                            0, 0);
}

int64_t Compress(const std::vector<uint8_t>* data, std::vector<uint8_t>* out_data)
{
    const int64_t bound = GetCompressedBufferSizeNeeded(data->size());
    out_data->resize(bound);

    const int64_t result = Compress(data->data(), data->size(), out_data->data());
    out_data->resize(result);
    return result;
}

int64_t Decompress(const void* data, const int64_t data_size, const void* out_data, int64_t out_data_size)
{
    assert(oodle_handle);
    assert(OodleLZ_Decompress);
    return OodleLZ_Decompress(data, data_size, out_data, out_data_size, 1, 0, 0, nullptr, nullptr, nullptr, nullptr,
                              nullptr, 0, 3);
}

int64_t Decompress(const std::vector<uint8_t>* data, std::vector<uint8_t>* out_data)
{
    return Decompress(data->data(), data->size(), out_data->data(), out_data->size());
}
}; // namespace ava::Oodle

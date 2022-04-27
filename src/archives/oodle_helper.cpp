#include <archives/oodle_helper.h>

#include <Windows.h>
#include <assert.h>
#include <filesystem>

namespace ava::Oodle
{
Result LoadLib(const char* oodle_dll_path)
{
    if (oodle_handle) {
        return E_OK;
    }

    if (!std::filesystem::exists(oodle_dll_path)) {
        // throw std::invalid_argument("The Oodle DLL path specified does not exist.");
        return E_OODLE_LIBRARY_MISSING;
    }

    auto handle = LoadLibrary(oodle_dll_path);
    if (!handle) {
        // throw std::runtime_error("Failed to load Oodle DLL.");
        return E_OODLE_FAILED_TO_LOAD;
    }

    LoadLib(handle);
    we_looded_oodle = true;
    return E_OK;
}

Result LoadLib(void* handle)
{
    if (oodle_handle || !handle) {
        return E_OK;
    }

    oodle_handle = handle;

    // load functions
    OodleLZ_Compress   = (OodleLZ_Compress_t)GetProcAddress((HMODULE)oodle_handle, "OodleLZ_Compress");
    OodleLZ_Decompress = (OodleLZ_Decompress_t)GetProcAddress((HMODULE)oodle_handle, "OodleLZ_Decompress");
    if (!OodleLZ_Compress || !OodleLZ_Decompress) {
        UnloadLib();
        // throw std::runtime_error("Failed to find required functions inside Oodle DLL.");
        return E_OODLE_BAD_SIGNATURE;
    }

    return E_OK;
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
    if (!OodleLZ_Compress) {
        return 0;
    }

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
    if (!OodleLZ_Decompress) {
        return 0;
    }

    return OodleLZ_Decompress(data, data_size, out_data, out_data_size, 1, 0, 0, nullptr, nullptr, nullptr, nullptr,
                              nullptr, 0, 3);
}

int64_t Decompress(const std::vector<uint8_t>* data, std::vector<uint8_t>* out_data)
{
    return Decompress(data->data(), data->size(), out_data->data(), out_data->size());
}
}; // namespace ava::Oodle

#include "../include/archives/oodle_helper.h"

#include <Windows.h>
#include <assert.h>

namespace ava::OodleLZ
{
void Load(const std::filesystem::path& oodle_dll_path)
{
    if (oo2core_7_win64) {
        return;
    }

    if (!std::filesystem::exists(oodle_dll_path)) {
        throw std::invalid_argument("The Oodle DLL path specified does not exist.");
    }

    oo2core_7_win64 = LoadLibrary(oodle_dll_path.string().c_str());
    if (!oo2core_7_win64) {
        throw std::runtime_error("Failed to load Oodle DLL.");
    }

    Compress_orig   = (Compress_t)GetProcAddress((HMODULE)oo2core_7_win64, "OodleLZ_Compress");
    Decompress_orig = (Decompress_t)GetProcAddress((HMODULE)oo2core_7_win64, "OodleLZ_Decompress");
    if (!Compress_orig || !Decompress_orig) {
        Unload();
        throw std::runtime_error("Failed to find required functions inside Oodle DLL.");
    }
}

void Unload()
{
    if (!oo2core_7_win64) {
        return;
    }

    FreeLibrary((HMODULE)oo2core_7_win64);
    oo2core_7_win64 = nullptr;
    Compress_orig   = nullptr;
    Decompress_orig = nullptr;
}

int64_t Compress(const void* data, const int64_t data_size, const void* out_data)
{
    assert(oo2core_7_win64);
    assert(Compress_orig);
    return Compress_orig(OodleLZCompresor_Kraken, data, data_size, out_data, OodleLZCompressionLevel_None, 0, 0, 0, 0,
                         0);
}

int64_t Compress(const std::vector<uint8_t>* data, std::vector<uint8_t>* out_data)
{
    const int64_t bound = GetCompressedBufferSizeNeeded(data->size());
    out_data->resize(bound);

    const int64_t result = Compress(data->data(), data->size(), out_data->data());
    // out_data->resize(result);
    return result;
}

int64_t Decompress(const void* data, const int64_t data_size, const void* out_data, int64_t out_data_size)
{
    assert(oo2core_7_win64);
    assert(Decompress_orig);
    return Decompress_orig(data, data_size, out_data, out_data_size, 1, 0, 0, nullptr, nullptr, nullptr, nullptr,
                           nullptr, 0, 3);
}

int64_t Decompress(const std::vector<uint8_t>* data, std::vector<uint8_t>* out_data)
{
    return Decompress(data->data(), data->size(), out_data->data(), out_data->size());
}
}; // namespace ava::OodleLZ

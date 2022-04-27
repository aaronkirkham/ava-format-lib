#pragma once

#include "../error.h"

#include <cstdint>
#include <vector>

namespace ava::Oodle
{
enum CompressionLevel {
    OodleLZCompressionLevel_HyperFast4 = 0,
    OodleLZCompressionLevel_HyperFast3,
    OodleLZCompressionLevel_HyperFast2,
    OodleLZCompressionLevel_HyperFast1,
    OodleLZCompressionLevel_None,
    OodleLZCompressionLevel_SuperFast,
    OodleLZCompressionLevel_VeryFast,
    OodleLZCompressionLevel_Fast,
    OodleLZCompressionLevel_Normal,
    OodleLZCompressionLevel_Optimal1,
    OodleLZCompressionLevel_Optimal2,
    OodleLZCompressionLevel_Optimal3,
    OodleLZCompressionLevel_Optimal4,
    OodleLZCompressionLevel_Optimal5,
    OodleLZCompressionLevel_TooHigh,
};

enum Compresor {
    OodleLZCompresor_LZH = 0,
    OodleLZCompresor_LZHLW,
    OodleLZCompresor_LZNIB,
    OodleLZCompresor_None,
    OodleLZCompresor_LZB16,
    OodleLZCompresor_LZBLW,
    OodleLZCompresor_LZA,
    OodleLZCompresor_LZNA,
    OodleLZCompresor_Kraken,
    OodleLZCompresor_Mermaid,
    OodleLZCompresor_BitKnit,
    OodleLZCompresor_Selkie,
    OodleLZCompresor_Hydra,
    OodleLZCompresor_Leviathan,
};

using OodleLZ_Compress_t   = int64_t (*)(Compresor type, const void* input, int64_t input_size, const void* output,
                                       CompressionLevel level, uint32_t*, int64_t, int64_t, int64_t, int64_t);
using OodleLZ_Decompress_t = int64_t (*)(const void* input, int64_t input_size, const void* output, int64_t output_size,
                                         int32_t, int64_t, int64_t, void*, void*, void*, void*, void* unk,
                                         int64_t unk_size, int64_t);

static bool                 we_looded_oodle    = false;
static void*                oodle_handle       = nullptr;
static OodleLZ_Compress_t   OodleLZ_Compress   = nullptr;
static OodleLZ_Decompress_t OodleLZ_Decompress = nullptr;

Result LoadLib(const char* oodle_dll_path);
Result LoadLib(void* handle);
void   UnloadLib();

static int64_t GetCompressedBufferSizeNeeded(int64_t size)
{
    return (size + 274 * ((size + 0x3FFFF) / 0x40000));
}

int64_t Compress(const void* data, const int64_t data_size, const void* out_data);
int64_t Compress(const std::vector<uint8_t>* data, std::vector<uint8_t>* out_data);
int64_t Decompress(const void* data, const int64_t data_size, const void* out_data, int64_t out_data_size);
int64_t Decompress(const std::vector<uint8_t>* data, std::vector<uint8_t>* out_data);
}; // namespace ava::Oodle

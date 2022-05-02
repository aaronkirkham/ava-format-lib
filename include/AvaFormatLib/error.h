#pragma once

namespace ava
{
enum Result {
    E_OK,
    E_INVALID_ARGUMENT,
    E_NOT_IMPLEMENTED,

    // oodle
    E_OODLE_LIBRARY_MISSING,
    E_OODLE_FAILED_TO_LOAD,
    E_OODLE_BAD_SIGNATURE,

    // TAB
    E_TAB_INVALID_MAGIC,
    E_TAB_UNKNOWN_ENTRY,
    E_TAB_INPUT_REQUIRES_COMPRESSION_BLOCKS,
    E_TAB_COMPRESS_BLOCK_FAILED,
    E_TAB_DECOMPRESS_BLOCK_FAILED,

    // AAF
    E_AAF_INVALID_MAGIC,
    E_AAF_INVALID_CHUNK_MAGIC,
    E_AAF_COMPRESS_CHUNK_FAILED,
    E_AAF_DECOMPRESS_CHUNK_FAILED,

    // SARC
    E_SARC_INVALID_MAGIC,
    E_SARC_UNKNOWN_VERSION,
    E_SARC_PATCHED_ENTRY,

    // RBMDL
    E_RBMDL_INVALID_MAGIC,
    E_RBMDL_UNKNOWN_VERSION,
    E_RBMDL_BAD_CHECKSUM,

    // ADF
    E_ADF_INVALID_MAGIC,

    // RTPC
    E_RTPC_INVALID_MAGIC,

    // AVTX
    E_AVTX_INVALID_MAGIC,
    E_AVTX_UNKNOWN_VERSION,
    E_AVTX_SOURCE_BUFFER_NEEDED,
};

static const char* ResultToString(Result result)
{
    switch (result) {
        case E_OK: return "E_OK";
        case E_INVALID_ARGUMENT: return "E_INVALID_ARGUMENT";
        case E_NOT_IMPLEMENTED: return "E_NOT_IMPLEMENTED";

        // oodle
        case E_OODLE_LIBRARY_MISSING: return "E_OODLE_LIBRARY_MISSING";
        case E_OODLE_FAILED_TO_LOAD: return "E_OODLE_FAILED_TO_LOAD";
        case E_OODLE_BAD_SIGNATURE: return "E_OODLE_BAD_SIGNATURE";

        // TAB
        case E_TAB_INVALID_MAGIC: return "E_TAB_INVALID_MAGIC";
        case E_TAB_UNKNOWN_ENTRY: return "E_TAB_UNKNOWN_ENTRY";
        case E_TAB_INPUT_REQUIRES_COMPRESSION_BLOCKS: return "E_TAB_INPUT_REQUIRES_COMPRESSION_BLOCKS";
        case E_TAB_COMPRESS_BLOCK_FAILED: return "E_TAB_COMPRESS_BLOCK_FAILED";
        case E_TAB_DECOMPRESS_BLOCK_FAILED: return "E_TAB_DECOMPRESS_BLOCK_FAILED";

        // AAF
        case E_AAF_INVALID_MAGIC: return "E_AAF_INVALID_MAGIC";
        case E_AAF_INVALID_CHUNK_MAGIC: return "E_AAF_INVALID_CHUNK_MAGIC";
        case E_AAF_COMPRESS_CHUNK_FAILED: return "E_AAF_COMPRESS_CHUNK_FAILED";
        case E_AAF_DECOMPRESS_CHUNK_FAILED: return "E_AAF_DECOMPRESS_CHUNK_FAILED";

        // SARC
        case E_SARC_INVALID_MAGIC: return "E_SARC_INVALID_MAGIC";
        case E_SARC_UNKNOWN_VERSION: return "E_SARC_UNKNOWN_VERSION";
        case E_SARC_PATCHED_ENTRY: return "E_SARC_PATCHED_ENTRY";

        // RBMDL
        case E_RBMDL_INVALID_MAGIC: return "E_RBMDL_INVALID_MAGIC";
        case E_RBMDL_UNKNOWN_VERSION: return "E_RBMDL_UNKNOWN_VERSION";
        case E_RBMDL_BAD_CHECKSUM: return "E_RBMDL_BAD_CHECKSUM";

        // ADF
        case E_ADF_INVALID_MAGIC: return "E_ADF_INVALID_MAGIC";

        // RTPC
        case E_RTPC_INVALID_MAGIC: return "E_RTPC_INVALID_MAGIC";

        // AVTX
        case E_AVTX_INVALID_MAGIC: return "E_AVTX_INVALID_MAGIC";
        case E_AVTX_UNKNOWN_VERSION: return "E_AVTX_UNKNOWN_VERSION";
        case E_AVTX_SOURCE_BUFFER_NEEDED: return "E_AVTX_SOURCE_BUFFER_NEEDED";
    }

    return "UNKNOWN ERROR";
}

#define AVA_FL_SUCCEEDED(x) (x == ava::Result::E_OK)
#define AVA_FL_FAILED(x) (x != ava::Result::E_OK)

#ifndef AVA_FL_ERROR_LOG
#define AVA_FL_ERROR_LOG (void)0
#endif

#define AVA_FL_ENSURE(condition, ret)                                                                                  \
    if (auto result = (condition); result != ava::Result::E_OK) {                                                      \
        AVA_FL_ERROR_LOG("{} : {} [{}] ({}:{})\n", #condition, ResultToString(result), result, __FILE__, __LINE__);    \
        return ret;                                                                                                    \
    }
} // namespace ava
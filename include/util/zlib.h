#pragma once

#include <cstdint>
#include <zlib/zlib.h>
#include <zlib/zutil.h>

namespace ava::zlib
{
/**
 * Reimplementation of zlib compress() function, with MAX_WBITS corrected, so we don't have to override DEF_WBITS.
 * Compresses to raw DEFLATE so we don't have to manually remove GZIP/ZLIB header/checksum.
 */
int32_t Compress(const uint8_t* src, uint32_t src_len, uint8_t* dest, uint32_t* dest_len)
{
    z_stream       stream;
    int32_t        err;
    const uint32_t max = -1;
    uint32_t       left;

    left      = *dest_len;
    *dest_len = 0;

    stream.zalloc = (alloc_func)0;
    stream.zfree  = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);
    if (err != Z_OK)
        return err;

    stream.next_out  = dest;
    stream.avail_out = 0;
    stream.next_in   = (z_const Bytef*)src;
    stream.avail_in  = 0;

    do {
        if (stream.avail_out == 0) {
            stream.avail_out = left > (uLong)max ? max : (uInt)left;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = src_len > (uLong)max ? max : (uInt)src_len;
            src_len -= stream.avail_in;
        }
        err = deflate(&stream, src_len ? Z_NO_FLUSH : Z_FINISH);
    } while (err == Z_OK);

    *dest_len = stream.total_out;
    deflateEnd(&stream);
    return err == Z_STREAM_END ? Z_OK : err;
}

/**
 * Reimplementation of zlib uncompress() function, with MAX_WBITS corrected, so we don't have to override DEF_WBITS.
 */
int32_t Decompress(const uint8_t* src, uint32_t* src_len, uint8_t* dest, uint32_t* dest_len)
{
    z_stream   stream;
    int32_t    err;
    const uInt max = (uInt)-1;
    uint32_t   len, left;
    uint8_t    buf[1]; /* for detection of incomplete stream when *destLen == 0 */

    len = *src_len;
    if (*dest_len) {
        left      = *dest_len;
        *dest_len = 0;
    } else {
        left = 1;
        dest = buf;
    }

    stream.next_in  = (z_const Bytef*)src;
    stream.avail_in = 0;
    stream.zalloc   = (alloc_func)0;
    stream.zfree    = (free_func)0;
    stream.opaque   = (voidpf)0;

    err = inflateInit2(&stream, -MAX_WBITS);
    if (err != Z_OK)
        return err;

    stream.next_out  = dest;
    stream.avail_out = 0;

    do {
        if (stream.avail_out == 0) {
            stream.avail_out = left > (uLong)max ? max : (uInt)left;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = len > (uLong)max ? max : (uInt)len;
            len -= stream.avail_in;
        }
        err = inflate(&stream, Z_NO_FLUSH);
    } while (err == Z_OK);

    *src_len -= len + stream.avail_in;
    if (dest != buf)
        *dest_len = stream.total_out;
    else if (stream.total_out && err == Z_BUF_ERROR)
        left = 1;

    inflateEnd(&stream);
    return err == Z_STREAM_END
               ? Z_OK
               : err == Z_NEED_DICT ? Z_DATA_ERROR : err == Z_BUF_ERROR && left + stream.avail_out ? Z_DATA_ERROR : err;
}
}; // namespace ava::zlib

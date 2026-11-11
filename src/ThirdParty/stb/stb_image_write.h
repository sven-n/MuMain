/* stb_image_write - v1.16 - public domain - http://nothings.org/stb
 *
 * Minimal implementation for MuMain ground truth capture.
 * Only PNG writing is implemented (used by GroundTruthCapture.cpp).
 *
 * USAGE:
 *   In ONE C/C++ file that #includes this file, do this:
 *      #define STB_IMAGE_WRITE_IMPLEMENTATION
 *   before the #include.  That file will build the implementation.
 *   All other files should just #include it without the #define.
 *
 * The functions:
 *   int stbi_write_png(char const *filename, int w, int h, int comp,
 *                      const void *data, int stride_in_bytes);
 *
 * Each function returns 0 on failure and non-zero on success.
 *
 * The data is arranged row-major, from top-to-bottom (i.e., first pixel is
 * top-left), with no padding between rows. The channels per pixel are given by
 * 'comp': 1=Y, 2=YA, 3=RGB, 4=RGBA.
 *
 * PNG creates output files with the same number of components as the input.
 * The PNG encoder requires libz or the built-in simple deflate; this minimal
 * version uses the built-in encoder for portability.
 *
 * CREDITS:
 *   Sean Barrett           -  PNG writing
 *   GitHub users and open  -  bugfixes and improvements
 *
 * LICENSE:
 *   Public domain (www.unlicense.org)
 */

// Note: This header intentionally uses #ifndef/#define guards instead of #pragma once.
// The STB single-header pattern requires the outer #ifndef guard to wrap both the
// declaration section and the #ifdef STB_IMAGE_WRITE_IMPLEMENTATION section. Using
// #pragma once alone would not correctly prevent double-inclusion of declarations while
// allowing the implementation to be compiled once. This is a deliberate exception to
// the project #pragma once standard (AC-STD-1), per Story 4.1.1 code review findings.
#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef STBIW_ASSERT
#include <assert.h>
#define STBIW_ASSERT(x) assert(x)
#endif

#ifndef STBIWDEF
#ifdef STB_IMAGE_WRITE_STATIC
#define STBIWDEF static
#else
#ifdef __cplusplus
#define STBIWDEF extern "C"
#else
#define STBIWDEF extern
#endif
#endif
#endif

STBIWDEF int stbi_write_png(char const* filename, int w, int h, int comp, const void* data,
                            int stride_in_bytes);

#ifdef __cplusplus
}
#endif

#ifdef STB_IMAGE_WRITE_IMPLEMENTATION

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#define STBIW_MSVC
#endif

#ifdef STBIW_MSVC
#pragma warning(push)
#pragma warning(disable : 4996) // _CRT_SECURE_NO_WARNINGS
#endif

static unsigned int stbiw__crc32(unsigned char* buffer, int len)
{
    static unsigned int crc_table[256] = {0};
    if (crc_table[1] == 0)
    {
        for (unsigned int i = 0; i < 256; ++i)
        {
            unsigned int c = i;
            for (int k = 0; k < 8; ++k)
            {
                c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
            }
            crc_table[i] = c;
        }
    }
    unsigned int crc = 0xFFFFFFFF;
    for (int i = 0; i < len; ++i)
    {
        crc = crc_table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

static void stbiw__putc(unsigned char* out, int* pos, unsigned char c)
{
    out[(*pos)++] = c;
}

static void stbiw__write32(unsigned char* out, int* pos, unsigned int val)
{
    out[(*pos)++] = (val >> 24) & 0xFF;
    out[(*pos)++] = (val >> 16) & 0xFF;
    out[(*pos)++] = (val >> 8) & 0xFF;
    out[(*pos)++] = val & 0xFF;
}

static void stbiw__write_chunk(unsigned char* out, int* pos, const char* type,
                               unsigned char* data, int len)
{
    stbiw__write32(out, pos, (unsigned int)len);
    out[(*pos)++] = type[0];
    out[(*pos)++] = type[1];
    out[(*pos)++] = type[2];
    out[(*pos)++] = type[3];
    int type_start = *pos - 4;
    if (data && len > 0)
    {
        memcpy(out + *pos, data, (size_t)len);
        *pos += len;
    }
    // CRC over type + data
    unsigned int crc = stbiw__crc32(out + type_start, 4 + len);
    stbiw__write32(out, pos, crc);
}

// Simple zlib-compatible deflate (uncompressed blocks for simplicity/portability)
static unsigned char* stbiw__zlib_compress(unsigned char* data, int data_len, int* out_len,
                                           int /*quality*/)
{
    // Estimate output size: zlib header(2) + uncompressed blocks + adler32(4)
    // Each uncompressed block: 5 bytes header + up to 65535 bytes data
    int nblocks = (data_len + 65534) / 65535;
    if (nblocks == 0)
    {
        nblocks = 1;
    }
    int buf_size = 2 + nblocks * 5 + data_len + 4;
    unsigned char* out = (unsigned char*)malloc((size_t)buf_size);
    if (!out)
    {
        return nullptr;
    }
    int pos = 0;

    // zlib header: CMF=0x78 (deflate, window size 32K), FLG to make header divisible by 31
    out[pos++] = 0x78;
    out[pos++] = 0x01; // compression level: fastest

    int remaining = data_len;
    int src_pos = 0;

    while (remaining > 0)
    {
        int block_size = remaining < 65535 ? remaining : 65535;
        int is_last = (remaining <= 65535) ? 1 : 0;
        // BFINAL | BTYPE=00 (no compression)
        out[pos++] = (unsigned char)is_last;
        out[pos++] = (unsigned char)(block_size & 0xFF);
        out[pos++] = (unsigned char)((block_size >> 8) & 0xFF);
        out[pos++] = (unsigned char)((~block_size) & 0xFF);
        out[pos++] = (unsigned char)((~block_size >> 8) & 0xFF);
        memcpy(out + pos, data + src_pos, (size_t)block_size);
        pos += block_size;
        src_pos += block_size;
        remaining -= block_size;
    }

    // Adler-32
    unsigned int adler = 1;
    unsigned int s1 = adler & 0xFFFF;
    unsigned int s2 = (adler >> 16) & 0xFFFF;
    for (int i = 0; i < data_len; ++i)
    {
        s1 = (s1 + data[i]) % 65521;
        s2 = (s2 + s1) % 65521;
    }
    adler = (s2 << 16) | s1;
    out[pos++] = (adler >> 24) & 0xFF;
    out[pos++] = (adler >> 16) & 0xFF;
    out[pos++] = (adler >> 8) & 0xFF;
    out[pos++] = adler & 0xFF;

    *out_len = pos;
    return out;
}

int stbi_write_png(char const* filename, int x, int y, int comp, const void* data,
                   int stride_bytes)
{
    if (!filename || !data || x <= 0 || y <= 0 || comp < 1 || comp > 4)
    {
        return 0;
    }

    if (stride_bytes == 0)
    {
        stride_bytes = x * comp;
    }

    // PNG filter type 0 (None) applied to each row
    // Each row: 1 filter byte + comp*x bytes
    int raw_size = y * (1 + x * comp);
    unsigned char* raw = (unsigned char*)malloc((size_t)raw_size);
    if (!raw)
    {
        return 0;
    }

    for (int j = 0; j < y; ++j)
    {
        raw[j * (x * comp + 1)] = 0; // filter type None
        memcpy(raw + j * (x * comp + 1) + 1,
               (unsigned char*)data + j * stride_bytes,
               (size_t)(x * comp));
    }

    int zlib_len = 0;
    unsigned char* zlib_data = stbiw__zlib_compress(raw, raw_size, &zlib_len, 8);
    free(raw);
    if (!zlib_data)
    {
        return 0;
    }

    // Estimate PNG size: sig(8) + IHDR(25) + IDAT(12+zlib_len) + IEND(12)
    int png_size = 8 + 25 + 12 + zlib_len + 12;
    unsigned char* png = (unsigned char*)malloc((size_t)png_size);
    if (!png)
    {
        free(zlib_data);
        return 0;
    }

    int pos = 0;
    // PNG signature
    static const unsigned char sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    memcpy(png + pos, sig, 8);
    pos += 8;

    // IHDR
    unsigned char ihdr[13];
    ihdr[0] = (x >> 24) & 0xFF;
    ihdr[1] = (x >> 16) & 0xFF;
    ihdr[2] = (x >> 8) & 0xFF;
    ihdr[3] = x & 0xFF;
    ihdr[4] = (y >> 24) & 0xFF;
    ihdr[5] = (y >> 16) & 0xFF;
    ihdr[6] = (y >> 8) & 0xFF;
    ihdr[7] = y & 0xFF;
    ihdr[8] = 8; // bit depth
    // color type: 0=gray, 2=RGB, 3=palette, 4=gray+alpha, 6=RGB+alpha
    static const unsigned char color_types[5] = {0, 0, 4, 2, 6};
    ihdr[9] = color_types[comp];
    ihdr[10] = 0; // compression method
    ihdr[11] = 0; // filter method
    ihdr[12] = 0; // interlace method
    stbiw__write_chunk(png, &pos, "IHDR", ihdr, 13);

    // IDAT
    stbiw__write_chunk(png, &pos, "IDAT", zlib_data, zlib_len);
    free(zlib_data);

    // IEND
    stbiw__write_chunk(png, &pos, "IEND", nullptr, 0);

    // Write to file
    FILE* f = fopen(filename, "wb");
    if (!f)
    {
        free(png);
        return 0;
    }
    int written = (int)fwrite(png, 1, (size_t)pos, f);
    fclose(f);
    free(png);
    return written == pos ? 1 : 0;
}

#ifdef STBIW_MSVC
#pragma warning(pop)
#endif

#endif // STB_IMAGE_WRITE_IMPLEMENTATION
#endif // INCLUDE_STB_IMAGE_WRITE_H

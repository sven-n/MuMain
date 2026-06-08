// Non-Windows implementation of the mmio RIFF reader/writer (WinMM.h).
// Backs the WAV loader with buffered stdio file access.
#ifndef _WIN32

#include "Core/Platform/WinMM.h"

#include <cstdio>
#include <cstdlib>  // wcstombs

namespace
{
    struct MuMmio { FILE* fp; };

    FOURCC ReadFourCC(const unsigned char* p)
    {
        return static_cast<FOURCC>(p[0]) | (static_cast<FOURCC>(p[1]) << 8) |
               (static_cast<FOURCC>(p[2]) << 16) | (static_cast<FOURCC>(p[3]) << 24);
    }
}

HMMIO mmioOpenW(wchar_t* szFilename, void* /*lpmmioinfo*/, DWORD dwOpenFlags)
{
    if (!szFilename) return nullptr;

    char path[4096] = { 0 };
    if (std::wcstombs(path, szFilename, sizeof(path) - 1) == static_cast<size_t>(-1)) return nullptr;

    const char* mode = (dwOpenFlags & MMIO_WRITE) ? "wb" : "rb";
    FILE* fp = std::fopen(path, mode);
    if (!fp) return nullptr;

    return static_cast<HMMIO>(new MuMmio{ fp });
}

int mmioClose(HMMIO hmmio, UINT /*wFlags*/)
{
    auto* h = static_cast<MuMmio*>(hmmio);
    if (!h) return 0;
    if (h->fp) std::fclose(h->fp);
    delete h;
    return 0;
}

int mmioRead(HMMIO hmmio, char* pch, int cch)
{
    auto* h = static_cast<MuMmio*>(hmmio);
    if (!h || !h->fp || !pch || cch < 0) return -1;
    return static_cast<int>(std::fread(pch, 1, static_cast<size_t>(cch), h->fp));
}

int mmioWrite(HMMIO hmmio, const char* pch, int cch)
{
    auto* h = static_cast<MuMmio*>(hmmio);
    if (!h || !h->fp || !pch || cch < 0) return -1;
    return static_cast<int>(std::fwrite(pch, 1, static_cast<size_t>(cch), h->fp));
}

// Read the next chunk header (or search for a specific chunk with MMIO_FINDCHUNK
// / MMIO_FINDRIFF), leaving the file positioned at the start of the chunk data.
int mmioDescend(HMMIO hmmio, MMCKINFO* lpck, const MMCKINFO* lpckParent, UINT wFlags)
{
    auto* h = static_cast<MuMmio*>(hmmio);
    if (!h || !h->fp || !lpck) return 1;

    const FOURCC wantId   = lpck->ckid;     // MMIO_FINDCHUNK
    const FOURCC wantType = lpck->fccType;  // MMIO_FINDRIFF

    for (;;)
    {
        const long chunkStart = std::ftell(h->fp);
        if (chunkStart < 0) return 1;

        // Don't search past the end of the parent chunk's data.
        if (lpckParent &&
            static_cast<DWORD>(chunkStart) >= lpckParent->dwDataOffset + lpckParent->cksize)
            return 1;

        unsigned char header[8];
        if (std::fread(header, 1, sizeof(header), h->fp) != sizeof(header)) return 1;  // EOF

        lpck->ckid         = ReadFourCC(header);
        lpck->cksize       = ReadFourCC(header + 4);  // little-endian u32
        lpck->fccType      = 0;
        lpck->dwFlags      = 0;
        lpck->dwDataOffset = static_cast<DWORD>(chunkStart + 8);

        if (lpck->ckid == FOURCC_RIFF || lpck->ckid == FOURCC_LIST)
        {
            unsigned char formType[4];
            if (std::fread(formType, 1, sizeof(formType), h->fp) != sizeof(formType)) return 1;
            lpck->fccType      = ReadFourCC(formType);
            lpck->dwDataOffset = static_cast<DWORD>(chunkStart + 12);
        }

        bool match;
        if (wFlags & MMIO_FINDCHUNK)      match = (lpck->ckid == wantId);
        else if (wFlags & MMIO_FINDRIFF)  match = (lpck->ckid == FOURCC_RIFF && lpck->fccType == wantType);
        else                              match = true;

        if (match)
        {
            std::fseek(h->fp, static_cast<long>(lpck->dwDataOffset), SEEK_SET);
            return 0;
        }

        // Skip to the next chunk; RIFF chunks are word-aligned (pad byte if odd).
        const long next = chunkStart + 8 + static_cast<long>(lpck->cksize) +
                          static_cast<long>(lpck->cksize & 1u);
        if (std::fseek(h->fp, next, SEEK_SET) != 0) return 1;
    }
}

// Seek past the end of the current chunk's data (word-aligned). Used on plain
// data chunks (the fmt chunk), not on the RIFF container.
int mmioAscend(HMMIO hmmio, MMCKINFO* lpck, UINT /*wFlags*/)
{
    auto* h = static_cast<MuMmio*>(hmmio);
    if (!h || !h->fp || !lpck) return 1;

    const long end = static_cast<long>(lpck->dwDataOffset) +
                     static_cast<long>(lpck->cksize) +
                     static_cast<long>(lpck->cksize & 1u);
    return (std::fseek(h->fp, end, SEEK_SET) == 0) ? 0 : 1;
}

#endif // !_WIN32

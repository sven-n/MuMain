// Non-Windows implementation of the mmio RIFF reader/writer (WinMM.h).
// Backs the WAV loader with buffered stdio file access.
#ifndef _WIN32

#include "Core/Platform/WinMM.h"

#include <cstdio>
#include <cstdlib>  // wcstombs
#include <climits>  // LONG_MAX

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

    try
    {
        return static_cast<HMMIO>(new MuMmio{ fp });
    }
    catch (...)  // don't leak the file if the (tiny) allocation throws
    {
        std::fclose(fp);
        return nullptr;
    }
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

    // Search limit: the end of the parent chunk's data, or unbounded at top level.
    // All offset math below is unsigned 64-bit so corrupted chunk sizes can never
    // overflow into a negative seek or skip a chunk backwards.
    const unsigned long long parentEnd =
        lpckParent ? static_cast<unsigned long long>(lpckParent->dwDataOffset) + lpckParent->cksize
                   : ~0ull;

    for (;;)
    {
        const long pos = std::ftell(h->fp);
        if (pos < 0) return 1;
        const unsigned long long start = static_cast<unsigned long long>(pos);

        if (start + 8 > parentEnd) return 1;  // header must fit within the parent

        unsigned char header[8];
        if (std::fread(header, 1, sizeof(header), h->fp) != sizeof(header)) return 1;  // EOF

        lpck->ckid         = ReadFourCC(header);
        lpck->cksize       = ReadFourCC(header + 4);  // little-endian u32
        lpck->fccType      = 0;
        lpck->dwFlags      = 0;
        lpck->dwDataOffset = static_cast<DWORD>(start + 8);

        if (lpck->ckid == FOURCC_RIFF || lpck->ckid == FOURCC_LIST)
        {
            if (start + 12 > parentEnd) return 1;  // form type must fit too
            unsigned char formType[4];
            if (std::fread(formType, 1, sizeof(formType), h->fp) != sizeof(formType)) return 1;
            lpck->fccType      = ReadFourCC(formType);
            lpck->dwDataOffset = static_cast<DWORD>(start + 12);
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
        const unsigned long long aligned = static_cast<unsigned long long>(lpck->cksize) + (lpck->cksize & 1u);
        const unsigned long long next = start + 8 + aligned;  // always advances by >= 8
        if (next > parentEnd) return 1;                                       // runs past parent
        if (next > static_cast<unsigned long long>(LONG_MAX)) return 1;       // beyond seekable range
        if (std::fseek(h->fp, static_cast<long>(next), SEEK_SET) != 0) return 1;
    }
}

// Seek past the end of the current chunk's data (word-aligned). Used on plain
// data chunks (the fmt chunk), not on the RIFF container.
int mmioAscend(HMMIO hmmio, MMCKINFO* lpck, UINT /*wFlags*/)
{
    auto* h = static_cast<MuMmio*>(hmmio);
    if (!h || !h->fp || !lpck) return 1;

    const unsigned long long aligned = static_cast<unsigned long long>(lpck->cksize) + (lpck->cksize & 1u);
    const unsigned long long end = static_cast<unsigned long long>(lpck->dwDataOffset) + aligned;
    if (end > static_cast<unsigned long long>(LONG_MAX)) return 1;
    return (std::fseek(h->fp, static_cast<long>(end), SEEK_SET) == 0) ? 0 : 1;
}

#endif // !_WIN32

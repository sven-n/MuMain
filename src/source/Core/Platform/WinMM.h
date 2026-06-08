// Portable multimedia-IO shim (issue #462, Phase 3).
//
// The WAV file loader (waveIO) uses the Win32 mmio RIFF reader/writer and the
// wave-format structs from <mmsystem.h>/<mmreg.h>. On Windows those headers are
// used unchanged; elsewhere this provides the small subset the loader needs.
#pragma once

#ifdef _WIN32

// mmio + wave-format types come from <mmsystem.h>.

#else  // ---- non-Windows ----------------------------------------------------

#include "Core/Platform/WinCompat.h"  // WORD, DWORD, BYTE

typedef DWORD FOURCC;

// Build a four-character-code from four chars (little-endian, as on disk).
#define mmioFOURCC(c0, c1, c2, c3)                                       \
    ((DWORD)(BYTE)(c0)        | ((DWORD)(BYTE)(c1) << 8) |               \
     ((DWORD)(BYTE)(c2) << 16) | ((DWORD)(BYTE)(c3) << 24))

#ifndef FOURCC_RIFF
#define FOURCC_RIFF mmioFOURCC('R', 'I', 'F', 'F')
#endif
#ifndef FOURCC_LIST
#define FOURCC_LIST mmioFOURCC('L', 'I', 'S', 'T')
#endif

// mmioOpen flags.
#define MMIO_READ      0x00000000
#define MMIO_WRITE     0x00000001
#define MMIO_CREATE    0x00001000
#define MMIO_ALLOCBUF  0x00010000
// mmioDescend search flags.
#define MMIO_FINDCHUNK 0x0010
#define MMIO_FINDRIFF  0x0020

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM 1
#endif

// Wave-format structs map directly onto file bytes, so pack them tightly:
// sizeof(PCMWAVEFORMAT) must be 16 and sizeof(WAVEFORMATEX) 18.
#pragma pack(push, 1)
typedef struct waveformat_tag {
    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
} WAVEFORMAT;
typedef struct pcmwaveformat_tag {
    WAVEFORMAT wf;
    WORD       wBitsPerSample;
} PCMWAVEFORMAT;
typedef struct tWAVEFORMATEX {
    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
    WORD  wBitsPerSample;
    WORD  cbSize;
} WAVEFORMATEX;
#pragma pack(pop)

// In-memory RIFF chunk descriptor (no packing needed).
typedef struct _MMCKINFO {
    FOURCC ckid;          // chunk id
    DWORD  cksize;        // chunk size (bytes after the 8-byte header)
    FOURCC fccType;       // form/list type (for RIFF/LIST chunks)
    DWORD  dwDataOffset;  // file offset of the chunk data
    DWORD  dwFlags;
} MMCKINFO, * LPMMCKINFO;

typedef void* HMMIO;

// mmio subset used by the WAV loader. mmioDescend/mmioAscend return 0 on
// success (MMSYSERR_NOERROR), non-zero on failure; mmioRead/mmioWrite return
// the byte count (-1 on error), as on Windows. Implemented in WinMM.cpp.
HMMIO mmioOpenW(wchar_t* szFilename, void* lpmmioinfo, DWORD dwOpenFlags);
int   mmioClose(HMMIO hmmio, UINT wFlags);
int   mmioRead(HMMIO hmmio, char* pch, int cch);
int   mmioWrite(HMMIO hmmio, const char* pch, int cch);
int   mmioDescend(HMMIO hmmio, MMCKINFO* lpck, const MMCKINFO* lpckParent, UINT wFlags);
int   mmioAscend(HMMIO hmmio, MMCKINFO* lpck, UINT wFlags);

#endif // _WIN32

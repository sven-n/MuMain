#pragma once

#include "Core/Platform/WinCompat.h"
#ifdef _WIN32
#include <mmsystem.h>
#else
// Minimal multimedia-IO types so this header parses off Windows (the DirectSound
// implementation in the .cpp is a Win32 audio subsystem ported later, #462).
typedef void* HMMIO;
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
#endif
#include <cstdint>

class waveIO
{
public:
    enum class Mode : std::uint8_t
    {
        Input = 0,
        Output = 1
    };

    explicit waveIO(Mode mode);
    waveIO();
    explicit waveIO(bool legacyMode);
    ~waveIO();

    waveIO(const waveIO&) = delete;
    waveIO& operator=(const waveIO&) = delete;

    bool LoadWaveHeader(const wchar_t* filename);
    bool WriteWaveHeader(const wchar_t* filename, const PCMWAVEFORMAT& format, int waveDataSize);
    bool CloseWaveFile();

    WAVEFORMATEX GetWaveFormatEx() const { return m_wfex; }
    bool ReadWaveData(char* buffer, int bufferSize);
    bool WriteWaveData(const char* buffer, int bufferSize);
    int GetDataSize() const { return m_DataSize; }

private:
    bool IsInputMode() const noexcept;
    bool IsOutputMode() const noexcept;

    HMMIO        m_hmmio;
    WAVEFORMATEX m_wfex;
    int          m_DataSize;
    int          m_DataLeft;
    int          m_SilentSample;
    Mode         m_mode;
};

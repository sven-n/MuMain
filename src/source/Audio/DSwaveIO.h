#pragma once

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <cstdint>

// waveIO — legacy DirectSound/WinMM wave file I/O.
// This class is Windows-only (WinMM API). It is replaced by miniaudio in EPIC-5.
// Guarded with #ifdef _WIN32 so macOS/Linux builds compile without mmsystem.h types.
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

    WAVEFORMATEX GetWaveFormatEx() const
    {
        return m_wfex;
    }
    bool ReadWaveData(char* buffer, int bufferSize);
    bool WriteWaveData(const char* buffer, int bufferSize);
    int GetDataSize() const
    {
        return m_DataSize;
    }

private:
    bool IsInputMode() const noexcept;
    bool IsOutputMode() const noexcept;

    HMMIO m_hmmio;
    WAVEFORMATEX m_wfex;
    int m_DataSize;
    int m_DataLeft;
    int m_SilentSample;
    Mode m_mode;
};

#endif // _WIN32

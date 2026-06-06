#pragma once

#include <windows.h>
#include <mmsystem.h>
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

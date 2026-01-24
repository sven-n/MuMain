#include "stdafx.h"
#include "DSwaveIO.h"

#include <cwchar>
#include <cstring>

#include <windows.h>
#include <mmsystem.h>

// Undefine the editor redirect macro so we can use std::fwprintf directly
// to log to both stderr and the in-game editor console.
#ifdef _EDITOR
#undef fwprintf
extern "C" int editor_fwprintf(FILE* stream, const wchar_t* format, ...);
#endif

namespace
{
constexpr int kWaveHeaderSize = 16;
constexpr int kSilentSample8Bit = 128;

void ReportWaveWarning(const wchar_t* message, const wchar_t* filename = nullptr)
{
    if (filename != nullptr)
    {
        (void)std::fwprintf(stderr, L"%ls (%ls)\n", message, filename);
#ifdef _EDITOR
        editor_fwprintf(stderr, L"%ls (%ls)\n", message, filename);
#endif
    }
    else
    {
        (void)std::fwprintf(stderr, L"%ls\n", message);
#ifdef _EDITOR
        editor_fwprintf(stderr, L"%ls\n", message);
#endif
    }
}
} // namespace

waveIO::waveIO(Mode mode)
    : m_hmmio(nullptr)
    , m_wfex{}
    , m_DataSize(0)
    , m_DataLeft(0)
    , m_SilentSample(0)
    , m_mode(mode)
{
}

waveIO::waveIO()
    : waveIO(Mode::Input)
{
}

waveIO::waveIO(bool legacyMode)
    : waveIO(legacyMode ? Mode::Output : Mode::Input)
{
}

waveIO::~waveIO()
{
    CloseWaveFile();
}

bool waveIO::CloseWaveFile()
{
    if (m_hmmio != nullptr)
    {
        mmioClose(m_hmmio, 0);
        m_hmmio = nullptr;
    }
    return true;
}

bool waveIO::IsInputMode() const noexcept
{
    return m_mode == Mode::Input;
}

bool waveIO::IsOutputMode() const noexcept
{
    return m_mode == Mode::Output;
}

bool waveIO::LoadWaveHeader(const wchar_t* filename)
{
    if (filename == nullptr || !IsInputMode())
    {
        return false;
    }

    CloseWaveFile();

    m_hmmio = mmioOpenW(const_cast<wchar_t*>(filename), nullptr, MMIO_ALLOCBUF | MMIO_READ);
    if (m_hmmio == nullptr)
    {
        ReportWaveWarning(L"Cannot find wave file", filename);
        return false;
    }

    MMCKINFO riffChunk {};
    if (mmioDescend(m_hmmio, &riffChunk, nullptr, 0) != 0)
    {
        ReportWaveWarning(L"Cannot descend into RIFF chunk", filename);
        CloseWaveFile();
        return false;
    }

    if (riffChunk.ckid != FOURCC_RIFF || riffChunk.fccType != mmioFOURCC('W', 'A', 'V', 'E'))
    {
        ReportWaveWarning(L"File is not a valid WAVE container", filename);
        CloseWaveFile();
        return false;
    }

    MMCKINFO formatChunk {};
    formatChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(m_hmmio, &formatChunk, &riffChunk, MMIO_FINDCHUNK) != 0)
    {
        ReportWaveWarning(L"Failed to locate fmt chunk", filename);
        CloseWaveFile();
        return false;
    }

    if (mmioRead(m_hmmio, reinterpret_cast<char*>(&m_wfex), sizeof(PCMWAVEFORMAT)) == -1)
    {
        ReportWaveWarning(L"Failed to read wave format", filename);
        CloseWaveFile();
        return false;
    }

    if (m_wfex.wBitsPerSample == 8)
    {
        m_SilentSample = kSilentSample8Bit;
    }

    if (m_wfex.wFormatTag != WAVE_FORMAT_PCM)
    {
        ReportWaveWarning(L"Unsupported wave format (expecting PCM)", filename);
        CloseWaveFile();
        return false;
    }

    if (mmioAscend(m_hmmio, &formatChunk, 0) != 0)
    {
        ReportWaveWarning(L"Failed to ascend fmt chunk", filename);
        CloseWaveFile();
        return false;
    }

    MMCKINFO dataChunk {};
    dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(m_hmmio, &dataChunk, &riffChunk, MMIO_FINDCHUNK) != 0)
    {
        MessageBoxW(nullptr, L"Bad Format in Wave file!", L"WaveLoad", MB_OK | MB_ICONSTOP);
        CloseWaveFile();
        return false;
    }

    m_DataSize = static_cast<int>(dataChunk.cksize);
    m_DataLeft = m_DataSize;
    return true;
}

bool waveIO::ReadWaveData(char* buffer, int bufferSize)
{
    if (!IsInputMode() || buffer == nullptr || bufferSize <= 0 || m_hmmio == nullptr)
    {
        return false;
    }

    int bytesToRead = bufferSize;
    if (m_DataLeft < bufferSize)
    {
        bytesToRead = m_DataLeft;
        std::memset(buffer, 0, static_cast<std::size_t>(bufferSize));
    }

    const int readResult = mmioRead(m_hmmio, buffer, bytesToRead);
    if (readResult < bytesToRead)
    {
        ReportWaveWarning(L"Failed to read expected number of bytes");
        CloseWaveFile();
        return false;
    }

    m_DataLeft -= bytesToRead;
    return true;
}

bool waveIO::WriteWaveData(const char* buffer, int bufferSize)
{
    if (!IsOutputMode() || buffer == nullptr || bufferSize <= 0 || m_hmmio == nullptr)
    {
        return false;
    }

    const int result = mmioWrite(m_hmmio, const_cast<char*>(buffer), bufferSize);
    if (result != bufferSize)
    {
        ReportWaveWarning(L"Failed to write audio data");
        CloseWaveFile();
        return false;
    }
    return true;
}

bool waveIO::WriteWaveHeader(const wchar_t* filename, const PCMWAVEFORMAT& format, int waveDataSize)
{
    if (filename == nullptr || !IsOutputMode())
    {
        return false;
    }

    CloseWaveFile();

    m_hmmio = mmioOpenW(const_cast<wchar_t*>(filename), nullptr, MMIO_CREATE | MMIO_WRITE);
    if (m_hmmio == nullptr)
    {
        ReportWaveWarning(L"Failed to create output wave file", filename);
        return false;
    }

    const int riffSize = 12 + sizeof(PCMWAVEFORMAT) + 8 + waveDataSize;

    if (mmioWrite(m_hmmio, "RIFF", 4) != 4 ||
        mmioWrite(m_hmmio, reinterpret_cast<const char*>(&riffSize), 4) != 4 ||
        mmioWrite(m_hmmio, "WAVE", 4) != 4)
    {
        ReportWaveWarning(L"Failed to write RIFF header", filename);
        CloseWaveFile();
        return false;
    }

    if (mmioWrite(m_hmmio, "fmt ", 4) != 4 ||
        mmioWrite(m_hmmio, reinterpret_cast<const char*>(&kWaveHeaderSize), 4) != 4 ||
        mmioWrite(m_hmmio, reinterpret_cast<const char*>(&format), sizeof(format)) != sizeof(format))
    {
        ReportWaveWarning(L"Failed to write fmt chunk", filename);
        CloseWaveFile();
        return false;
    }

    if (mmioWrite(m_hmmio, "data", 4) != 4 ||
        mmioWrite(m_hmmio, reinterpret_cast<const char*>(&waveDataSize), 4) != 4)
    {
        ReportWaveWarning(L"Failed to write data chunk header", filename);
        CloseWaveFile();
        return false;
    }

    m_wfex = *reinterpret_cast<const WAVEFORMATEX*>(&format);
    m_DataSize = waveDataSize;
    m_DataLeft = waveDataSize;
    return true;
}
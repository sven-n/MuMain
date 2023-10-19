#include "stdafx.h"
#include "DSwaveIO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

waveIO::waveIO(bool IO)
{
    m_IO = IO;
    m_hmmio = nullptr;
    m_SilentSample = 0;  // Default assumes we are dealing with 16bit samples
}

waveIO::~waveIO()
{
    CloseWaveFile();
}

bool waveIO::CloseWaveFile()
{
    if (m_hmmio)
        mmioClose(m_hmmio, 0);

    return true;
}

bool waveIO::LoadWaveHeader(wchar_t* szFilename)
{
    MMCKINFO		ckOutRIFF;          // chunk info. for output RIFF chunk
    MMCKINFO		ckOut;              // info. for a chunk in output file

    if (m_IO != INPUT)
        return false;

    // Open the wave file for reading using buffered I/O.
    m_hmmio = mmioOpen(szFilename, nullptr, MMIO_ALLOCBUF | MMIO_READ);
    if (m_hmmio == nullptr)
    {
        wprintf(L"Cannot find file %s\n", szFilename);
        return false;
    }

    // Descend the file into the 'RIFF' chunk.
    if (mmioDescend(m_hmmio, &ckOutRIFF, nullptr, 0) != 0)
    {
        mmioClose(m_hmmio, 0);
        wprintf(L"Cannot read file!\n");
        return false;
    }

    // Make sure the file is a WAVE file.
    if ((ckOutRIFF.ckid != FOURCC_RIFF) ||
        (ckOutRIFF.fccType != mmioFOURCC('W', 'A', 'V', 'E')))
    {
        mmioClose(m_hmmio, 0);
        wprintf(L"Bad Wave Format!\n");
        return false;
    }

    // Search the file for for the 'fmt ' chunk.
    ckOut.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(m_hmmio, &ckOut, &ckOutRIFF, MMIO_FINDCHUNK) != 0)
    {
        mmioClose(m_hmmio, 0);
        wprintf(L"Bad Riff Format!\n");
        return false;
    }

    // Read PCM wave format data from file into WAVEFORMATEX struct.
    // NOTE: Read ONLY sizeof(PCMWAVEFORMAT) into wf
    if (mmioRead(m_hmmio, (char*)&m_wfex, sizeof(PCMWAVEFORMAT)) == -1)
    {
        mmioClose(m_hmmio, 0);
        wprintf(L"Bad Riff Format!\n");
        return false;
    }

    if (m_wfex.wBitsPerSample == 8)
    {
        m_SilentSample = 128;
    }

    // Check it is PCM not some compressed format which we do not cope with..
    if (m_wfex.wFormatTag != WAVE_FORMAT_PCM)
    {
        mmioClose(m_hmmio, 0);
        wprintf(L"UnSupported Wave Format!\n");
        return false;
    }

    // Ascend the file out of the 'fmt ' chunk.
    if (mmioAscend(m_hmmio, &ckOut, 0) != 0)
    {
        mmioClose(m_hmmio, 0);
        wprintf(L"Cannot read file!\n");
        return false;
    }

    // Search the file for the 'data' chunk.
    ckOut.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(m_hmmio, &ckOut, &ckOutRIFF, MMIO_FINDCHUNK) != 0)
    {
        mmioClose(m_hmmio, 0);
        MessageBox(nullptr, L"Bad Format in Wave file!", L"WaveLoad", MB_OK | MB_ICONSTOP);
        return FALSE;
    }

    m_DataSize = ckOut.cksize;
    m_DataLeft = m_DataSize;

    return true;
}

bool waveIO::ReadWaveData(char* buffer, int nSizeOfBuffer)
{
    int nBytesToRead;
    int hResult;

    if (m_IO != INPUT)
        return false;

    // Read in nSizeOfBuffer bytes from the current file pointer location
    // and store them in buffer (checking for possible end of file)
    if (m_DataLeft < nSizeOfBuffer)
    {
        nBytesToRead = m_DataLeft;
        // Because our data won't fill up the buffer, lets first fill it with silence
        memset(buffer, 0, nSizeOfBuffer);
    }
    else
        nBytesToRead = nSizeOfBuffer;

    hResult = mmioRead(m_hmmio, buffer, nBytesToRead);
    if (hResult < nBytesToRead)
    {
        wprintf(L"hResult is %d", hResult);
        wprintf(L"Failed to read audio data\n");
        mmioClose(m_hmmio, 0);
        return false;
    }
    else
    {
        m_DataLeft = (m_DataLeft - nBytesToRead);
    }

    return true;
}

bool waveIO::WriteWaveData(char* buffer, int nSizeOfBuffer)
{
    int result;

    if (m_IO != OUTPUT)
        return false;

    // Write nSizeOfBuffer bytes from buffer to our resulting wavefile
    result = mmioWrite(m_hmmio, buffer, nSizeOfBuffer);
    if (result != nSizeOfBuffer)
    {
        wprintf(L"Failed to write audio data\n");
        mmioClose(m_hmmio, 0);
        return false;
    }
    return true;
}

bool waveIO::WriteWaveHeader(wchar_t* szFilename, PCMWAVEFORMAT wf, int nWaveDataSize)
{
    int	nRIFFSize, nFormatSize, result;

    if (m_IO != OUTPUT)
        return false;

    m_hmmio = mmioOpen(szFilename, nullptr, MMIO_CREATE | MMIO_WRITE);

    if (m_hmmio == nullptr)
    {
        wprintf(L"Failed to create output file\n");
        return false;
    }

    nFormatSize = 16;
    nRIFFSize = 12 + sizeof(PCMWAVEFORMAT) + 8 + nWaveDataSize;

    if ((result = mmioWrite(m_hmmio, "RIFF", 4)) != 4)
    {
        wprintf(L"Failed to write RIFF\n");
        mmioClose(m_hmmio, 0);
        return false;
    }

    if ((result = mmioWrite(m_hmmio, (char*)&nRIFFSize, 4)) != 4)
    {
        wprintf(L"Failed to write RIFF size\n");
        mmioClose(m_hmmio, 0);
        return false;
    }

    if ((result = mmioWrite(m_hmmio, "WAVE", 4)) != 4)
    {
        wprintf(L"Failed to write WAVE\n");
        mmioClose(m_hmmio, 0);
        return false;
    }

    if ((result = mmioWrite(m_hmmio, "fmt ", 4)) != 4)
    {
        wprintf(L"Failed to write fmt \n");
        mmioClose(m_hmmio, 0);
        return false;
    }

    if ((result = mmioWrite(m_hmmio, (char*)&nFormatSize, 4)) != 4)
    {
        wprintf(L"Failed to write fmt size\n");
        mmioClose(m_hmmio, 0);
        return false;
    }

    if ((result = mmioWrite(m_hmmio, (char*)&wf, sizeof(wf))) != sizeof(wf))
    {
        wprintf(L"Failed to write Wave Format header\n");
        mmioClose(m_hmmio, 0);
        return false;
    }

    if ((result = mmioWrite(m_hmmio, "data", 4)) != 4)
    {
        wprintf(L"Failed to write data\n");
        mmioClose(m_hmmio, 0);
        return false;
    }

    if ((result = mmioWrite(m_hmmio, (char*)&nWaveDataSize, 4)) != 4)
    {
        wprintf(L"Failed to write data size\n");
        mmioClose(m_hmmio, 0);
        return false;
    }

    return true;
}
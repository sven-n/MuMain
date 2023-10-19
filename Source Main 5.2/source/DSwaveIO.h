// waveIO.h: interface for the waveIO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVEIO_H__A0449D63_0097_11D4_A092_A64970C5F176__INCLUDED_)
#define AFX_WAVEIO_H__A0449D63_0097_11D4_A092_A64970C5F176__INCLUDED_

#pragma once

#define INPUT	0
#define OUTPUT	1

class waveIO
{
public:
    waveIO(bool IO);
    virtual ~waveIO();
    bool LoadWaveHeader(wchar_t* szFilename);
    bool WriteWaveHeader(wchar_t* szFilename, PCMWAVEFORMAT wf, int nWaveDateSize);
    bool CloseWaveFile();
    WAVEFORMATEX GetWaveFormatEx() { return m_wfex; }
    bool ReadWaveData(char* buffer, int nSizeOfBuffer);
    bool WriteWaveData(char* buffer, int nSizeOfBuffer);
    int GetDataSize() { return m_DataSize; }
private:
    HMMIO			m_hmmio;	// Handle to WAVE file
    WAVEFORMATEX	m_wfex;		// WAVEFORMATEX structure
    int				m_DataSize;	// Stores size of wave data
    int				m_DataLeft;
    int				m_SilentSample;
    bool			m_IO;
};

#endif // !defined(AFX_WAVEIO_H__A0449D63_0097_11D4_A092_A64970C5F176__INCLUDED_)

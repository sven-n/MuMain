#pragma once

#define MAX_LENGTH_CPUNAME	( 128)
#define MAX_LENGTH_OSINFO	( 128)
#define MAX_DXVERSION		( 128)

typedef struct
{
    wchar_t m_lpszCPU[MAX_LENGTH_CPUNAME];
    wchar_t m_lpszOS[MAX_LENGTH_OSINFO];
    int m_iMemorySize;

    wchar_t m_lpszDxVersion[MAX_DXVERSION];
} ER_SystemInfo;

class CErrorReport
{
public:
    CErrorReport();
    virtual ~CErrorReport();

    void Clear(void);

protected:
    HANDLE m_hFile;
    wchar_t m_lpszFileName[MAX_PATH];
    int m_iKey;
public:
    void Create(wchar_t* lpszFileName);
    void Destroy(void);
protected:
    void CutHead(void);
    wchar_t* CheckHeadToCut(wchar_t* lpszBuffer, DWORD dwNumber);

protected:
    BOOL WriteFile(HANDLE hFile, void* lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
public:
    void WriteDebugInfoStr(wchar_t* lpszToWrite);
    void Write(const wchar_t* lpszFormat, ...);
    void HexWrite(void* pBuffer, int iSize);

    void AddSeparator(void);
    void WriteLogBegin(void);
    void WriteCurrentTime(BOOL bLineShift = TRUE);

    void WriteSystemInfo(ER_SystemInfo* si);
    void WriteOpenGLInfo(void);
    void WriteImeInfo(HWND hWnd);
    void WriteSoundCardInfo(void);
};

extern CErrorReport g_ErrorReport;

void GetSystemInfo(ER_SystemInfo* si);


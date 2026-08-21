#pragma once

#include <filesystem>
#include <fstream>
#include <cstdint>

#define MAX_LENGTH_CPUNAME	( 128)
#define MAX_LENGTH_OSINFO	( 128)
#define MAX_GPU_BACKEND_LEN	( 128)

struct SDL_Window;

typedef struct
{
    wchar_t m_lpszCPU[MAX_LENGTH_CPUNAME];
    wchar_t m_lpszOS[MAX_LENGTH_OSINFO];
    std::int64_t m_iMemorySize;

    wchar_t m_lpszGpuBackend[MAX_GPU_BACKEND_LEN];
} ER_SystemInfo;

class CErrorReport
{
public:
    CErrorReport();
    virtual ~CErrorReport();

    void Clear(void);

protected:
    std::ofstream m_fileStream;
    std::filesystem::path m_filePath;
    int m_iKey;
public:
    void Create(const wchar_t* lpszFileName);
    void Destroy(void);
protected:
    void CutHead(void);
public:
    void WriteDebugInfoStr(wchar_t* lpszToWrite);
    void Write(const wchar_t* lpszFormat, ...);
    void HexWrite(void* pBuffer, int iSize);

    void AddSeparator(void);
    void WriteLogBegin(void);
    void WriteCurrentTime(BOOL bLineShift = TRUE);

    void WriteSystemInfo(ER_SystemInfo* si);
    void WriteOpenGLInfo(void);
    void WriteFontInfo(void);
    void WriteImeInfo(SDL_Window* window);
    void WriteSoundCardInfo(void);
};

extern CErrorReport g_ErrorReport;

void MuGetSystemInfo(ER_SystemInfo* si);

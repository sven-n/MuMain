#pragma once

#include <filesystem>
#include <fstream>

#define MAX_LENGTH_CPUNAME (128)
#define MAX_LENGTH_OSINFO (128)
#define MAX_DXVERSION (128)

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

#ifdef _WIN32
    void WriteSystemInfo(ER_SystemInfo* si);
    void WriteOpenGLInfo(void);
    void WriteImeInfo(HWND hWnd);
    void WriteSoundCardInfo(void);
#endif
};

extern CErrorReport g_ErrorReport;

// Async-signal-safe file descriptor for crash handler writes.
// Set in CErrorReport::Create() after m_fileStream.open().
// Used by POSIX signal handlers (PosixSignalHandlers.cpp) to write
// crash diagnostics directly via write() — the only async-signal-safe
// way to output to MuError.log from a signal handler.
// [VS0-QUAL-SIGNAL-HANDLERS]
extern volatile int g_errorReportFd;

// Stringify helper: LOG_CALL(OpenMonsterModel, MONSTER_MODEL_DRAGON)
// writes "OpenMonsterModel(MONSTER_MODEL_DRAGON)" to MuError.log,
// then calls OpenMonsterModel(MONSTER_MODEL_DRAGON).
// Falls back to numeric output for computed arguments — use the
// direct function call with manual logging in those rare cases.
// Note: L#x does not work on Clang — use a two-step widen macro instead.
#define MU_WIDEN_IMPL(x) L##x
#define MU_WIDEN(x) MU_WIDEN_IMPL(x)
#define MU_STRINGIFY(x) #x
#define LOG_CALL(func, arg)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        g_ErrorReport.Write(L"%ls(%ls)\r\n", MU_WIDEN(MU_STRINGIFY(func)), MU_WIDEN(MU_STRINGIFY(arg)));                \
        func(arg);                                                                                                     \
    } while (0)

#ifdef _WIN32
void GetSystemInfo(ER_SystemInfo* si);
#endif

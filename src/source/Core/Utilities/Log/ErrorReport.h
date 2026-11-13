#pragma once

struct SDL_Window;

class CErrorReport
{
public:
    CErrorReport() = default;
    virtual ~CErrorReport() = default;

    void WriteDebugInfoStr(wchar_t* lpszToWrite);
    void Write(const wchar_t* lpszFormat, ...);
    void HexWrite(void* pBuffer, int iSize);

    void AddSeparator(void);
    void WriteLogBegin(void);
    void WriteCurrentTime(BOOL bLineShift = TRUE);
    void WriteBuildInfo(void);

    void WriteSystemInfo(void);
    void WriteGpuInfo(const char* deviceName, const char* driverName, const char* driverVersion);
    void WriteFontInfo(void);
    void WriteImeInfo(SDL_Window* window);
    void WriteSoundCardInfo(void);
};

extern CErrorReport g_ErrorReport;

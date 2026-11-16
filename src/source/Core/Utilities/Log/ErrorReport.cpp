// ErrorReport.cpp: implementation of the CErrorReport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "ErrorReport.h"
#include "Core/Platform/Audio/AudioDeviceNames.h"
#include "Core/Utilities/BuildInfo.h"
#include "Core/Utilities/PlatformInfo.h"
#include "Core/Utilities/VersionFiles.h"
#include "MuGitCommit.h"

#include <string>
#include <thread>

CErrorReport g_ErrorReport;

namespace
{
const char* OrUnknown(const char* text)
{
    return (text != nullptr && *text != '\0') ? text : "unknown";
}

// When the executable was linked. __DATE__ and __TIME__ are only when this file was compiled, which an incremental
// build skips while the commit and its "local changes" mark stay the same; they are the fallback.
std::string DescribeBuildTime()
{
    const std::string written = Core::Build::DescribeFileTime(Core::Platform::GetExecutablePath());
    if (!written.empty())
        return "built " + written;
    return std::string("compiled ") + Core::Build::kDate + " " + Core::Build::kTime;
}

std::string WideToUtf8(const wchar_t* value)
{
    if (value == nullptr)
    {
        return {};
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1)
    {
        return {};
    }

    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value, -1, result.data(), size, nullptr, nullptr);
    result.pop_back();
    return result;
}
}

void CErrorReport::WriteDebugInfoStr(wchar_t* lpszToWrite)
{
    std::string line = WideToUtf8(lpszToWrite);
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
        line.pop_back();
    if (line.empty())
        return;
    // The report is the client's progress trail, not a list of errors; flushed per line so a crash keeps its tail.
    const auto logger = mu::log::Get("core");
    MU_LOG_INFO(logger, "{}", line);
    logger->flush();
}

void CErrorReport::Write(const wchar_t* lpszFormat, ...)
{
    wchar_t lpszBuffer[1024] = { 0, };
    va_list va;
    va_start(va, lpszFormat);
    vswprintf(lpszBuffer, 1024, lpszFormat, va);
    va_end(va);

    WriteDebugInfoStr(lpszBuffer);
}

void CErrorReport::HexWrite(void* pBuffer, int iSize)
{
    wchar_t szLine[256] = { 0, };
    int offset = 0;
    offset += mu_swprintf(szLine, L"0x%00000008X : ", (DWORD*)pBuffer);
    for (int i = 0; i < iSize; i++) {
        offset += mu_swprintf(szLine + offset, L"%02X", *((BYTE*)pBuffer + i));
        if (i > 0 && i < iSize - 1) {
            if (i % 16 == 15) {	//. new line
                offset += mu_swprintf(szLine + offset, L"\r\n");
                WriteDebugInfoStr(szLine);
                offset = 0;
                offset += mu_swprintf(szLine + offset, L"           : ");
            }
            else if (i % 4 == 3) { //. space
                offset += mu_swprintf(szLine + offset, L" ");
            }
        }
    }
    offset += mu_swprintf(szLine + offset, L"\r\n");
    WriteDebugInfoStr(szLine);
}

void CErrorReport::AddSeparator(void)
{
    Write(L"-------------------------------------------------------------------------------------\r\n");
}

void CErrorReport::WriteLogBegin(void)
{
    Write(L"###### Log Begin ######\r\n");
}

void CErrorReport::WriteCurrentTime(BOOL bLineShift)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    g_ErrorReport.Write(L"%4d/%02d/%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
    if (bLineShift)
    {
        g_ErrorReport.Write(L"\r\n");
    }
}

// Which client a report came from: the release download, the commit it was built from (so an old build is easy
// to spot), how it was built, and whether the installed data belongs to it.
void CErrorReport::WriteBuildInfo(void)
{
    const std::string release =
        Core::Build::DescribeRelease(Core::Build::ReadVersionFile(Core::Build::kReleaseVersionFile));
    Write(L"Release \t\t: %hs\r\n", release.c_str());

    const bool hasLocalChanges = MU_GIT_COMMIT_CHANGED != 0;
    Write(L"Build commit \t\t: %hs%ls\r\n", MU_GIT_COMMIT, hasLocalChanges ? L" (with local changes)" : L"");
    Write(L"Build \t\t\t: %hs %hs %hs %hs, %hs\r\n", Core::Build::kConfiguration, Core::Build::kEditor,
          Core::Build::kCompiler, Core::Build::kArchitecture, DescribeBuildTime().c_str());

    const std::string data =
        Core::Build::DescribeData(Core::Build::ReadVersionFile(Core::Build::kDataVersionFile), MU_DATA_ASSET_ID);
    Write(L"Data \t\t\t: %hs\r\n", data.c_str());
}

void CErrorReport::WriteSystemInfo(void)
{
    Write(L"<System information>\r\n");
    Write(L"OS \t\t\t: %ls\r\n", Core::Platform::GetOSVersionDetails().c_str());
    Write(L"CPU \t\t\t: %hs (%u threads)\r\n", OrUnknown(Core::Platform::GetProcessorName().c_str()),
          std::thread::hardware_concurrency());
    Write(L"RAM \t\t\t: %d MB\r\n", Core::Platform::GetSystemMemoryMB());
}

// Once the renderer is up: the GPU, the graphics API SDL picked, and the driver version where the API reports
// one (Vulkan and Direct3D 12 do, Metal doesn't).
void CErrorReport::WriteGpuInfo(const char* deviceName, const char* driverName, const char* driverVersion)
{
    Write(L"GPU \t\t\t: %hs\r\n", OrUnknown(deviceName));
    Write(L"GPU API \t\t: %hs\r\n", OrUnknown(driverName));
    Write(L"GPU driver \t\t: %hs\r\n", OrUnknown(driverVersion));
}

void CErrorReport::WriteFontInfo(void)
{
    Write(L"<UI font>\r\n");
#ifdef _WIN32
    Write(L"Source\t\t: Win32 GDI (system fonts)\r\n");
#else
    // On non-Windows the font is discovered at runtime (fontconfig + fallbacks);
    // log what was found so a "no UI text" report is diagnosable. Paths are
    // ASCII, written through the %hs narrow conversion.
    const std::string diag = MuFontDiagnostics();
    if (diag.empty())
    {
        Write(L"(no font resolved)\r\n");
    }
    else
    {
        size_t pos = 0;
        while (pos < diag.size())
        {
            const size_t nl = diag.find('\n', pos);
            const std::string line = diag.substr(pos, (nl == std::string::npos ? diag.size() : nl) - pos);
            pos = (nl == std::string::npos) ? diag.size() : nl + 1;
            if (!line.empty())
                Write(L"%hs\r\n", line.c_str());
        }
        if (diag.find("NOT FOUND") != std::string::npos)
            Write(L"!! UI text is disabled - no usable font found. Install a "
                  L"sans-serif font or set MU_FONT.\r\n");
    }
#endif
}

// ---- Win32 crash-report details -----------------------------------------------
// IME and sound-card details for the crash log. The Windows versions use Win32
// types, so they are split from the other platforms' (issue #462).
#ifdef _WIN32

void CErrorReport::WriteImeInfo(SDL_Window* /*window*/)
{
    Write(L"<Text input information>\r\n");
    Write(L"Backend\t\t: SDL3\r\n");
}

typedef struct tagER_SOUNDDEVICE {
    wchar_t	szGuid[64];
    wchar_t	szDeviceName[128];
    wchar_t	szDriverName[128];
} ER_SOUNDDEVICEINFO;

typedef struct tagSOUNDDEVICEENUM {
    enum { MAX_DEVICENUM = 20 };
    tagSOUNDDEVICEENUM() { nDeivceCount = 0; }
    ER_SOUNDDEVICEINFO		infoSoundDevice[MAX_DEVICENUM];
    size_t				nDeivceCount;

    tagER_SOUNDDEVICE& operator [] (size_t p) {
        return infoSoundDevice[p];
    }
    tagER_SOUNDDEVICE& GetNextDevice() {
        return infoSoundDevice[nDeivceCount];
    }
} ER_SOUNDDEVICEENUMINFO;

INT_PTR CALLBACK DSoundEnumCallback(GUID* pGUID, LPWSTR strDesc, LPWSTR strDrvName, VOID* pContext)
{
    if (pGUID) {
        auto* pSoundDeviceEnumInfo = (ER_SOUNDDEVICEENUMINFO*)pContext;
        wcscpy(pSoundDeviceEnumInfo->GetNextDevice().szDeviceName, strDesc);
        wcscpy(pSoundDeviceEnumInfo->GetNextDevice().szDriverName, strDrvName);
        pSoundDeviceEnumInfo->nDeivceCount++;
    }
    return TRUE;
}

BOOL GetFileVersion(wchar_t* lpszFileName, WORD* pwVersion);

void CErrorReport::WriteSoundCardInfo(void)
{
    Write(L"<Sound device information>\r\n");
    const auto names = mu::GetAudioDeviceNames();
    if (names.empty())
    {
        Write(L"No playback device found.\r\n");
        return;
    }
    for (const auto& name : names) Write(L"Description \t\t: %hs\r\n", name.c_str());
}

#else  // ---- non-Windows ----------------------------------------------------

// IME is a Windows input service; there is nothing to report elsewhere.
void CErrorReport::WriteImeInfo(SDL_Window* /*window*/)
{
    Write(L"<Text input information>\r\n");
    Write(L"Backend\t\t: SDL3\r\n");
}

// Audio runs through SDL; the DirectSound device enumeration has no equivalent.
void CErrorReport::WriteSoundCardInfo(void)
{
    Write(L"<Sound device information>\r\n");
    const auto names = mu::GetAudioDeviceNames();
    if (names.empty())
    {
        Write(L"No playback device found.\r\n");
        return;
    }
    for (const auto& name : names) Write(L"Description \t\t: %hs\r\n", name.c_str());
}

#endif // _WIN32 (Win32 crash-report details)

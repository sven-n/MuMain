// ErrorReport.cpp: implementation of the CErrorReport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <sys/utsname.h>

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#endif

#include <SDL3/SDL.h>

#include "ErrorReport.h"

// Global error report instance — defined here so MUCore-linked binaries (e.g. MuTests) can resolve it.
// Declared as extern in ErrorReport.h; used from Winmain.cpp and MuTimer.cpp.
CErrorReport g_ErrorReport;

// Async-signal-safe file descriptor for crash handler writes.
// Initialized to -1 (disabled). Set in CErrorReport::Create() after m_fileStream.open().
// Used by POSIX signal handlers (PosixSignalHandlers.cpp) via write(g_errorReportFd, ...).
// [VS0-QUAL-SIGNAL-HANDLERS]
volatile int g_errorReportFd = -1;

void DeleteSocket();

// ---------------------------------------------------------------------------
// Internal helper: BMP-only wide-to-UTF-8 conversion.
// Avoids deprecated std::wstring_convert. All MU Online text is BMP (≤ U+FFFF)
// per development-standards.md §1, so 3-byte UTF-8 maximum is correct.
// Pattern from Story 1.2.1 (mu_wfopen shim in PlatformCompat.h).
// ---------------------------------------------------------------------------
static std::string WideToUtf8(const wchar_t* wide)
{
    std::string result;
    while (wide && *wide)
    {
        wchar_t ch = *wide++;
        if (ch < 0x80)
        {
            result += static_cast<char>(ch);
        }
        else if (ch < 0x800)
        {
            result += static_cast<char>(0xC0 | (ch >> 6));
            result += static_cast<char>(0x80 | (ch & 0x3F));
        }
        else
        {
            // Skip UTF-16 surrogate codepoints (U+D800-U+DFFF) — invalid in UTF-8
            if (ch >= 0xD800 && ch <= 0xDFFF)
            {
                continue;
            }
            result += static_cast<char>(0xE0 | (ch >> 12));
            result += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (ch & 0x3F));
        }
    }
    return result;
}

CErrorReport::CErrorReport()
{
    Clear();
    Create(L"MuError.log");
}

CErrorReport::~CErrorReport()
{
    Destroy();
}

void CErrorReport::Clear(void)
{
    m_filePath.clear();
    m_iKey = 0;
}

void CErrorReport::Create(const wchar_t* lpszFileName)
{
    m_iKey = 0;
    if (m_fileStream.is_open())
    {
        m_fileStream.close();
    }
    std::string utf8Name = WideToUtf8(lpszFileName);
    m_filePath = std::filesystem::path(utf8Name);

    // CutHead reads any existing content and trims old sessions before appending
    CutHead();

    m_fileStream.open(m_filePath, std::ios::out | std::ios::app);
    if (!m_fileStream.is_open())
    {
        fprintf(stderr, "PLAT: ErrorReport — cannot create %s\n", m_filePath.string().c_str());
    }

#ifndef _WIN32
    // Expose a raw file descriptor for async-signal-safe crash handler writes.
    // open() with O_WRONLY|O_APPEND is the portable POSIX approach — avoids
    // relying on non-standard rdbuf()->fd() extensions.
    // [VS0-QUAL-SIGNAL-HANDLERS]
    if (m_fileStream.is_open())
    {
        // Close any previously opened fd (safe to call on -1, guarded)
        int oldFd = g_errorReportFd;
        if (oldFd >= 0)
        {
            close(oldFd);
        }
        g_errorReportFd = open(m_filePath.string().c_str(), O_WRONLY | O_APPEND);
    }
#endif
}

void CErrorReport::Destroy(void)
{
#ifndef _WIN32
    // Close the async-signal-safe fd before closing the stream.
    // [VS0-QUAL-SIGNAL-HANDLERS]
    int fd = g_errorReportFd;
    g_errorReportFd = -1;
    if (fd >= 0)
    {
        close(fd);
    }
#endif
    m_fileStream.close();
    Clear();
}

void CErrorReport::CutHead(void)
{
    std::ifstream inFile(m_filePath);
    if (!inFile.is_open())
    {
        return; // No existing file — nothing to trim
    }
    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();

    // Locate all session begin markers (ASCII, so UTF-8 find() works correctly)
    const std::string marker = "###### Log Begin ######";
    std::vector<size_t> positions;
    size_t pos = 0;
    while ((pos = content.find(marker, pos)) != std::string::npos)
    {
        positions.push_back(pos);
        pos += marker.size();
    }

    std::string trimmed;
    bool needRewrite = false;

    if (positions.size() >= 5)
    {
        // Keep last 4 sessions — trim everything before the (N-4)th marker
        trimmed = content.substr(positions[positions.size() - 4]);
        needRewrite = true;
    }
    else if (content.size() >= 32 * 1024 - 1)
    {
        // Overall 32 KB cap: truncate to second half
        trimmed = content.substr(content.size() / 2);
        needRewrite = true;
    }

    if (needRewrite)
    {
        std::filesystem::remove(m_filePath);
        std::ofstream outFile(m_filePath, std::ios::out);
        outFile.write(trimmed.c_str(), static_cast<std::streamsize>(trimmed.size()));
        outFile.close();
    }
}

void CErrorReport::WriteDebugInfoStr(wchar_t* lpszToWrite)
{
    if (m_fileStream.is_open())
    {
        std::string utf8 = WideToUtf8(lpszToWrite);
        m_fileStream.write(utf8.c_str(), static_cast<std::streamsize>(utf8.size()));
        m_fileStream.flush(); // ensure crash-time diagnostics reach disk (no app-level buffer loss)
        if (m_fileStream.fail())
        {
            std::wstring wPath = m_filePath.wstring();
            m_fileStream.close();
            m_fileStream.clear();
            Create(wPath.c_str());
        }
    }
}

void CErrorReport::Write(const wchar_t* lpszFormat, ...)
{
    wchar_t lpszBuffer[1024] = {
        0,
    };
    va_list va;
    va_start(va, lpszFormat);
    vswprintf(lpszBuffer, 1024, lpszFormat, va);
    va_end(va);

    WriteDebugInfoStr(lpszBuffer);
}

void CErrorReport::HexWrite(void* pBuffer, int iSize)
{
    wchar_t szLine[256] = {
        0,
    };
    int offset = 0;
#ifdef _WIN32
    offset += swprintf(szLine, L"0x%08X : ", (DWORD)(uintptr_t)pBuffer);
#else
    offset += std::swprintf(szLine, 1024, L"0x%08X : ", (DWORD)(uintptr_t)pBuffer);
#endif
    for (int i = 0; i < iSize; i++)
    {
        offset += mu_swprintf(szLine + offset, L"%02X", *((BYTE*)pBuffer + i));
        if (i > 0 && i < iSize - 1)
        {
            if (i % 16 == 15)
            { //. new line
                offset += mu_swprintf(szLine + offset, L"\r\n");
                WriteDebugInfoStr(szLine);
                offset = 0;
                offset += mu_swprintf(szLine + offset, L"           : ");
            }
            else if (i % 4 == 3)
            { //. space
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
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_local{};
#ifdef _WIN32
    localtime_s(&tm_local, &t);
#else
    localtime_r(&t, &tm_local);
#endif
    wchar_t szTime[32];
    mu_swprintf(szTime, L"%04d/%02d/%02d %02d:%02d", tm_local.tm_year + 1900, tm_local.tm_mon + 1, tm_local.tm_mday,
                tm_local.tm_hour, tm_local.tm_min);
    Write(L"%ls", szTime);
    if (bLineShift)
    {
        Write(L"\r\n");
    }
}

// ---------------------------------------------------------------------------
// Forward declarations for cross-platform helpers
// ---------------------------------------------------------------------------
namespace mu
{
std::vector<std::string> GetAudioDeviceNames();
} // namespace mu

// ---------------------------------------------------------------------------
// WriteSystemInfo — log system information to error report
// [Story 7-6-7: AC-3]
// ---------------------------------------------------------------------------
void CErrorReport::WriteSystemInfo(ER_SystemInfo* si)
{
    Write(L"<System information>\r\n");
    Write(L"OS \t\t\t: %ls\r\n", si->m_lpszOS);
    Write(L"CPU \t\t\t: %ls\r\n", si->m_lpszCPU);
    Write(L"RAM \t\t\t: %dMB\r\n", 1 + (si->m_iMemorySize / 1024 / 1024));
    AddSeparator();
    Write(L"GPU Backend \t\t: %ls\r\n", si->m_lpszGpuBackend);
}

// ---------------------------------------------------------------------------
// WriteOpenGLInfo — log OpenGL driver information
// [Story 7-6-7: AC-4] glGetString/glGetIntegerv are pure OpenGL — no guard needed
// ---------------------------------------------------------------------------
void CErrorReport::WriteOpenGLInfo(void)
{
    Write(L"<OpenGL information>\r\n");

    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* version = (const char*)glGetString(GL_VERSION);

    Write(L"Vendor\t\t: %hs\r\n", vendor ? vendor : "N/A");
    Write(L"Render\t\t: %hs\r\n", renderer ? renderer : "N/A");
    Write(L"OpenGL version\t: %hs\r\n", version ? version : "N/A");

    GLint iResult[2] = {0, 0};
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, iResult);
    Write(L"Max Texture size\t: %d x %d\r\n", iResult[0], iResult[0]);
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, iResult);
    Write(L"Max Viewport size\t: %d x %d\r\n", iResult[0], iResult[1]);
}

// ---------------------------------------------------------------------------
// WriteImeInfo — log text input method state via SDL3
// [Story 7-6-7: AC-5, AC-10]
// ---------------------------------------------------------------------------
void CErrorReport::WriteImeInfo(SDL_Window* pWindow)
{
    Write(L"<Text Input information>\r\n");

    if (pWindow && SDL_TextInputActive(pWindow))
    {
        Write(L"Text Input\t\t: Active\r\n");
    }
    else
    {
        Write(L"Text Input\t\t: Inactive\r\n");
    }

    int sdlVersion = SDL_GetVersion();
    Write(L"SDL Version\t\t: %d.%d.%d\r\n", sdlVersion / 1000000, (sdlVersion / 1000) % 1000, sdlVersion % 1000);
}

// ---------------------------------------------------------------------------
// WriteSoundCardInfo — enumerate audio devices via miniaudio
// [Story 7-6-7: AC-6]
// ---------------------------------------------------------------------------
void CErrorReport::WriteSoundCardInfo(void)
{
    std::vector<std::string> names = mu::GetAudioDeviceNames();

    if (!names.empty())
    {
        Write(L"<Sound card information>\r\n");
        for (const auto& name : names)
        {
            Write(L"Audio Device\t\t: %hs\r\n", name.c_str());
        }
    }
    else
    {
        Write(L"No audio device found.\r\n");
    }

    AddSeparator();
}

// ---------------------------------------------------------------------------
// GetSystemInfo — populate ER_SystemInfo using cross-platform POSIX APIs
// [Story 7-6-7: AC-3]
// ---------------------------------------------------------------------------
void GetSystemInfo(ER_SystemInfo* si)
{
    memset(si, 0, sizeof(ER_SystemInfo));

    // OS — uname() is POSIX, available on all platforms
    struct utsname u;
    if (uname(&u) == 0)
    {
        swprintf(si->m_lpszOS, MAX_LENGTH_OSINFO, L"%hs %hs", u.sysname, u.release);
    }
    else
    {
        wcscpy(si->m_lpszOS, L"Unknown OS");
    }

    // CPU
#ifdef __APPLE__
    char cpuBrand[128] = {0};
    size_t cpuBrandLen = sizeof(cpuBrand);
    if (sysctlbyname("machdep.cpu.brand_string", cpuBrand, &cpuBrandLen, nullptr, 0) == 0)
    {
        swprintf(si->m_lpszCPU, MAX_LENGTH_CPUNAME, L"%hs", cpuBrand);
    }
    else
    {
        wcscpy(si->m_lpszCPU, L"Unknown CPU");
    }
#else
    // Linux: read /proc/cpuinfo
    std::ifstream cpuFile("/proc/cpuinfo");
    std::string cpuLine;
    bool cpuFound = false;
    while (std::getline(cpuFile, cpuLine))
    {
        if (cpuLine.find("model name") != std::string::npos)
        {
            auto pos = cpuLine.find(':');
            if (pos != std::string::npos)
            {
                std::string model = cpuLine.substr(pos + 2);
                swprintf(si->m_lpszCPU, MAX_LENGTH_CPUNAME, L"%hs", model.c_str());
                cpuFound = true;
            }
            break;
        }
    }
    if (!cpuFound)
    {
        wcscpy(si->m_lpszCPU, L"Unknown CPU");
    }
#endif

    // RAM
#ifdef __APPLE__
    uint64_t memSize = 0;
    size_t memLen = sizeof(memSize);
    if (sysctlbyname("hw.memsize", &memSize, &memLen, nullptr, 0) == 0)
    {
        si->m_iMemorySize = static_cast<int>(memSize);
    }
#else
    // Linux: read /proc/meminfo
    std::ifstream memFile("/proc/meminfo");
    std::string memLine;
    while (std::getline(memFile, memLine))
    {
        if (memLine.find("MemTotal") != std::string::npos)
        {
            // Format: "MemTotal:    16384000 kB"
            auto pos = memLine.find(':');
            if (pos != std::string::npos)
            {
                long long memKb = std::strtoll(memLine.c_str() + pos + 1, nullptr, 10);
                si->m_iMemorySize = static_cast<int>(memKb * 1024);
            }
            break;
        }
    }
#endif

    // GPU Backend — populated by caller from MuRenderer if available
    wcscpy(si->m_lpszGpuBackend, L"unknown");
}

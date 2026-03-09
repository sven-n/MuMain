// ErrorReport.cpp: implementation of the CErrorReport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _WIN32
#include <ddraw.h>
#include <dinput.h>
#include <dmusicc.h>
#include <eh.h>
#include <imagehlp.h>
#endif

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#endif

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

#ifdef _WIN32

void CErrorReport::WriteSystemInfo(ER_SystemInfo* si)
{
    Write(L"<System information>\r\n");
    Write(L"OS \t\t\t: %ls\r\n", si->m_lpszOS);
    Write(L"CPU \t\t\t: %ls\r\n", si->m_lpszCPU);
    Write(L"RAM \t\t\t: %dMB\r\n", 1 + (si->m_iMemorySize / 1024 / 1024));
    AddSeparator();
    Write(L"Direct-X \t\t: %ls\r\n", si->m_lpszDxVersion);
}

void CErrorReport::WriteOpenGLInfo(void)
{
    Write(L"<OpenGL information>\r\n");
    Write(L"Vendor\t\t: %ls\r\n", (wchar_t*)glGetString(GL_VENDOR));
    Write(L"Render\t\t: %ls\r\n", (wchar_t*)glGetString(GL_RENDERER));
    Write(L"OpenGL version\t: %ls\r\n", (wchar_t*)glGetString(GL_VERSION));
    GLint iResult[2];
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, iResult);
    Write(L"Max Texture size\t: %d x %d\r\n", iResult[0], iResult[0]);
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, iResult);
    Write(L"Max Viewport size\t: %d x %d\r\n", iResult[0], iResult[1]);
}

void CErrorReport::WriteImeInfo(HWND hWnd)
{
    wchar_t lpszTemp[256];
    Write(L"<IME information>\r\n");

    HIMC hImc = ImmGetContext(hWnd);
    if (hImc)
    {
        HKL hKl = GetKeyboardLayout(0);
        ImmGetDescription(hKl, lpszTemp, 256);
        Write(L"IME Name\t\t: %ls\r\n", lpszTemp);
        ImmGetIMEFileName(hKl, lpszTemp, 256);
        Write(L"IME File Name\t\t: %ls\r\n", lpszTemp);
        ImmReleaseContext(hWnd, hImc);
    }
    GetKeyboardLayoutName(lpszTemp);
    Write(L"Keyboard type\t\t: %ls\r\n", lpszTemp);
}

typedef struct tagER_SOUNDDEVICE
{
    wchar_t szGuid[64];
    wchar_t szDeviceName[128];
    wchar_t szDriverName[128];
} ER_SOUNDDEVICEINFO;

typedef struct tagSOUNDDEVICEENUM
{
    enum
    {
        MAX_DEVICENUM = 20
    };
    // cppcheck-suppress uninitMemberVar
    tagSOUNDDEVICEENUM()
    {
        nDeivceCount = 0;
    }
    ER_SOUNDDEVICEINFO infoSoundDevice[MAX_DEVICENUM];
    size_t nDeivceCount;

    tagER_SOUNDDEVICE& operator[](size_t p)
    {
        return infoSoundDevice[p];
    }
    tagER_SOUNDDEVICE& GetNextDevice()
    {
        return infoSoundDevice[nDeivceCount];
    }
} ER_SOUNDDEVICEENUMINFO;

INT_PTR CALLBACK DSoundEnumCallback(GUID* pGUID, LPWSTR strDesc, LPWSTR strDrvName, VOID* pContext)
{
    if (pGUID)
    {
        // cppcheck-suppress dangerousTypeCast
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
    ER_SOUNDDEVICEENUMINFO sdi;
    DirectSoundEnumerate((LPDSENUMCALLBACK)DSoundEnumCallback, &sdi);

    if (sdi.nDeivceCount > 0)
    {
        Write(L"<Sound card information>\r\n");
    }
    else
    {
        Write(L"No sound card found.\r\n");
        return;
    }

    for (unsigned int i = 0; i < sdi.nDeivceCount; ++i)
    {
        Write(L"Sound Card \t\t: %ls\r\n", sdi.infoSoundDevice[i].szDeviceName);

        wchar_t lpszBuffer[MAX_PATH];
        GetSystemDirectory(lpszBuffer, MAX_PATH);
        wcscat(lpszBuffer, L"\\drivers\\");
        wcscat(lpszBuffer, sdi.infoSoundDevice[i].szDriverName);
        WORD wVersion[4];
        GetFileVersion(lpszBuffer, wVersion);

        Write(L"Sound Card Driver\t: %ls (%d.%d.%d.%d)\r\n", sdi.infoSoundDevice[i].szDriverName, wVersion[0],
              wVersion[1], wVersion[2], wVersion[3]);
    }

    AddSeparator();
}

void GetOSVersion(ER_SystemInfo* si)
{
    const wchar_t* lpszUnknown = L"Unknown";
    wchar_t lpszTemp[256];

    OSVERSIONINFO osiOne;
    osiOne.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osiOne);

    int iBuildNumberType = 0;
    mu_swprintf(si->m_lpszOS, L"%ls %d.%d ", lpszUnknown, osiOne.dwMajorVersion, osiOne.dwMinorVersion);

    switch (osiOne.dwMajorVersion)
    {
    case 3: // NT 3.51
        switch (osiOne.dwMinorVersion)
        {
        case 51:
            wcscpy(si->m_lpszOS, L"Windows NT 3.51");
            break;
        }
        break;
    case 4:
        switch (osiOne.dwMinorVersion)
        {
        case 0:
            switch (osiOne.dwPlatformId)
            {
            case VER_PLATFORM_WIN32_WINDOWS:
                wcscpy(si->m_lpszOS, L"Windows 95 ");
                if (osiOne.szCSDVersion[1] == 'C' || osiOne.szCSDVersion[1] == 'B')
                {
                    wcscat(si->m_lpszOS, L"OSR2");
                }
                iBuildNumberType = 1;
                break;
            case VER_PLATFORM_WIN32_NT:
                wcscpy(si->m_lpszOS, L"Windows NT 4.0 ");
                break;
            }
            break;
        case 10:
            wcscpy(si->m_lpszOS, L"Windows 98 ");
            if (osiOne.szCSDVersion[1] == 'A')
            {
                wcscat(si->m_lpszOS, L"SE ");
            }
            iBuildNumberType = 1;
            break;
        case 90:
            wcscpy(si->m_lpszOS, L"Windows Me ");
            iBuildNumberType = 1;
            break;
        }
        break;
    case 5:
        switch (osiOne.dwMinorVersion)
        {
        case 0:
            wcscpy(si->m_lpszOS, L"Windows 2000 ");
            {
                HKEY hKey;
                DWORD dwBufLen;
                if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                                  L"SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0,
                                                  KEY_QUERY_VALUE, &hKey))
                {
                    if (ERROR_SUCCESS == RegQueryValueEx(hKey, L"ProductType", NULL, NULL, (LPBYTE)lpszTemp, &dwBufLen))
                    {
                        if (0 == lstrcmpi(L"WINNT", lpszTemp))
                        {
                            wcscat(si->m_lpszOS, L"Professional ");
                        }
                        if (0 == lstrcmpi(L"LANMANNT", lpszTemp))
                        {
                            wcscat(si->m_lpszOS, L"Server ");
                        }
                        if (0 == lstrcmpi(L"SERVERNT", lpszTemp))
                        {
                            wcscat(si->m_lpszOS, L"Advanced Server ");
                        }
                    }

                    RegCloseKey(hKey);
                }
            }
            break;
        case 1:
            wcscpy(si->m_lpszOS, L"Windows XP ");
            break;
        case 2:
            wcscpy(si->m_lpszOS, L"Windows 2003 family ");
            break;
        }
        break;
    }
    switch (iBuildNumberType)
    {
    case 0:
        mu_swprintf(lpszTemp, L"Build %d ", osiOne.dwBuildNumber);
        break;
    case 1:
        mu_swprintf(lpszTemp, L"Build %d.%d.%d ", HIBYTE(HIWORD(osiOne.dwBuildNumber)),
                    LOBYTE(HIWORD(osiOne.dwBuildNumber)), LOWORD(osiOne.dwBuildNumber));
        break;
    }
    wcscat(si->m_lpszOS, lpszTemp);
    mu_swprintf(lpszTemp, L"(%ls)", osiOne.szCSDVersion);
    wcscat(si->m_lpszOS, lpszTemp);
}

// NOTE:
// The original implementation of GetCPUFrequency and GetCPUInfo relied on
// MSVC inline assembly (cpuid/rdtsc) and 32-bit affinity mask types, which
// are not portable and fail to compile under MinGW/GCC. For the purposes of
// error reporting on this toolchain, a simplified, portable implementation
// is sufficient.

__int64 GetCPUFrequency(unsigned int uiMeasureMSecs)
{
    (void)uiMeasureMSecs; // unused on this platform

    // High‑resolution CPU frequency measurement is handled elsewhere
    // (see Utilities/CpuUsage.cpp). Here we return 0 to indicate that
    // a specific CPU MHz value is not available in this build.
    return 0;
}

void GetCPUInfo(ER_SystemInfo* si)
{
    if (si == nullptr)
        return;

    wcscpy(si->m_lpszCPU, L"Unknown CPU");
}

typedef HRESULT(WINAPI* DIRECTDRAWCREATE)(GUID*, LPDIRECTDRAW*, IUnknown*);
typedef HRESULT(WINAPI* DIRECTDRAWCREATEEX)(GUID*, VOID**, REFIID, IUnknown*);
typedef HRESULT(WINAPI* DIRECTINPUTCREATE)(HINSTANCE, DWORD, LPDIRECTINPUT*, IUnknown*);

DWORD GetDXVersion()
{
    DIRECTDRAWCREATE DirectDrawCreate = NULL;
    DIRECTDRAWCREATEEX DirectDrawCreateEx = NULL;
    DIRECTINPUTCREATE DirectInputCreate = NULL;
    HINSTANCE hDDrawDLL = NULL;
    HINSTANCE hDInputDLL = NULL;
    HINSTANCE hD3D8DLL = NULL;
    HINSTANCE hD3D9DLL = NULL;
    LPDIRECTDRAW pDDraw = NULL;
    LPDIRECTDRAW2 pDDraw2 = NULL;
    LPDIRECTDRAWSURFACE pSurf = NULL;
    LPDIRECTDRAWSURFACE3 pSurf3 = NULL;
    LPDIRECTDRAWSURFACE4 pSurf4 = NULL;
    DWORD dwDXVersion = 0;
    HRESULT hr;

    // First see if DDRAW.DLL even exists.
    hDDrawDLL = LoadLibrary(L"DDRAW.DLL");
    if (hDDrawDLL == NULL)
    {
        dwDXVersion = 0;
        return dwDXVersion;
    }

    // See if we can create the DirectDraw object.
    DirectDrawCreate = (DIRECTDRAWCREATE)GetProcAddress(hDDrawDLL, "DirectDrawCreate");
    if (DirectDrawCreate == NULL)
    {
        dwDXVersion = 0;
        FreeLibrary(hDDrawDLL);

        __TraceF(TEXT("===> Couldn't LoadLibrary DDraw\r\n"));
        return dwDXVersion;
    }

    hr = DirectDrawCreate(NULL, &pDDraw, NULL);
    if (FAILED(hr))
    {
        dwDXVersion = 0;
        FreeLibrary(hDDrawDLL);
        __TraceF(TEXT("===> Couldn't create DDraw\r\n"));
        return dwDXVersion;
    }

    // So DirectDraw exists.  We are at least DX1.
    dwDXVersion = 0x100;

    // Let's see if IID_IDirectDraw2 exists.
    hr = pDDraw->QueryInterface(IID_IDirectDraw2, (VOID**)&pDDraw2);
    if (FAILED(hr))
    {
        // No IDirectDraw2 exists... must be DX1
        pDDraw->Release();
        FreeLibrary(hDDrawDLL);
        __TraceF(TEXT("===> Couldn't QI DDraw2\r\n"));
        return dwDXVersion;
    }

    // IDirectDraw2 exists. We must be at least DX2
    pDDraw2->Release();
    dwDXVersion = 0x200;

    //-------------------------------------------------------------------------
    // DirectX 3.0 Checks
    //-------------------------------------------------------------------------

    // DirectInput was added for DX3
    hDInputDLL = LoadLibrary(L"DINPUT.DLL");
    if (hDInputDLL == NULL)
    {
        // No DInput... must not be DX3
        __TraceF(TEXT("===> Couldn't LoadLibrary DInput\r\n"));
        pDDraw->Release();
        return dwDXVersion;
    }

    DirectInputCreate = (DIRECTINPUTCREATE)GetProcAddress(hDInputDLL, "DirectInputCreateA");
    if (DirectInputCreate == NULL)
    {
        // No DInput... must be DX2
        FreeLibrary(hDInputDLL);
        FreeLibrary(hDDrawDLL);
        pDDraw->Release();
        __TraceF(TEXT("===> Couldn't GetProcAddress DInputCreate\r\n"));
        return dwDXVersion;
    }

    // DirectInputCreate exists. We are at least DX3
    dwDXVersion = 0x300;
    FreeLibrary(hDInputDLL);

    // Can do checks for 3a vs 3b here

    //-------------------------------------------------------------------------
    // DirectX 5.0 Checks
    //-------------------------------------------------------------------------

    // We can tell if DX5 is present by checking for the existence of
    // IDirectDrawSurface3. First, we need a surface to QI off of.
    DDSURFACEDESC ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    hr = pDDraw->SetCooperativeLevel(NULL, DDSCL_NORMAL);
    if (FAILED(hr))
    {
        // Failure. This means DDraw isn't properly installed.
        pDDraw->Release();
        FreeLibrary(hDDrawDLL);
        dwDXVersion = 0;
        __TraceF(TEXT("===> Couldn't Set coop level\r\n"));
        return dwDXVersion;
    }

    hr = pDDraw->CreateSurface(&ddsd, &pSurf, NULL);
    if (FAILED(hr))
    {
        // Failure. This means DDraw isn't properly installed.
        pDDraw->Release();
        FreeLibrary(hDDrawDLL);
        dwDXVersion = 0;
        __TraceF(TEXT("===> Couldn't CreateSurface\r\n"));
        return dwDXVersion;
    }

    // Query for the IDirectDrawSurface3 interface
    if (FAILED(pSurf->QueryInterface(IID_IDirectDrawSurface3, (VOID**)&pSurf3)))
    {
        pDDraw->Release();
        FreeLibrary(hDDrawDLL);
        return dwDXVersion;
    }

    // QI for IDirectDrawSurface3 succeeded. We must be at least DX5
    dwDXVersion = 0x500;

    //-------------------------------------------------------------------------
    // DirectX 6.0 Checks
    //-------------------------------------------------------------------------

    // The IDirectDrawSurface4 interface was introduced with DX 6.0
    if (FAILED(pSurf->QueryInterface(IID_IDirectDrawSurface4, (VOID**)&pSurf4)))
    {
        pDDraw->Release();
        FreeLibrary(hDDrawDLL);
        return dwDXVersion;
    }

    // IDirectDrawSurface4 was create successfully. We must be at least DX6
    dwDXVersion = 0x600;
    pSurf->Release();
    pDDraw->Release();

    //-------------------------------------------------------------------------
    // DirectX 6.1 Checks
    //-------------------------------------------------------------------------

    // Check for DMusic, which was introduced with DX6.1
    LPDIRECTMUSIC pDMusic = NULL;
    CoInitialize(NULL);
    hr = CoCreateInstance(CLSID_DirectMusic, NULL, CLSCTX_INPROC_SERVER, IID_IDirectMusic, (VOID**)&pDMusic);
    if (FAILED(hr))
    {
        __TraceF(TEXT("===> Couldn't create CLSID_DirectMusic\r\n"));
        FreeLibrary(hDDrawDLL);
        return dwDXVersion;
    }

    // DirectMusic was created successfully. We must be at least DX6.1
    dwDXVersion = 0x601;
    pDMusic->Release();
    CoUninitialize();

    //-------------------------------------------------------------------------
    // DirectX 7.0 Checks
    //-------------------------------------------------------------------------

    // Check for DirectX 7 by creating a DDraw7 object
    LPDIRECTDRAW7 pDD7;
    DirectDrawCreateEx = (DIRECTDRAWCREATEEX)GetProcAddress(hDDrawDLL, "DirectDrawCreateEx");
    if (NULL == DirectDrawCreateEx)
    {
        FreeLibrary(hDDrawDLL);
        return dwDXVersion;
    }

    if (FAILED(DirectDrawCreateEx(NULL, (VOID**)&pDD7, IID_IDirectDraw7, NULL)))
    {
        FreeLibrary(hDDrawDLL);
        return dwDXVersion;
    }

    // DDraw7 was created successfully. We must be at least DX7.0
    dwDXVersion = 0x700;
    pDD7->Release();

    //-------------------------------------------------------------------------
    // DirectX 8.0 Checks
    //-------------------------------------------------------------------------

    // Simply see if D3D8.dll exists.
    hD3D8DLL = LoadLibrary(L"D3D8.DLL");
    if (hD3D8DLL == NULL)
    {
        FreeLibrary(hDDrawDLL);
        return dwDXVersion;
    }

    // D3D8.dll exists. We must be at least DX8.0
    dwDXVersion = 0x800;

    //-------------------------------------------------------------------------
    // DirectX 9.0 Checks
    //-------------------------------------------------------------------------
    hD3D9DLL = LoadLibrary(L"D3D9.DLL");
    if (hD3D9DLL == NULL)
    {
        FreeLibrary(hDDrawDLL);
        FreeLibrary(hD3D8DLL);
        return dwDXVersion;
    }
    dwDXVersion = 0x900;

    //-------------------------------------------------------------------------
    // End of checking for versions of DirectX
    //-------------------------------------------------------------------------

    // Close open libraries and return
    FreeLibrary(hDDrawDLL);
    FreeLibrary(hD3D8DLL);
    FreeLibrary(hD3D9DLL);

    return dwDXVersion;
}

void GetSystemInfo(ER_SystemInfo* si)
{
    ZeroMemory(si, sizeof(ER_SystemInfo));

    // CPU
    GetCPUInfo(si);

    // Memory
    MEMORYSTATUS ms;
    ms.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&ms);
    si->m_iMemorySize = ms.dwTotalPhys;

    // OS
    GetOSVersion(si);

    // DX
    DWORD dwDX = GetDXVersion();
    mu_swprintf(si->m_lpszDxVersion, L"Direct-X %d.%d", dwDX >> 8, dwDX & 0xFF);
}

#endif // _WIN32

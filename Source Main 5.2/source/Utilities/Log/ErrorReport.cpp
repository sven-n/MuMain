// ErrorReport.cpp: implementation of the CErrorReport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <ddraw.h>
#include <dinput.h>
#include <dmusicc.h>
#include <eh.h>
#include <imagehlp.h>
#include "ErrorReport.h"

void DeleteSocket();

CErrorReport g_ErrorReport;

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
    m_hFile = INVALID_HANDLE_VALUE;
    m_lpszFileName[0] = '\0';
    m_iKey = 0;
}

void CErrorReport::Create(wchar_t* lpszFileName)
{
    wcscpy(m_lpszFileName, lpszFileName);
    m_iKey = 0;
    m_hFile = CreateFile(m_lpszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Write UTF-16 BOM if new file
    if (m_hFile != INVALID_HANDLE_VALUE && GetLastError() != ERROR_ALREADY_EXISTS) {
        const wchar_t BOM = 0xFEFF;
        DWORD written;
        ::WriteFile(m_hFile, &BOM, sizeof(BOM), &written, NULL);
    }

    CutHead();
    SetFilePointer(m_hFile, 0, NULL, FILE_END);
}

void CErrorReport::Destroy(void)
{
    CloseHandle(m_hFile);
    Clear();
}

void CErrorReport::CutHead(void)
{
    DWORD dwNumber;
    wchar_t lpszBuffer[128 * 1024];
    ReadFile(m_hFile, lpszBuffer, 128 * 1024 - 1, &dwNumber, NULL);
    lpszBuffer[dwNumber] = '\0';
    wchar_t* lpCut = CheckHeadToCut(lpszBuffer, dwNumber);
    if (dwNumber >= 32 * 1024 - 1)
    {
        lpCut = &lpszBuffer[32 * 1024 - 1];
    }
    if (lpCut != lpszBuffer)
    {
        CloseHandle(m_hFile);
        DeleteFile(m_lpszFileName);
        m_hFile = CreateFile(m_lpszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        DWORD dwSize = dwNumber - (lpCut - lpszBuffer);
        m_iKey = 0;
        WriteFile(m_hFile, lpCut, dwSize, &dwNumber, NULL);
    }
}

wchar_t* CErrorReport::CheckHeadToCut(wchar_t* lpszBuffer, DWORD dwNumber)
{
    wchar_t* lpszBegin = L"###### Log Begin ######";
    int iLengthOfBegin = wcslen(lpszBegin);

    wchar_t* lpFoundList[128];
    int iFoundCount = 0;

    for (wchar_t* lpFind = lpszBuffer; lpFind && *lpFind; )
    {
        lpFind = wcschr(lpFind, (int)'#');
        if (lpFind)
        {
            if (0 == wcsncmp(lpFind, lpszBegin, iLengthOfBegin))
            {
                lpFoundList[iFoundCount++] = lpFind;
                lpFind += iLengthOfBegin;
            }
            else
            {
                lpFind++;
            }
        }
    }

    if (iFoundCount >= 5)
    {
        return (lpFoundList[iFoundCount - 4]);
    }
    return (lpszBuffer);
}

BOOL CErrorReport::WriteFile(HANDLE hFile, void* lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
    return ::WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

void CErrorReport::WriteDebugInfoStr(wchar_t* lpszToWrite)
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwNumber;
        WriteFile(m_hFile, lpszToWrite, wcslen(lpszToWrite) * sizeof(wchar_t), &dwNumber, NULL);

        if (dwNumber == 0)
        {
            CloseHandle(m_hFile);
            Create(m_lpszFileName);
        }
    }
}

void CErrorReport::Write(const wchar_t* lpszFormat, ...)
{
    wchar_t lpszBuffer[2048] = { 0 };
    va_list va;
    va_start(va, lpszFormat);
    vswprintf(lpszBuffer, _countof(lpszBuffer), lpszFormat, va);
    va_end(va);

    WriteDebugInfoStr(lpszBuffer);
}

void CErrorReport::HexWrite(void* pBuffer, int iSize)
{
    DWORD dwWritten = 0;
    wchar_t szLine[256] = { 0 };
    int offset = 0;
    offset += swprintf(szLine, L"0x%08X : ", (DWORD*)pBuffer);
    for (int i = 0; i < iSize; i++) {
        offset += swprintf(szLine + offset, L"%02X", *((BYTE*)pBuffer + i));
        if (i > 0 && i < iSize - 1) {
            if (i % 16 == 15) {
                offset += swprintf(szLine + offset, L"\r\n");
                WriteFile(m_hFile, szLine, wcslen(szLine) * sizeof(wchar_t), &dwWritten, NULL);
                offset = 0;
                offset += swprintf(szLine + offset, L"           : ");
            }
            else if (i % 4 == 3) {
                offset += swprintf(szLine + offset, L" ");
            }
        }
    }
    offset += swprintf(szLine + offset, L"\r\n");
    WriteFile(m_hFile, szLine, wcslen(szLine) * sizeof(wchar_t), &dwWritten, NULL);
}

void CErrorReport::AddSeparator(void)
{
    Write(L"------------------------------------------------------------\r\n");
}

void CErrorReport::WriteLogBegin(void)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    Write(L"\n###### Log Begin ######\r\n");
    Write(L"Date: %04d/%02d/%02d\r\n", st.wYear, st.wMonth, st.wDay);
    Write(L"Time: %02d:%02d:%02d\r\n", st.wHour, st.wMinute, st.wSecond);
    AddSeparator();
}

void CErrorReport::WriteCurrentTime(BOOL bLineShift)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    Write(L"%04d/%02d/%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
    if (bLineShift)
    {
        Write(L"\r\n");
    }
}

void CErrorReport::WriteLabelValue(const wchar_t* label, const wchar_t* value)
{
    const int labelWidth = 25;
    wchar_t buffer[256];
    swprintf(buffer, L"%-*s: %s\r\n", labelWidth, label, value);
    WriteDebugInfoStr(buffer);
}

void CErrorReport::WriteSystemInfo(ER_SystemInfo* si)
{
    Write(L"\n===== SYSTEM INFORMATION =====\r\n");
    WriteLabelValue(L"OS", si->m_lpszOS);
    WriteLabelValue(L"CPU", si->m_lpszCPU);

    wchar_t ram[32];
    swprintf(ram, L"%dMB", 1 + (si->m_iMemorySize / 1024 / 1024));
    WriteLabelValue(L"RAM", ram);

    AddSeparator();
    WriteLabelValue(L"Direct-X", si->m_lpszDxVersion);
}

void CErrorReport::WriteOpenGLInfo(void)
{
    Write(L"\n===== OPENGL INFORMATION =====\r\n");
    WriteLabelValue(L"Vendor", (wchar_t*)glGetString(GL_VENDOR));
    WriteLabelValue(L"Renderer", (wchar_t*)glGetString(GL_RENDERER));
    WriteLabelValue(L"OpenGL version", (wchar_t*)glGetString(GL_VERSION));

    GLint iResult[2];
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, iResult);
    wchar_t texSize[32];
    swprintf(texSize, L"%d x %d", iResult[0], iResult[0]);
    WriteLabelValue(L"Max Texture size", texSize);

    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, iResult);
    swprintf(texSize, L"%d x %d", iResult[0], iResult[1]);
    WriteLabelValue(L"Max Viewport size", texSize);
}

void CErrorReport::WriteImeInfo(HWND hWnd)
{
    Write(L"\n===== IME INFORMATION =====\r\n");
    wchar_t lpszTemp[256];

    HIMC hImc = ImmGetContext(hWnd);
    if (hImc)
    {
        HKL hKl = GetKeyboardLayout(0);
        ImmGetDescription(hKl, lpszTemp, 256);
        WriteLabelValue(L"IME Name", lpszTemp);
        ImmGetIMEFileName(hKl, lpszTemp, 256);
        WriteLabelValue(L"IME File Name", lpszTemp);
        ImmReleaseContext(hWnd, hImc);
    }
    GetKeyboardLayoutName(lpszTemp);
    WriteLabelValue(L"Keyboard type", lpszTemp);
}

void CErrorReport::WriteSoundCardInfo(void)
{
    ER_SOUNDDEVICEENUMINFO sdi;
    DirectSoundEnumerate((LPDSENUMCALLBACK)DSoundEnumCallback, &sdi);

    if (sdi.nDeivceCount > 0)
    {
        Write(L"\n===== SOUND CARD INFORMATION =====\r\n");
    }
    else
    {
        Write(L"\nNo sound card found.\r\n");
        return;
    }

    for (unsigned int i = 0; i < sdi.nDeivceCount; ++i)
    {
        WriteLabelValue(L"Sound Card", sdi.infoSoundDevice[i].szDeviceName);

        wchar_t lpszBuffer[MAX_PATH];
        GetSystemDirectory(lpszBuffer, MAX_PATH);
        wcscat(lpszBuffer, L"\\drivers\\");
        wcscat(lpszBuffer, sdi.infoSoundDevice[i].szDriverName);
        WORD wVersion[4];
        if (GetFileVersion(lpszBuffer, wVersion))
        {
            wchar_t driverInfo[256];
            swprintf(driverInfo, L"%s (%d.%d.%d.%d)",
                sdi.infoSoundDevice[i].szDriverName,
                wVersion[0], wVersion[1], wVersion[2], wVersion[3]);
            WriteLabelValue(L"Driver", driverInfo);
        }
        else
        {
            WriteLabelValue(L"Driver", sdi.infoSoundDevice[i].szDriverName);
        }
    }
    AddSeparator();
}

// [Rest of your existing functions (GetOSVersion, GetCPUFrequency, GetCPUInfo, GetDXVersion, GetSystemInfo) remain exactly the same]

void GetOSVersion(ER_SystemInfo* si)
{
    wchar_t* lpszUnknown = L"Unknown";
    wchar_t lpszTemp[256];

    OSVERSIONINFO osiOne;
    osiOne.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osiOne);

    int iBuildNumberType = 0;
    swprintf(si->m_lpszOS, L"%s %d.%d ", lpszUnknown, osiOne.dwMajorVersion, osiOne.dwMinorVersion);

    switch (osiOne.dwMajorVersion)
    {
    case 3:	// NT 3.51
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
                if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
                    0, KEY_QUERY_VALUE, &hKey))
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
        swprintf(lpszTemp, L"Build %d ", osiOne.dwBuildNumber);
        break;
    case 1:
        swprintf(lpszTemp, L"Build %d.%d.%d ", HIBYTE(HIWORD(osiOne.dwBuildNumber)), LOBYTE(HIWORD(osiOne.dwBuildNumber)), LOWORD(osiOne.dwBuildNumber));
        break;
    }
    wcscat(si->m_lpszOS, lpszTemp);
    swprintf(lpszTemp, L"(%s)", osiOne.szCSDVersion);
    wcscat(si->m_lpszOS, lpszTemp);
}

__int64 GetCPUFrequency(unsigned int uiMeasureMSecs)
{
    assert(uiMeasureMSecs > 0);

    // First we get the CPUID standard level 0x00000001
    DWORD reg;
    __asm
    {
        mov eax, 1
        cpuid
        mov reg, edx
    }

    // Then we check, if the RDTSC (Real Date Time Stamp Counter) is available.
    // This function is necessary for our measure process.
    if (!(reg & (1 << 4)))
    {
        return (0);
    }

    // After that we declare some vars and check the frequency of the high
    // resolution timer for the measure process.
    // If there's no high-res timer, we exit.
    __int64 llFreq;
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&llFreq))
    {
        return (0);
    }

    // Now we can init the measure process. We set the process and thread priority
    // to the highest available level (Realtime priority). Also we focus the
    // first processor in the multiprocessor system.
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    DWORD dwCurPriorityClass = GetPriorityClass(hProcess);
    int iCurThreadPriority = GetThreadPriority(hThread);
    DWORD dwProcessMask, dwSystemMask, dwNewMask = 1;
    GetProcessAffinityMask(hProcess, &dwProcessMask, &dwSystemMask);

    SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);
    SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
    SetProcessAffinityMask(hProcess, dwNewMask);

    // Now we call a CPUID to ensure, that all other prior called functions are
    // completed now (serialization)
    __asm { cpuid }

    __int64 llStartTime, llEndTime;
    __int64 llStart, llEnd;
    // We ask the high-res timer for the start time
    QueryPerformanceCounter((LARGE_INTEGER*)&llStartTime);
    // Then we get the current cpu clock and store it
    __asm
    {
        rdtsc
        mov dword ptr[llStart + 4], edx
        mov dword ptr[llStart], eax
    }

    // Now we wart for some msecs
    Sleep(uiMeasureMSecs);

    // We ask for the end time
    QueryPerformanceCounter((LARGE_INTEGER*)&llEndTime);
    // And also for the end cpu clock
    __asm
    {
        rdtsc
        mov dword ptr[llEnd + 4], edx
        mov dword ptr[llEnd], eax
    }

    // Now we can restore the default process and thread priorities
    SetProcessAffinityMask(hProcess, dwProcessMask);
    SetThreadPriority(hThread, iCurThreadPriority);
    SetPriorityClass(hProcess, dwCurPriorityClass);

    // Then we calculate the time and clock differences
    __int64 llDif = llEnd - llStart;
    __int64 llTimeDif = llEndTime - llStartTime;

    // And finally the frequency is the clock difference divided by the time
    // difference.
    auto llFrequency = (__int64)(((double)llDif) / (((double)llTimeDif) / llFreq));
    // At last we just return the frequency that is also stored in the call
    // member var uqwFrequency
    return llFrequency;
}

void GetCPUInfo(ER_SystemInfo* si)
{
    wchar_t* lpszUnknown = L"Unknown";
    wchar_t lpszTemp[256];

    DWORD eaxreg, ebxreg, ecxreg, edxreg;

    // We read the standard CPUID level 0x00000000 which should
    // be available on every x86 processor
    __asm
    {
        mov eax, 0
        cpuid
        mov eaxreg, eax
        mov ebxreg, ebx
        mov edxreg, edx
        mov ecxreg, ecx
    }
    int iBrand = ebxreg;
    *((DWORD*)si->m_lpszCPU) = ebxreg;
    *((DWORD*)(si->m_lpszCPU + 4)) = edxreg;
    *((DWORD*)(si->m_lpszCPU + 8)) = ecxreg;
    wcscat(si->m_lpszCPU, L" - ");
    unsigned long ulMaxSupportedLevel, ulMaxSupportedExtendedLevel;
    ulMaxSupportedLevel = eaxreg & 0xFFFF;
    // Then we read the ext. CPUID level 0x80000000
    // ...to check the max. supportted extended CPUID level
    __asm
    {
        mov eax, 0x80000000
        cpuid
        mov eaxreg, eax
    }
    ulMaxSupportedExtendedLevel = eaxreg;

    // First we get the CPUID standard level 0x00000001
    __asm
    {
        mov eax, 1
        cpuid
        mov eaxreg, eax
    }
    unsigned int uiFamily = (eaxreg >> 8) & 0xF;
    unsigned int uiModel = (eaxreg >> 4) & 0xF;

    switch (iBrand)
    {
    case 0x756E6547:	// GenuineIntel
        switch (uiFamily)
        {
        case 3:	// 386
            break;
        case 4:	// 486
            break;
        case 5:	// pentium
            break;
        case 6:	// pentium pro - pentium 3
            switch (uiModel)
            {
            case 0:
            case 1:
            default:
                wcscat(si->m_lpszCPU, L"Pentium Pro");
                break;
            case 3:
            case 5:
                wcscat(si->m_lpszCPU, L"Pentium 2");
                break;
            case 6:
                wcscat(si->m_lpszCPU, L"Pentium Celeron");
                break;
            case 7:
            case 8:
            case 0xA:
            case 0xB:
                wcscat(si->m_lpszCPU, L"Pentium 3");
                break;
            }
            break;
        case 15:	// pentium 4
            wcscat(si->m_lpszCPU, L"Pentium 4");
            break;
        default:
            wcscat(si->m_lpszCPU, lpszUnknown);
            break;
        }
        break;
    case 0x68747541:	// AuthenticAMD
        switch (uiFamily)
        {
        case 4:	// 486, 586
            switch (uiModel)
            {
            case 3:
            case 7:
            case 8:
            case 9:
                wcscat(si->m_lpszCPU, L"AMD 486");
                break;
            case 0xE:
            case 0xF:
                wcscat(si->m_lpszCPU, L"AMD 586");
                break;
            default:
                wcscat(si->m_lpszCPU, L"AMD Unknown (486 or 586)");
                break;
            }
        case 5:	// K5, K6
            switch (uiModel)
            {
            case 0:
            case 1:
            case 2:
            case 3:
                wcscat(si->m_lpszCPU, L"AMD K5 5k86");
                break;
            case 6:
            case 7:
                wcscat(si->m_lpszCPU, L"AMD K6");
                break;
            case 8:
                wcscat(si->m_lpszCPU, L"AMD K6-2");
                break;
            case 9:
                wcscat(si->m_lpszCPU, L"AMD K6-3");
                break;
            case 0xD:
                wcscat(si->m_lpszCPU, L"AMD K6-2+ or K6-3+");
                break;
            default:
                wcscat(si->m_lpszCPU, L"AMD Unknown (K5 or K6)");
                break;
            }
            break;
        case 6:	// K7 Athlon, Duron
            switch (uiModel)
            {
            case 1:
            case 2:
            case 4:
            case 6:
                wcscat(si->m_lpszCPU, L"AMD K-7 Athlon");
                break;
            case 3:
            case 7:
                wcscat(si->m_lpszCPU, L"AMD K-7 Duron");
                break;
            default:
                wcscat(si->m_lpszCPU, L"AMD K-7 Unknown");
                break;
            }
            break;
        default:
            wcscat(si->m_lpszCPU, L"AMD Unknown");
            break;
        }
        break;
    case 0x69727943:	// CyrixInstead
    default:
        wcscat(si->m_lpszCPU, lpszUnknown);
        break;
    }

    __int64 llFreq = GetCPUFrequency(50) / 1000000;
    swprintf(lpszTemp, L" %dMhz", (int)llFreq);
    wcscat(si->m_lpszCPU, lpszTemp);
}

typedef HRESULT(WINAPI* DIRECTDRAWCREATE)(GUID*, LPDIRECTDRAW*, IUnknown*);
typedef HRESULT(WINAPI* DIRECTDRAWCREATEEX)(GUID*, VOID**, REFIID, IUnknown*);
typedef HRESULT(WINAPI* DIRECTINPUTCREATE)(HINSTANCE, DWORD, LPDIRECTINPUT*,
    IUnknown*);

DWORD GetDXVersion()
{
    DIRECTDRAWCREATE     DirectDrawCreate = NULL;
    DIRECTDRAWCREATEEX   DirectDrawCreateEx = NULL;
    DIRECTINPUTCREATE    DirectInputCreate = NULL;
    HINSTANCE            hDDrawDLL = NULL;
    HINSTANCE            hDInputDLL = NULL;
    HINSTANCE            hD3D8DLL = NULL;
    HINSTANCE			 hD3D9DLL = NULL;
    LPDIRECTDRAW         pDDraw = NULL;
    LPDIRECTDRAW2        pDDraw2 = NULL;
    LPDIRECTDRAWSURFACE  pSurf = NULL;
    LPDIRECTDRAWSURFACE3 pSurf3 = NULL;
    LPDIRECTDRAWSURFACE4 pSurf4 = NULL;
    DWORD                dwDXVersion = 0;
    HRESULT              hr;

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

    DirectInputCreate = (DIRECTINPUTCREATE)GetProcAddress(hDInputDLL,
        "DirectInputCreateA");
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
    if (FAILED(pSurf->QueryInterface(IID_IDirectDrawSurface3,
        (VOID**)&pSurf3)))
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
    if (FAILED(pSurf->QueryInterface(IID_IDirectDrawSurface4,
        (VOID**)&pSurf4)))
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
    hr = CoCreateInstance(CLSID_DirectMusic, NULL, CLSCTX_INPROC_SERVER,
        IID_IDirectMusic, (VOID**)&pDMusic);
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
    DirectDrawCreateEx = (DIRECTDRAWCREATEEX)GetProcAddress(hDDrawDLL,
        "DirectDrawCreateEx");
    if (NULL == DirectDrawCreateEx)
    {
        FreeLibrary(hDDrawDLL);
        return dwDXVersion;
    }

    if (FAILED(DirectDrawCreateEx(NULL, (VOID**)&pDD7, IID_IDirectDraw7,
        NULL)))
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
    swprintf(si->m_lpszDxVersion, L"Direct-X %d.%d", dwDX >> 8, dwDX & 0xFF);
}

#include "stdafx.h"

#include "sysinfo.h"
#include "rdtsc.h"

bool leaf::GetOSInfoString(OUT std::wstring& osinfo)
{
    OSVERSIONINFO osi;
    ZeroMemory(&osi, sizeof(OSVERSIONINFO));
    osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&osi) == false)
        return false;

    bool bGetVersion = false;
    int iBuildNumberType = 0;
    switch (osi.dwMajorVersion)
    {
    case 3:	// NT 3.51
    {
        if (osi.dwMinorVersion == 51) {
            osinfo = L"Windows NT 3.51";
            bGetVersion = true;
        }
    }
    break;
    case 4:
    {
        switch (osi.dwMinorVersion)
        {
        case 0:
        {
            switch (osi.dwPlatformId)
            {
            case VER_PLATFORM_WIN32_WINDOWS:
            {
                osinfo = L"Windows 95";
                iBuildNumberType = 1;
                bGetVersion = true;
                if (osi.szCSDVersion[1] == 'C' || osi.szCSDVersion[1] == 'B') {
                    osinfo += L" OSR2";
                }
            }
            break;
            case VER_PLATFORM_WIN32_NT:
            {
                osinfo = L"Windows NT 4.0";
                bGetVersion = true;
            }
            break;
            }
        }
        break;
        case 10:
        {
            osinfo = L"Windows 98";
            iBuildNumberType = 1;
            bGetVersion = true;
            if (osi.szCSDVersion[1] == 'A') {
                osinfo += L" SE";
            }
        }
        break;
        case 90:
        {
            osinfo = L"Windows Me";
            iBuildNumberType = 1;
            bGetVersion = true;
        }
        break;
        }
    }
    break;
    case 5:
    {
        switch (osi.dwMinorVersion)
        {
        case 0:
        {
            osinfo = L"Windows 2000";
            bGetVersion = true;

            leaf::CRegKey RegKey;
            RegKey.SetKey(leaf::CRegKey::_HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\ProductOptions");

            std::wstring	ProductType;
            RegKey.ReadString(L"ProductType", ProductType);

            if (wcsicmp(ProductType.c_str(), L"WINNT") == 0) {
                osinfo += L" Professional";
            }
            else if (wcsicmp(ProductType.c_str(), L"LANMANNT") == 0) {
                osinfo += L" Server";
            }
            else if (wcsicmp(ProductType.c_str(), L"SERVERNT") == 0) {
                osinfo += L" Advanced Server";
            }
        }
        break;
        case 1:
        {
            osinfo = L"Windows XP";
            bGetVersion = true;
        }
        break;
        case 2:
        {
            osinfo = L"Windows 2003 family";
            bGetVersion = true;
        }
        break;
        }
    }
    break;
    }

    if (!bGetVersion) {
        wchar_t szOSVersion[128] = { 0, };
        swprintf(szOSVersion, L"Unknown %d.%d", osi.dwMajorVersion, osi.dwMinorVersion);
        osinfo = szOSVersion;
    }

    wchar_t szBuildNum[64] = { 0, };
    switch (iBuildNumberType)
    {
    case 0:
    {
        swprintf(szBuildNum, L" Build %d", osi.dwBuildNumber);
    }
    break;
    case 1:
    {
        swprintf(szBuildNum, L" Build %d.%d.%d ", HIBYTE(HIWORD(osi.dwBuildNumber)),
            LOBYTE(HIWORD(osi.dwBuildNumber)), LOWORD(osi.dwBuildNumber));
    }
    break;
    }
    osinfo += szBuildNum;

    if (osi.szCSDVersion[0]) {
        wchar_t szCSDVersion[128] = { 0, };
        swprintf(szCSDVersion, L"(%s)", osi.szCSDVersion);
        osinfo += szCSDVersion;
    }
    return true;
}
void leaf::GetCPUInfoString(OUT std::wstring& cpuinfo)
{
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
    if (iBrand) {
        wchar_t szBrand[24] = { 0, };
        *((DWORD*)szBrand) = ebxreg;
        *((DWORD*)(szBrand + 4)) = edxreg;
        *((DWORD*)(szBrand + 8)) = ecxreg;

        cpuinfo = szBrand;
        cpuinfo += L" - ";
    }
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
    {
        switch (uiFamily)
        {
        case 3:	// 386
            break;
        case 4:	// 486
            break;
        case 5:	// pentium
            break;
        case 6:	// pentium pro - pentium 3
        {
            switch (uiModel)
            {
            case 0:
            case 1:
            default:
                cpuinfo += L"Pentium Pro";
                break;
            case 3:
            case 5:
                cpuinfo += L"Pentium 2";
                break;
            case 6:
                cpuinfo += L"Pentium Celeron";
                break;
            case 7:
            case 8:
            case 0xA:
            case 0xB:
                cpuinfo += L"Pentium 3";
                break;
            }	//. switch(uiModel)
        }
        break;
        case 15:	// pentium 4
        {
            cpuinfo += L"Pentium 4";
        }
        break;
        default:
        {
            cpuinfo += L"Unknown";
        }
        break;
        }	//. switch(uiFamily)
    }
    break;
    case 0x68747541:	// AuthenticAMD
    {
        switch (uiFamily)
        {
        case 4:	// 486, 586
        {
            switch (uiModel)
            {
            case 3:
            case 7:
            case 8:
            case 9:
                cpuinfo += L"AMD 486";
                break;
            case 0xE:
            case 0xF:
                cpuinfo += L"AMD 586";
                break;
            default:
                cpuinfo += L"AMD Unknown (486 or 586)";
                break;
            }	//. switch(uiModel)
        }
        break;
        case 5:	// K5, K6
        {
            switch (uiModel)
            {
            case 0:
            case 1:
            case 2:
            case 3:
                cpuinfo += L"AMD K5 5k86";
                break;
            case 6:
            case 7:
                cpuinfo += L"AMD K6";
                break;
            case 8:
                cpuinfo += L"AMD K6-2";
                break;
            case 9:
                cpuinfo += L"AMD K6-3";
                break;
            case 0xD:
                cpuinfo += L"AMD K6-2+ or K6-3+";
                break;
            default:
                cpuinfo += L"AMD Unknown (K5 or K6)";
                break;
            }	//. switch(uiModel)
        }
        break;
        case 6:	// K7 Athlon, Duron
        {
            switch (uiModel)
            {
            case 1:
            case 2:
            case 4:
            case 6:
                cpuinfo += L"AMD K-7 Athlon";
                break;
            case 3:
            case 7:
                cpuinfo += L"AMD K-7 Duron";
                break;
            default:
                cpuinfo += L"AMD K-7 Unknown";
                break;
            }	//. switch(uiModel)
        }
        break;
        default:
        {
            cpuinfo += L"AMD Unknown";
        }
        }	//. switch(uiFamily)
    }
    break;
    case 0x69727943:	// CyrixInstead
    default:
    {
        cpuinfo = L"Unknown";
    }
    }	//. switch(iBrand)

    // ¼Óµµ
    __int64 llFreq = GetCPUFrequency(50) / 1000000;
    wchar_t szFreq[24] = { 0, };
    if (llFreq > 1000) {
        double fFreq = double(llFreq) / 1000.f;
        swprintf(szFreq, L" CPU %.2fGHz", fFreq);
    }
    else {
        swprintf(szFreq, L" CPU %dMHz", (int)llFreq);
    }
    cpuinfo += szFreq;
}
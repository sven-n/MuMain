#include "stdafx.h"
#include "PlatformInfo.h"

#ifdef _WIN32
// Intentionally not including <winternl.h>: it redeclares types already in
// <windows.h> and conflicts under MinGW. OSVERSIONINFOW (from windows.h) is
// layout-identical to RTL_OSVERSIONINFOW, so RtlGetVersion can write into it.
#else
#include <sys/utsname.h>
#include <cstdio>       // sscanf
#endif

#include <cwchar>       // swprintf

namespace Core::Platform
{

#ifdef _WIN32

std::wstring GetOSVersionString()
{
    // GetVersionEx is intercepted by the application-compatibility manifest and
    // caps at 6.2 (Windows 8) for unmanifested apps. RtlGetVersion bypasses that
    // shim and reports the true OS version, so resolve it dynamically from ntdll.
    using RtlGetVersionPtr = LONG(WINAPI*)(OSVERSIONINFOW*);

    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll == nullptr)
        return L"Windows (unknown)";

    auto pRtlGetVersion =
        reinterpret_cast<RtlGetVersionPtr>(GetProcAddress(hNtdll, "RtlGetVersion"));
    if (pRtlGetVersion == nullptr)
        return L"Windows (unknown)";

    OSVERSIONINFOW info = {};
    info.dwOSVersionInfoSize = sizeof(info);
    if (pRtlGetVersion(&info) != 0)  // 0 == STATUS_SUCCESS
        return L"Windows (unknown)";

    wchar_t buf[64];
    swprintf(buf, std::size(buf), L"Windows %lu.%lu",
             info.dwMajorVersion, info.dwMinorVersion);
    return buf;
}

#else  // ---- non-Windows ----------------------------------------------------

std::wstring GetOSVersionString()
{
    utsname uts{};
    if (uname(&uts) != 0)
        return L"Unknown OS";

    // sysname is the kernel name ("Linux"); release is like "6.18.33-...". Show
    // only the kernel major.minor to keep the overlay line short.
    int major = 0;
    int minor = 0;
    wchar_t buf[64];
    if (sscanf(uts.release, "%d.%d", &major, &minor) == 2)
        swprintf(buf, std::size(buf), L"%hs %d.%d", uts.sysname, major, minor);
    else
        swprintf(buf, std::size(buf), L"%hs", uts.sysname);
    return buf;
}

#endif // _WIN32

}

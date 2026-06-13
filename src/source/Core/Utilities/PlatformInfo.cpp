#include "stdafx.h"
#include "PlatformInfo.h"

#ifdef _WIN32
// Intentionally not including <winternl.h>: it redeclares types already in
// <windows.h> and conflicts under MinGW. OSVERSIONINFOW (from windows.h) is
// layout-identical to RTL_OSVERSIONINFOW, so RtlGetVersion can write into it.
#elif defined(__ANDROID__)
#include <sys/system_properties.h>  // __system_property_get
#elif defined(__APPLE__)
#include <TargetConditionals.h>     // TARGET_OS_IPHONE
#include <sys/sysctl.h>             // sysctlbyname
#else
#include <sys/utsname.h>
#include <cstdio>       // sscanf
#endif

#include <cwchar>       // swprintf
#include <iterator>     // std::size

// The OS name and version are fixed for the process lifetime, so each platform
// computes the string once into a function-local static. GetOSVersionString is
// called every frame from the debug overlay; caching keeps it off the hot path.

namespace Core::Platform
{

#ifdef _WIN32

std::wstring GetOSVersionString()
{
    static const std::wstring osVersion = []() -> std::wstring
    {
        // GetVersionEx is intercepted by the application-compatibility manifest
        // and caps at 6.2 (Windows 8) for unmanifested apps. RtlGetVersion
        // bypasses that shim and reports the true OS version, so resolve it
        // dynamically from ntdll.
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
    }();
    return osVersion;
}

#elif defined(__ANDROID__)

std::wstring GetOSVersionString()
{
    static const std::wstring osVersion = []() -> std::wstring
    {
        // ro.build.version.release is the user-facing release string, e.g. "14".
        char release[PROP_VALUE_MAX] = {};
        if (__system_property_get("ro.build.version.release", release) <= 0)
            return L"Android";

        wchar_t buf[64];
        swprintf(buf, std::size(buf), L"Android %s", release);
        return buf;
    }();
    return osVersion;
}

#elif defined(__APPLE__)

std::wstring GetOSVersionString()
{
    static const std::wstring osVersion = []() -> std::wstring
    {
#if TARGET_OS_IPHONE
        const char* osName = "iOS";
#else
        const char* osName = "macOS";
#endif
        // kern.osproductversion is the marketing version (e.g. "14.5"),
        // available since macOS 10.13.4 / iOS 11.
        char version[64] = {};
        size_t len = sizeof(version);
        wchar_t buf[64];
        if (sysctlbyname("kern.osproductversion", version, &len, nullptr, 0) == 0 &&
            version[0] != '\0')
            swprintf(buf, std::size(buf), L"%s %s", osName, version);
        else
            swprintf(buf, std::size(buf), L"%s", osName);
        return buf;
    }();
    return osVersion;
}

#else  // ---- generic POSIX (Linux, BSD) ------------------------------------

std::wstring GetOSVersionString()
{
    static const std::wstring osVersion = []() -> std::wstring
    {
        utsname uts{};
        if (uname(&uts) != 0)
            return L"Unknown OS";

        // sysname is the kernel name ("Linux"); release is like "6.18.33-...".
        // Show only the kernel major.minor to keep the overlay line short.
        int major = 0;
        int minor = 0;
        wchar_t buf[64];
        if (sscanf(uts.release, "%d.%d", &major, &minor) == 2)
            swprintf(buf, std::size(buf), L"%s %d.%d", uts.sysname, major, minor);
        else
            swprintf(buf, std::size(buf), L"%s", uts.sysname);
        return buf;
    }();
    return osVersion;
}

#endif // _WIN32

}

#include "stdafx.h"
#include "PlatformInfo.h"

#include <string>

#ifdef _WIN32
#include <cwchar>       // swprintf
#include <iterator>     // std::size
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

// The OS name and version are fixed for the process lifetime, so each platform
// computes the string once into a function-local static. GetOSVersionString is
// called every frame from the debug overlay; caching keeps it off the hot path.
//
// The non-Windows branches build the result with std::wstring concatenation
// rather than swprintf("%s", ...): a narrow-to-wide "%s" conversion in a wide
// printf is non-portable. OS strings are ASCII, so the byte-wise widening via
// the iterator-pair wstring constructor is safe here.

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

        const std::string releaseStr(release);
        return L"Android " + std::wstring(releaseStr.begin(), releaseStr.end());
    }();
    return osVersion;
}

#elif defined(__APPLE__)

std::wstring GetOSVersionString()
{
    static const std::wstring osVersion = []() -> std::wstring
    {
#if TARGET_OS_IPHONE
        const std::string name = "iOS";
#else
        const std::string name = "macOS";
#endif
        const std::wstring nameW(name.begin(), name.end());

        // kern.osproductversion is the marketing version (e.g. "14.5"),
        // available since macOS 10.13.4 / iOS 11.
        char version[64] = {};
        size_t len = sizeof(version);
        if (sysctlbyname("kern.osproductversion", version, &len, nullptr, 0) != 0 ||
            version[0] == '\0')
            return nameW;

        const std::string versionStr(version);
        return nameW + L" " + std::wstring(versionStr.begin(), versionStr.end());
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
        const std::string sysName(uts.sysname);
        const std::wstring sysNameW(sysName.begin(), sysName.end());

        int major = 0;
        int minor = 0;
        if (sscanf(uts.release, "%d.%d", &major, &minor) != 2)
            return sysNameW;

        return sysNameW + L" " + std::to_wstring(major) + L"." + std::to_wstring(minor);
    }();
    return osVersion;
}

#endif // _WIN32

}

// MuSystemInfo — cross-platform system info detection.
// Story 7.10.1: Extracted from ErrorReport.cpp.
// [VS0-CORE-MIGRATE-LOGGING]

#include "stdafx.h"

#include "Core/MuSystemInfo.h"

#include <cstring>
#include <fstream>
#include <string>

#include <sys/utsname.h>

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

void MuGetSystemInfo(MuSystemInfo* si)
{
    memset(si, 0, sizeof(MuSystemInfo));

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
#elif defined(__linux__)
    // Linux: read /proc/cpuinfo
    std::ifstream cpuFile("/proc/cpuinfo");
    std::string cpuLine;
    bool cpuFound = false;
    while (std::getline(cpuFile, cpuLine))
    {
        if (cpuLine.find("model name") != std::string::npos)
        {
            auto pos = cpuLine.find(':');
            if (pos != std::string::npos && pos + 2 < cpuLine.size())
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
#else
    // Windows/MinGW: CPU info not available via POSIX APIs
    wcscpy(si->m_lpszCPU, L"N/A (MinGW)");
#endif

    // RAM
#ifdef __APPLE__
    uint64_t memSize = 0;
    size_t memLen = sizeof(memSize);
    if (sysctlbyname("hw.memsize", &memSize, &memLen, nullptr, 0) == 0)
    {
        si->m_iMemorySize = static_cast<int64_t>(memSize);
    }
#elif defined(__linux__)
    // Linux: read /proc/meminfo
    std::ifstream memFile("/proc/meminfo");
    std::string memLine;
    while (std::getline(memFile, memLine))
    {
        if (memLine.find("MemTotal") != std::string::npos)
        {
            auto pos = memLine.find(':');
            if (pos != std::string::npos)
            {
                long long memKb = std::strtoll(memLine.c_str() + pos + 1, nullptr, 10);
                si->m_iMemorySize = static_cast<int64_t>(memKb * 1024);
            }
            break;
        }
    }
#else
    // Windows/MinGW: RAM info not available via POSIX APIs
    si->m_iMemorySize = 0;
#endif

    // GPU Backend — populated by caller from MuRenderer if available
    wcscpy(si->m_lpszGpuBackend, L"unknown");
}

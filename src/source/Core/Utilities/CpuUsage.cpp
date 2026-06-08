#include "stdafx.h"
#include <chrono>
#include <stdexcept>
#include "CpuUsage.h"

#ifdef _WIN32

// Implementation for Windows
class CpuUsage::Impl
{
public:
    Impl()
    {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        m_numProcessors = sysInfo.dwNumberOfProcessors;
        m_lastCheckTime = std::chrono::steady_clock::now();
        m_lastProcessTime = 0;
        m_lastSystemTime = 0;
    }

    double GetUsage() 
    {
        // Get the current process times
        FILETIME creationTime, exitTime, kernelTime, userTime;
        if (!GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime))
        {
            return 0.0; // Error
        }

        // Convert process times to ULONGLONG
        ULONGLONG currentKernelTime = FileTimeToULL(kernelTime);
        ULONGLONG currentUserTime = FileTimeToULL(userTime);
        ULONGLONG currentProcessTime = currentKernelTime + currentUserTime;

        // Get the current wall-clock time using std::chrono
        auto now = std::chrono::steady_clock::now();
        auto elapsedWallTime = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastCheckTime).count();

        // First call: Initialize
        if (m_lastSystemTime == 0 || m_lastProcessTime == 0)
        {
            m_lastSystemTime = elapsedWallTime;
            m_lastProcessTime = currentProcessTime;
            m_lastCheckTime = now;
            return 0.0; // Not enough data to calculate usage
        }

        // Calculate elapsed times
        ULONGLONG systemTimeElapsed = elapsedWallTime;
        ULONGLONG processTimeElapsed = currentProcessTime - m_lastProcessTime;

        // Update last recorded times
        m_lastSystemTime = elapsedWallTime;
        m_lastProcessTime = currentProcessTime;
        m_lastCheckTime = now;

        // Avoid division by zero
        if (systemTimeElapsed == 0 || m_numProcessors == 0)
            return 0.0;

        // Calculate CPU usage as a percentage
        return std::max<double>(0.0, (100.0 * processTimeElapsed) / (systemTimeElapsed * m_numProcessors));
    }

private:
    ULONGLONG m_lastSystemTime;
    ULONGLONG m_lastProcessTime;
    DWORD m_numProcessors;
    std::chrono::steady_clock::time_point m_lastCheckTime;

    ULONGLONG FileTimeToULL(const FILETIME& ft)
    {
        return (static_cast<ULONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    }
};

#else  // ---- non-Windows ----------------------------------------------------

#include <sys/resource.h>  // getrusage
#include <thread>          // hardware_concurrency

// POSIX implementation: process CPU time (user + system) from getrusage against
// wall-clock time, scaled by the processor count -- the same ratio the Windows
// path computes from GetProcessTimes.
class CpuUsage::Impl
{
public:
    Impl()
        : m_numProcessors(std::max(1u, std::thread::hardware_concurrency()))
        , m_lastCheckTime(std::chrono::steady_clock::now())
        , m_lastProcessTime(0)
    {
    }

    double GetUsage()
    {
        rusage ru{};
        if (getrusage(RUSAGE_SELF, &ru) != 0)
            return 0.0;

        const unsigned long long currentProcessTime =
            (static_cast<unsigned long long>(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec) * 1000000ULL) +
            static_cast<unsigned long long>(ru.ru_utime.tv_usec + ru.ru_stime.tv_usec);

        const auto now = std::chrono::steady_clock::now();
        const long long elapsedWallTime =
            std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastCheckTime).count();

        // First call: seed the baselines, no usage yet.
        if (m_lastProcessTime == 0)
        {
            m_lastProcessTime = currentProcessTime;
            m_lastCheckTime = now;
            return 0.0;
        }

        const unsigned long long processTimeElapsed = currentProcessTime - m_lastProcessTime;
        m_lastProcessTime = currentProcessTime;
        m_lastCheckTime = now;

        if (elapsedWallTime <= 0 || m_numProcessors == 0)
            return 0.0;

        return std::max<double>(0.0, (100.0 * processTimeElapsed) /
                                     (static_cast<double>(elapsedWallTime) * m_numProcessors));
    }

private:
    unsigned int m_numProcessors;
    std::chrono::steady_clock::time_point m_lastCheckTime;
    unsigned long long m_lastProcessTime;
};

#endif // _WIN32

CpuUsage* CpuUsage::Instance()
{
    static CpuUsage instance;
    return &instance;
}

// Constructor
CpuUsage::CpuUsage() : pImpl(std::make_unique<Impl>()) {}

// Destructor
CpuUsage::~CpuUsage() = default;

// Public methods
double CpuUsage::GetUsage()
{
    return pImpl->GetUsage();
}

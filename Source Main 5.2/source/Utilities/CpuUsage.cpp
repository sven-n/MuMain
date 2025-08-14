#include "stdafx.h"
#include <chrono>
#include <stdexcept>
#include "CpuUsage.h"

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
        return max(0.0, (100.0 * processTimeElapsed) / (systemTimeElapsed * m_numProcessors));
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

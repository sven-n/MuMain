#include "stdafx.h"
#include "CpuUsage.h"
#include <windows.h>
#include <stdexcept>

// Implementation for Windows
class CpuUsage::Impl 
{
public:
    Impl() : m_lastSystemTime(0), m_lastProcessTime(0) {}

    double GetUsage() 
    {
        FILETIME now, creationTime, exitTime, kernelTime, userTime;
        ULONGLONG systemTimeElapsed, processTimeElapsed;

        // Get the system time and the process times
        GetSystemTimeAsFileTime(&now);
        if (!GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime))
        {
            return -1.0; // Error
        }

        // Convert FILETIME to ULONGLONG
        ULONGLONG currentSystemTime = FileTimeToULL(now);
        ULONGLONG currentKernelTime = FileTimeToULL(kernelTime);
        ULONGLONG currentUserTime = FileTimeToULL(userTime);

        // Check if this is the first call
        if (m_lastSystemTime == 0 || m_lastProcessTime == 0) 
        {
            m_lastSystemTime = currentSystemTime;
            m_lastProcessTime = currentKernelTime + currentUserTime;
            return 0.0; // First call, no valid data to calculate usage
        }

        // Calculate elapsed times
        systemTimeElapsed = currentSystemTime - m_lastSystemTime;
        processTimeElapsed = (currentKernelTime + currentUserTime) - m_lastProcessTime;

        // Store the current times for the next call
        m_lastSystemTime = currentSystemTime;
        m_lastProcessTime = currentKernelTime + currentUserTime;

        // Avoid division by zero
        if (systemTimeElapsed == 0)
            return 0.0;

        // CPU usage as a percentage
        return (100.0 * processTimeElapsed) / systemTimeElapsed;
    }

private:
    ULONGLONG m_lastSystemTime;
    ULONGLONG m_lastProcessTime;

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

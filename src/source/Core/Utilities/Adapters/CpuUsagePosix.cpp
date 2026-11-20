// Process CPU use on POSIX systems.
#include <chrono>

#include "Core/Utilities/CpuUsage.h"

#include <algorithm>      // std::max
#include <sys/resource.h> // getrusage
#include <thread>         // hardware_concurrency

// POSIX implementation: process CPU time (user + system) from getrusage against
// wall-clock time, scaled by the processor count -- the same ratio the Windows
// path computes from GetProcessTimes.
class CpuUsage::Impl
{
public:
    Impl()
        : m_numProcessors(std::max(1u, std::thread::hardware_concurrency())),
          m_lastCheckTime(std::chrono::steady_clock::now()),
          m_lastProcessTime(~0ULL) // sentinel: "no baseline yet" (0 is a valid CPU time)
    {
    }

    double GetUsage()
    {
        rusage ru{};
        if (getrusage(RUSAGE_SELF, &ru) != 0)
            return 0.0;

        const unsigned long long currentProcessTime = (static_cast<unsigned long long>(ru.ru_utime.tv_sec) +
                                                       static_cast<unsigned long long>(ru.ru_stime.tv_sec)) *
                                                          1000000ULL +
                                                      static_cast<unsigned long long>(ru.ru_utime.tv_usec) +
                                                      static_cast<unsigned long long>(ru.ru_stime.tv_usec);

        const auto now = std::chrono::steady_clock::now();
        const long long elapsedWallTime =
            std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastCheckTime).count();

        // First call: seed the baselines, no usage yet.
        if (m_lastProcessTime == ~0ULL)
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

        return std::max<double>(0.0, processTimeElapsed / (static_cast<double>(elapsedWallTime) * m_numProcessors));
    }

private:
    unsigned int m_numProcessors;
    std::chrono::steady_clock::time_point m_lastCheckTime;
    unsigned long long m_lastProcessTime;
};

CpuUsage* CpuUsage::Instance()
{
    static CpuUsage instance;
    return &instance;
}

CpuUsage::CpuUsage() : pImpl(std::make_unique<Impl>()) {}

CpuUsage::~CpuUsage() = default;

std::optional<double> CpuUsage::GetUsage()
{
    return pImpl->GetUsage();
}

#include "stdafx.h"
#include <chrono>
#include <cstdint>
#include <thread>
#include "CpuUsage.h"
#include "MuLogger.h"

// Story 7.6.4: Cross-platform CpuUsage implementation.
// Uses mu_get_process_cpu_times() (PlatformCompat.h) for process CPU time
// and std::thread::hardware_concurrency() for core count.
// [VS0-QUAL-WIN32CLEAN-CPUUSAGE]

class CpuUsage::Impl
{
public:
    Impl()
    {
        unsigned int hw = std::thread::hardware_concurrency();
        m_numProcessors = (hw > 0) ? hw : 1;
        m_lastCheckTime = std::chrono::steady_clock::now();
        m_lastProcessTimeNs = 0;
        m_bInitialized = false;
    }

    double GetUsage()
    {
        uint64_t kernelNs = 0;
        uint64_t userNs = 0;
        if (!mu_get_process_cpu_times(&kernelNs, &userNs))
        {
            mu::log::Get("core")->warn("CpuUsage: mu_get_process_cpu_times failed -- returning 0.0");
            return 0.0;
        }

        uint64_t currentProcessTimeNs = kernelNs + userNs;

        auto now = std::chrono::steady_clock::now();
        auto wallElapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_lastCheckTime).count();

        if (!m_bInitialized)
        {
            m_lastProcessTimeNs = currentProcessTimeNs;
            m_lastCheckTime = now;
            m_bInitialized = true;
            return 0.0;
        }

        // Detect OS clock issues (backward time, cgroup changes, VM migration)
        if (currentProcessTimeNs < m_lastProcessTimeNs)
        {
            mu::log::Get("core")->warn("CpuUsage: process time went backwards -- returning 0.0");
            m_lastProcessTimeNs = currentProcessTimeNs;
            m_lastCheckTime = now;
            return 0.0;
        }

        uint64_t processTimeElapsedNs = currentProcessTimeNs - m_lastProcessTimeNs;

        m_lastProcessTimeNs = currentProcessTimeNs;
        m_lastCheckTime = now;

        // m_numProcessors is guaranteed >= 1 (constructor falls back to 1 if hardware_concurrency() is 0).
        if (wallElapsedNs <= 0)
        {
            return 0.0;
        }

        double usage =
            static_cast<double>(processTimeElapsedNs) / (static_cast<double>(wallElapsedNs) * m_numProcessors);

        if (usage < 0.0)
        {
            return 0.0;
        }
        if (usage > 1.0)
        {
            return 1.0;
        }
        return usage;
    }

private:
    uint64_t m_lastProcessTimeNs;
    unsigned int m_numProcessors;
    bool m_bInitialized;
    std::chrono::steady_clock::time_point m_lastCheckTime;
};

CpuUsage* CpuUsage::Instance()
{
    static CpuUsage instance;
    return &instance;
}

CpuUsage::CpuUsage() : pImpl(std::make_unique<Impl>()) {}

CpuUsage::~CpuUsage() = default;

double CpuUsage::GetUsage()
{
    return pImpl->GetUsage();
}

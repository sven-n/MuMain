#pragma once

#include <cstdint>
#include <memory>

namespace Core::Utilities
{
[[nodiscard]] constexpr double CalculateCpuUsageFromFileTime(std::uint64_t processTicks100ns,
                                                             std::int64_t wallMicroseconds,
                                                             unsigned int processorCount)
{
    if (wallMicroseconds <= 0 || processorCount == 0)
    {
        return 0.0;
    }

    constexpr double kFileTimeTicksPerMicrosecond = 10.0;
    const double processMicroseconds = static_cast<double>(processTicks100ns) / kFileTimeTicksPerMicrosecond;
    return processMicroseconds / (static_cast<double>(wallMicroseconds) * processorCount);
}
}

class CpuUsage
{
public:
    static CpuUsage* Instance();

    // Returns CPU utilisation as fractional ratio in [0.0, 1.0] where 1.0 = 100% of all cores.
    // NOT thread-safe — must only be called from a single thread.
    // Safe-by-design: RecordCpuUsage worker thread is the only caller.
    double GetUsage();

private:
    CpuUsage();
    ~CpuUsage();

    class Impl;                  // Forward declaration of the implementation
    std::unique_ptr<Impl> pImpl; // Pointer to implementation
};

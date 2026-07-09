#pragma once

#include <memory>

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

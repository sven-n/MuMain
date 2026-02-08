#pragma once

#include <memory>

class CpuUsage {
public:
    static CpuUsage* Instance();

    double GetUsage();

private:
    CpuUsage();
    ~CpuUsage();

    class Impl;                  // Forward declaration of the implementation
    std::unique_ptr<Impl> pImpl; // Pointer to implementation
};

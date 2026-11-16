// For a target that can't measure process CPU time: every sample is empty.
#include "Core/Utilities/CpuUsage.h"

class CpuUsage::Impl
{
public:
    std::optional<double> GetUsage()
    {
        return std::nullopt;
    }
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

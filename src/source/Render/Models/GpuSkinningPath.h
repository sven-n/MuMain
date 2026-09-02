#pragma once

namespace Render::Models
{
enum class GpuSkinningPath
{
    CpuIneligible,
    GpuSubmitted,
    GpuFailed,
};

[[nodiscard]] constexpr GpuSkinningPath ResolveGpuSkinningPath(bool eligible, bool submitted)
{
    if (!eligible)
        return GpuSkinningPath::CpuIneligible;
    return submitted ? GpuSkinningPath::GpuSubmitted : GpuSkinningPath::GpuFailed;
}
}

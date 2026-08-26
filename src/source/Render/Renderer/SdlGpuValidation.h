#pragma once

namespace Render
{
[[nodiscard]] constexpr bool GpuValidationEnabled(bool assertionsDisabled)
{
    return !assertionsDisabled;
}

#ifdef NDEBUG
inline constexpr bool kGpuValidationEnabled = GpuValidationEnabled(true);
#else
inline constexpr bool kGpuValidationEnabled = GpuValidationEnabled(false);
#endif
}

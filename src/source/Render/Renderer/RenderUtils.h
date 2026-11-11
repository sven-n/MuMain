#pragma once

#include <cstdint>

namespace mu
{

[[nodiscard]] inline std::uint32_t PackABGR(float r, float g, float b, float a) noexcept
{
    const float clampedR = r < 0.f ? 0.f : (r > 1.f ? 1.f : r);
    const float clampedG = g < 0.f ? 0.f : (g > 1.f ? 1.f : g);
    const float clampedB = b < 0.f ? 0.f : (b > 1.f ? 1.f : b);
    const float clampedA = a < 0.f ? 0.f : (a > 1.f ? 1.f : a);
    return (static_cast<std::uint32_t>(clampedA * 255.f) << 24) |
           (static_cast<std::uint32_t>(clampedB * 255.f) << 16) |
           (static_cast<std::uint32_t>(clampedG * 255.f) << 8) |
           static_cast<std::uint32_t>(clampedR * 255.f);
}

} // namespace mu

#pragma once

#include <cstdint>

namespace mu
{

[[nodiscard]] inline std::uint32_t PackABGR(float r, float g, float b, float a) noexcept
{
    auto clamp01 = [](float v) noexcept -> float { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); };
    return (static_cast<std::uint32_t>(clamp01(a) * 255.f) << 24) |
           (static_cast<std::uint32_t>(clamp01(b) * 255.f) << 16) |
           (static_cast<std::uint32_t>(clamp01(g) * 255.f) << 8) | static_cast<std::uint32_t>(clamp01(r) * 255.f);
}

} // namespace mu

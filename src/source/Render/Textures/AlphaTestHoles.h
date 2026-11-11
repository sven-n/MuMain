#pragma once

#include <cstdint>
#include <span>

namespace Render::Textures
{
// The alpha test drops the pixels whose alpha is at or below 0.25 (EnableAlphaTest); as a byte that is 63 and below.
inline constexpr std::uint8_t AlphaTestDropsAtOrBelow = 63;

// Whether an RGBA image has a pixel the alpha test drops, a hole in a mesh drawn with it. The buffer's rows are
// `rowPixels` wide, which can be more than the image's `width` (a power-of-two buffer); the padding is not looked at.
[[nodiscard]] bool HasAlphaTestHoles(std::span<const std::uint8_t> rgba, int width, int height, int rowPixels);
} // namespace Render::Textures

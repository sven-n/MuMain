// SDLTtfColorPack.h — Story 7.9.8
// ABGR color packing for SDL_ttf text rendering.
// Byte layout: [7:0]=R, [15:8]=G, [23:16]=B, [31:24]=A — matches SDL_GPU vertex colors.
#pragma once

#include <cstdint>

namespace mu
{
namespace sdlttf
{

[[nodiscard]] constexpr std::uint32_t PackColorDWORD(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
{
    return (static_cast<std::uint32_t>(a) << 24)
         | (static_cast<std::uint32_t>(b) << 16)
         | (static_cast<std::uint32_t>(g) << 8)
         |  static_cast<std::uint32_t>(r);
}

} // namespace sdlttf
} // namespace mu

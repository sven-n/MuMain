#pragma once

#include <cstdint>

namespace mu::sdlttf
{

[[nodiscard]] constexpr std::uint32_t PackColorDWORD(std::uint8_t red, std::uint8_t green, std::uint8_t blue,
                                                     std::uint8_t alpha) noexcept
{
    return (static_cast<std::uint32_t>(alpha) << 24) | (static_cast<std::uint32_t>(blue) << 16) |
           (static_cast<std::uint32_t>(green) << 8) | static_cast<std::uint32_t>(red);
}

} // namespace mu::sdlttf

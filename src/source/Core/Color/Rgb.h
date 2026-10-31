#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace Core::Color
{
// An sRGB color with 8 bits a channel.
struct Rgb
{
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;

    bool operator==(const Rgb&) const = default;
};

// "#RRGGBB" (either case) and back ("#RRGGBB" in capitals); anything else is nothing.
std::optional<Rgb> ParseRgb(std::string_view text);
std::string FormatRgb(Rgb color);
} // namespace Core::Color

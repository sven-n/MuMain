#include "stdafx.h"

#include "Rgb.h"

#include <array>
#include <charconv>
#include <cstdio>

namespace Core::Color
{
namespace
{
constexpr size_t ShadeLength = 7; // "#RRGGBB"
constexpr size_t ChannelDigits = 2;
constexpr int HexBase = 16;

std::optional<std::uint8_t> ParseChannel(std::string_view digits)
{
    unsigned int value = 0;
    const char* end = digits.data() + digits.size();
    const auto [parsedEnd, error] = std::from_chars(digits.data(), end, value, HexBase);
    if (error != std::errc{} || parsedEnd != end)
    {
        return std::nullopt;
    }
    return static_cast<std::uint8_t>(value);
}
} // namespace

std::optional<Rgb> ParseRgb(std::string_view text)
{
    if (text.size() != ShadeLength || text.front() != '#')
    {
        return std::nullopt;
    }
    std::array<std::uint8_t, 3> channels{};
    for (size_t index = 0; index < channels.size(); ++index)
    {
        const std::optional<std::uint8_t> channel = ParseChannel(text.substr(1 + index * ChannelDigits, ChannelDigits));
        if (!channel)
        {
            return std::nullopt;
        }
        channels[index] = *channel;
    }
    return Rgb{channels[0], channels[1], channels[2]};
}

std::string FormatRgb(Rgb color)
{
    std::array<char, ShadeLength + 1> text{};
    std::snprintf(text.data(), text.size(), "#%02X%02X%02X", color.red, color.green, color.blue);
    return text.data();
}
} // namespace Core::Color

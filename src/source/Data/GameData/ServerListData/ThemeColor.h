#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace Data::ServerList
{
// A button color every theme (legacy, modern, ...) draws in a shade of its own.
enum class ThemeColor : std::uint8_t
{
    Default,
    Gray,
    Orange,
    Silver,
};

inline constexpr int ThemeColorCount = 4;

// The name of a theme color in the file ("orange"), and back.
const char* GetThemeColorName(ThemeColor color);
std::optional<ThemeColor> FindThemeColor(std::string_view name);
// The token in a theme's tokens.ini that holds the color's shade ("server-button-orange"), as
// the server select screen's stylesheets use it.
const char* GetThemeColorToken(ThemeColor color);
} // namespace Data::ServerList

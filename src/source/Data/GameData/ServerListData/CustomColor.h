#pragma once

#include <functional>
#include <map>
#include <string>

namespace Data::ServerList
{
// A color's name in UTF-16 units.
inline constexpr int MaxCustomColorNameLength = 32;

// A button color of the list's own, with a shade for each theme.
struct CustomColor
{
    // What rules name it by ("goldRed"; IsListId), none of the theme colors' names.
    std::string id;
    // What the editor calls it.
    std::string name;
    // "#RRGGBB" by theme folder ("legacy", "modern"); a theme without one draws its default color.
    std::map<std::string, std::string, std::less<>> shades;

    bool operator==(const CustomColor&) const = default;
};
} // namespace Data::ServerList

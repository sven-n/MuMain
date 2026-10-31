#include "stdafx.h"

#include "ThemeColor.h"

#include <algorithm>
#include <array>

namespace Data::ServerList
{
namespace
{
// In the order of ThemeColor.
constexpr std::array<const char*, ThemeColorCount> ThemeColorNames = {"default", "gray", "orange", "silver"};
} // namespace

const char* GetThemeColorName(ThemeColor color)
{
    return ThemeColorNames[static_cast<size_t>(color)];
}

std::optional<ThemeColor> FindThemeColor(std::string_view name)
{
    const auto found = std::find(ThemeColorNames.begin(), ThemeColorNames.end(), name);
    if (found == ThemeColorNames.end())
    {
        return std::nullopt;
    }
    return static_cast<ThemeColor>(std::distance(ThemeColorNames.begin(), found));
}
} // namespace Data::ServerList

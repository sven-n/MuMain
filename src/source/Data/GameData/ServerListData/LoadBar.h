#pragma once

#include "Core/Color/Rgb.h"
#include "Data/GameData/ServerListData/ServerListData.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

// The load bar of a channel's button: ten segments from the list's base color toward its full
// color (docs/server-list.md).
namespace Data::ServerList
{
inline constexpr int LoadBarSegmentCount = 10;

// One segment of the load bar.
struct LoadBarSegment
{
    // How much of it is lit, from 0 to 1.
    float lit = 0.0f;
    Core::Color::Rgb color;

    bool operator==(const LoadBarSegment&) const = default;
};

using LoadBarSegments = std::array<LoadBarSegment, LoadBarSegmentCount>;

// The parts of the bar that have a color of their own.
enum class LoadBarPart : std::uint8_t
{
    Base,
    Full,
    // The unlit part of a segment.
    Empty,
};

// The segments for a load in percent. Segment k (from 0) is `base` moved k tenths of the way to
// `full` (Core::Color::MixOklch) and lit by as much of its tenth of the load as is reached; a full
// or overloaded server has every segment lit in `full`.
LoadBarSegments GetLoadBarSegments(Core::Color::Rgb base, Core::Color::Rgb full, int percent);

// The token of a theme that holds a part's own color ("server-load-base").
const char* GetLoadBarToken(LoadBarPart part);
// The part's color in the list: a theme color's name or a CustomColor's id.
const std::string& GetLoadBarColor(const LoadBarColors& colors, LoadBarPart part);
std::string& GetLoadBarColor(LoadBarColors& colors, LoadBarPart part);

// Reads a token of a theme ("#00BD8B"); nothing for one the theme does not have.
using ThemeTokenReader = std::function<std::optional<std::string>(const char* token)>;

// Whether the part is drawn in the theme's own color for it: "default", or a color of the
// list's own without a shade for the theme folder `theme`.
bool IsThemeLoadBarColor(const ServerListData& list, LoadBarPart part, std::string_view theme);
// What the part is drawn in, in the theme folder `theme`: a shade of the list's own, or the
// theme's token for the color. Base and Full fall back to built-in colors when the token is
// no "#RRGGBB"; Empty is nothing then.
std::optional<Core::Color::Rgb> ResolveLoadBarColor(const ServerListData& list, LoadBarPart part,
                                                    std::string_view theme, const ThemeTokenReader& readToken);
// The same for `color` as the part's color (a theme color's name or a CustomColor's id).
std::optional<Core::Color::Rgb> ResolveLoadBarColor(const ServerListData& list, std::string_view color,
                                                    LoadBarPart part, std::string_view theme,
                                                    const ThemeTokenReader& readToken);
} // namespace Data::ServerList

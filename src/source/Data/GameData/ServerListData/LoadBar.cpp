#include "stdafx.h"

#include "LoadBar.h"

#include "ButtonColors.h"
#include "Core/Color/OklchMix.h"

#include <algorithm>
#include <array>

namespace Data::ServerList
{
namespace
{
using Core::Color::Rgb;

// In the order of LoadBarPart.
constexpr std::array<const char*, 3> LoadBarTokens = {"server-load-base", "server-load-full", "server-load-empty"};

// For a theme whose token is missing or no "#RRGGBB": the legacy theme's.
constexpr Rgb BuiltInBase{0x00, 0xBD, 0x8B};
constexpr Rgb BuiltInFull{0xE0, 0x48, 0x3E};

constexpr float PercentPerSegment = 100.0f / LoadBarSegmentCount;
} // namespace

LoadBarSegments GetLoadBarSegments(Rgb base, Rgb full, int percent)
{
    LoadBarSegments segments{};
    if (GetLoadState(percent) != LoadState::Open)
    {
        segments.fill(LoadBarSegment{1.0f, full});
        return segments;
    }
    const float reached = std::max(percent, 0) / PercentPerSegment;
    for (int index = 0; index < LoadBarSegmentCount; ++index)
    {
        LoadBarSegment& segment = segments[static_cast<size_t>(index)];
        segment.color = Core::Color::MixOklch(base, full, static_cast<float>(index) / LoadBarSegmentCount);
        segment.lit = std::clamp(reached - static_cast<float>(index), 0.0f, 1.0f);
    }
    return segments;
}

CoveredSegments GetCoveredSegments(std::span<const SegmentSpan> segments, float left, float right)
{
    // Edges that only touch, within layout rounding, don't cover.
    constexpr float Touch = 0.01f;
    CoveredSegments covered;
    bool found = false;
    for (size_t index = 0; index < segments.size(); ++index)
    {
        const SegmentSpan& segment = segments[index];
        if (segment.left + segment.width <= left + Touch || segment.left >= right - Touch)
        {
            continue;
        }
        if (!found)
        {
            covered.first = static_cast<int>(index);
            found = true;
        }
        covered.end = static_cast<int>(index) + 1;
    }
    return covered;
}

const char* GetLoadBarToken(LoadBarPart part)
{
    return LoadBarTokens[static_cast<size_t>(part)];
}

const std::string& GetLoadBarColor(const LoadBarColors& colors, LoadBarPart part)
{
    switch (part)
    {
    case LoadBarPart::Full:
        return colors.full;
    case LoadBarPart::Empty:
        return colors.empty;
    case LoadBarPart::Base:
    default:
        return colors.base;
    }
}

std::string& GetLoadBarColor(LoadBarColors& colors, LoadBarPart part)
{
    switch (part)
    {
    case LoadBarPart::Full:
        return colors.full;
    case LoadBarPart::Empty:
        return colors.empty;
    case LoadBarPart::Base:
    default:
        return colors.base;
    }
}

bool IsThemeLoadBarColor(const ServerListData& list, LoadBarPart part, std::string_view theme)
{
    const ButtonColor color = ResolveButtonColor(list, GetLoadBarColor(list.load.bar, part), theme);
    return color.shade.empty() && color.theme == ThemeColor::Default;
}

std::optional<Rgb> ResolveLoadBarColor(const ServerListData& list, LoadBarPart part, std::string_view theme,
                                       const ThemeTokenReader& readToken)
{
    return ResolveLoadBarColor(list, GetLoadBarColor(list.load.bar, part), part, theme, readToken);
}

std::optional<Rgb> ResolveLoadBarColor(const ServerListData& list, std::string_view colorName, LoadBarPart part,
                                       std::string_view theme, const ThemeTokenReader& readToken)
{
    const ButtonColor color = ResolveButtonColor(list, colorName, theme);
    if (!color.shade.empty())
    {
        return Core::Color::ParseRgb(color.shade);
    }
    const char* token = color.theme == ThemeColor::Default ? GetLoadBarToken(part) : GetThemeColorToken(color.theme);
    const std::optional<std::string> value = readToken(token);
    if (const std::optional<Rgb> parsed = value ? Core::Color::ParseRgb(*value) : std::nullopt)
    {
        return parsed;
    }
    switch (part)
    {
    case LoadBarPart::Base:
        return BuiltInBase;
    case LoadBarPart::Full:
        return BuiltInFull;
    case LoadBarPart::Empty:
    default:
        return std::nullopt;
    }
}
} // namespace Data::ServerList

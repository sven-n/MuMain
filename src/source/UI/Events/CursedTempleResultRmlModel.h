#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One of the result window's own centred lines. Where it sits and what colour it is are the
// theme's; only what it says and the size it was shrunk to for its box travel.
struct CursedTempleResultLine
{
    Rml::String text;
    float textPx = 0.f;

    bool operator==(const CursedTempleResultLine&) const = default;
};

// One player's row of the result table: which side they fought for, who they are and what they
// came away with. The hero's own row is marked so the theme can pick it out.
struct CursedTempleResultRow
{
    Rml::String team;
    Rml::String name;
    Rml::String className;
    Rml::String addedExp;
    Rml::String point;
    bool hero = false;

    bool operator==(const CursedTempleResultRow&) const = default;
};

struct CursedTempleResultRmlModel
{
    float textPx = 0.f;       // native normal text size in physical px (RmlNativeTextSize.h)
    float lineHeightPx = 0.f; // native normal line height, physical px (the hero row's height)

    CursedTempleResultLine heroListLabel;
    CursedTempleResultLine columnHeader;
    // The same headings one by one, for a theme that places each over its own column.
    Rml::String campLabel, characterLabel, classLabel, expLabel, pointLabel;
    CursedTempleResultLine rewardHint;
    // The two teams' rows, each block starting at its own row in the theme.
    std::vector<CursedTempleResultRow> alliedRows;
    std::vector<CursedTempleResultRow> illusionRows;
    Rml::String closeText;
    // CButton::Render(): 23 / 2 - h / 2 whole units down, the native line height in physical px.
    float labelLinePx = 0.f;

    // The victory / defeat banner fading in above the window: 0 none, 1 success, 2 failure.
    int banner = 0;
    float bannerAlpha = 0.f;
};
} // namespace mu::ui::window

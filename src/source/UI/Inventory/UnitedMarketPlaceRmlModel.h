#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
struct UnitedMarketPlaceLineEntry
{
    Rml::String text;
    float top = 0.f; // reference px in the panel
};

struct UnitedMarketPlaceRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;     // native normal text size in physical px (RmlRootTransform.h)
    float boldTextPx = 0.f; // native bold text size

    Rml::String title;
    std::vector<UnitedMarketPlaceLineEntry> lines;
    Rml::String warpText;
    // CButton::Render(): the label centred on its native line height, 23 / 2 - h / 2 whole units
    // down (reference px), its line box in physical px.
    float labelTop = 0.f;
    float labelLinePx = 0.f;
    bool warpLocked = false; // the Warp button after a click, or while a remaining time is set
    Rml::String exitTooltip;
};
} // namespace mu::ui::window

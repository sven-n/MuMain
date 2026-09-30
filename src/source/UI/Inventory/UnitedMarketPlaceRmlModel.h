#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
struct UnitedMarketPlaceRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;     // native normal text size in physical px (RmlRootTransform.h)
    float boldTextPx = 0.f; // native bold text size

    Rml::String title;
    // The description's seven slots, in order; a slot the current place has nothing to say in is
    // empty rather than absent, so the theme can keep placing the ones after it.
    std::vector<Rml::String> lines;
    Rml::String warpText;
    // CButton::Render(): the label centred on its native line height, 23 / 2 - h / 2 whole units
    // down (reference px), its line box in physical px.
    float labelTop = 0.f;
    float labelLinePx = 0.f;
    bool warpLocked = false; // the Warp button after a click, or while a remaining time is set
    Rml::String exitTooltip;
};
} // namespace mu::ui::window

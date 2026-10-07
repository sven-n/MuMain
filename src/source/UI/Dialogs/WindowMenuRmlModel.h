#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
struct WindowMenuRowEntry
{
    Rml::String label;
    int index = 0; // CWindowMenu::MenuEntry
};

struct WindowMenuRmlModel
{
    // The Hud layout's W/640 x H/480 stretch (UI::Scaling::GetActiveTransform() while CManager
    // runs this window) and its inverse for the counter-scaled text leaves.
    float scaleX = 1.f, scaleY = 1.f;
    float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)

    std::vector<WindowMenuRowEntry> rows;
};
} // namespace mu::ui::window

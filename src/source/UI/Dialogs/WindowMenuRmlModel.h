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
    // The HUD board (LayoutMode::HudBoard while CManager runs the window): the 640x480 frame's
    // offset in physical px and its one scale, cancelled by the counter-scaled text leaves.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)

    std::vector<WindowMenuRowEntry> rows;
};
} // namespace mu::ui::window

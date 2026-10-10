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
    float textPx = 0.f; // native text size in physical px (RmlNativeTextSize.h)

    std::vector<WindowMenuRowEntry> rows;
};
} // namespace mu::ui::window

#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
struct QuickCommandRowEntry
{
    Rml::String label;
};

struct QuickCommandRmlModel
{
    float textPx = 0.f;     // native normal text size in physical px (RmlNativeTextSize.h)
    float boldTextPx = 0.f; // native bold text size (the player's name)

    Rml::String targetName;
    std::vector<QuickCommandRowEntry> rows;
};
} // namespace mu::ui::window

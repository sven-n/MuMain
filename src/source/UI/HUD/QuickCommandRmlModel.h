#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
struct QuickCommandRowEntry
{
    Rml::String label;
    bool selected = false; // the row under the pointer (CQuickCommandWindow::SelectedCommandIndex())
};

struct QuickCommandRmlModel
{
    // Dialog layout -- UI::Scaling::GetActiveTransform() while CManager runs this window; the
    // menu's own top-left (m_Pos, set at the pointer when it opens) goes through it too.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;     // native normal text size in physical px (RmlRootTransform.h)
    float boldTextPx = 0.f; // native bold text size (the player's name)

    Rml::String targetName;
    std::vector<QuickCommandRowEntry> rows;
};
} // namespace mu::ui::window

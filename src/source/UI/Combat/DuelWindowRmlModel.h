#pragma once

#include <RmlUi/Core/Types.h>

namespace mu::ui::window
{
struct DuelWindowRmlModel
{
    // The Hud layout's W/640 x H/480 stretch (UI::Scaling::GetActiveTransform() while CManager
    // runs this window) and its inverse for the counter-scaled text leaves.
    float scaleX = 1.f, scaleY = 1.f;
    float inverseScaleX = 1.f, inverseScaleY = 1.f;
    float boldTextPx = 0.f; // native bold text size in physical px

    // The window's top-left, reference px (CDuelWindow::m_Pos).
    float panelX = 0.f, panelY = 0.f;

    Rml::String heroName, heroScore, enemyName, enemyScore;
};
} // namespace mu::ui::window

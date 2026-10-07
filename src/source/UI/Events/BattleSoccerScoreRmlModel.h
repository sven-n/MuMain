#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One team's line: score, guild mark (64 cell colours, empty: none drawn) and name.
struct BattleSoccerTeamEntry
{
    bool red = false; // (255, 60, 0), else blue (0, 150, 255)
    Rml::String score;
    Rml::String name;
    std::vector<Rml::String> mark;
};

struct BattleSoccerScoreRmlModel
{
    // The Hud layout's W/640 x H/480 stretch (UI::Scaling::GetActiveTransform() while CManager
    // runs this window) and its inverse for the counter-scaled text leaves.
    float scaleX = 1.f, scaleY = 1.f;
    float boldTextPx = 0.f; // native bold text size in physical px

    // The window's top-left, reference px (CBattleSoccerScore::m_Pos).
    float panelX = 0.f, panelY = 0.f;

    // Top line first; empty when neither a guild war nor a spectated match is on.
    std::vector<BattleSoccerTeamEntry> teams;
};
} // namespace mu::ui::window

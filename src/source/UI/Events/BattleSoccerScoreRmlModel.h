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
    float boldTextPx = 0.f; // native bold text size in physical px

    // Top line first; empty when neither a guild war nor a spectated match is on.
    std::vector<BattleSoccerTeamEntry> teams;
};
} // namespace mu::ui::window

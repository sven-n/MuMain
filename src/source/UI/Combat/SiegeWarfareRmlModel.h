#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// A 3 x 3 dot on the siege mini map, reference px from the frame's top-left (not clipped to the
// map, like the original's RenderColor()). Every position of the model is from that corner.
struct SiegeWarDotEntry
{
    float left = 0.f, top = 0.f;

    bool operator==(const SiegeWarDotEntry&) const = default;
};

// A team's command (attack / defence / wait) on the mini map: the team number and the command
// icon, pulsing red for a new one (CSiegeWarBase::RenderCmdIconInMiniMap()).
struct SiegeWarCommandEntry
{
    float left = 0.f, top = 0.f; // the command's map point
    int command = 0;             // 0 attack, 1 defence, 2 wait
    Rml::String team;            // "1".."7"
    // The pulse a new command fades through, 1 + sin(lifetime * 0.2) as an rgb(): the red is the
    // theme's but the ramp is per-frame, and RCSS cannot mix a bound fraction into a colour, so
    // the two stay welded here. The geometry guard deliberately does not cover `color`.
    Rml::String color;

    bool operator==(const SiegeWarCommandEntry&) const = default;
};

// One of the commander's buttons: the seven team buttons, and the three command buttons beside
// the chosen team. A chosen team shows its pressed art.
struct SiegeWarButtonEntry
{
    bool selected = false;
    Rml::String label;

    bool operator==(const SiegeWarButtonEntry&) const = default;
};

struct SiegeWarfareRmlModel
{
    float boldTextPx = 0.f, bigTextPx = 0.f; // native text sizes in physical px

    float alpha = 1.f;                // the transparency button's value (0.5 .. 1)
    Rml::String mapRect;              // the shown part of World31/Map1, texture px "x y w h"
    Rml::String alphaLabel;

    bool timeVisible = false; // the siege's remaining time, only while a siege runs
    Rml::String timeText;

    std::vector<SiegeWarDotEntry> dots; // everyone else in view, and the commander's guild members
    float heroLeft = 0.f, heroTop = 0.f;
    std::vector<SiegeWarCommandEntry> commands;

    // The battle-skill frame (guild master / sub master / battle master during a siege).
    bool skillVisible = false;
    Rml::String skillRect; // the skill's cell in newui_skill2, texture px "x y w h"
    // Whether the hero has the kills the skill needs: the original reddened the icon until then.
    bool skillAffordable = false;
    Rml::String killsNeeded, kills;

    // The commander's team buttons, the command buttons for the chosen team (0-based), and the
    // chosen command under the pointer.
    std::vector<SiegeWarButtonEntry> teams;
    bool ordersVisible = false;
    int chosenTeam = -1;
    bool cursorVisible = false;
    float cursorLeft = 0.f, cursorTop = 0.f;
    int cursorCommand = 0;
    Rml::String cursorTeam;
};
} // namespace mu::ui::window

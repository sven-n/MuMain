#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One gauge piece of the spectator frame: the part of a menu_pk bar texture from texel 0,
// `rect` ("0 0 w h", texels), stretched over `width` x `height` reference px at (`left`, `top`);
// the left fighter's health bar is drawn mirrored.
struct DuelWatchGaugeEntry
{
    Rml::String src; // the bar texture, relative to the themed document
    Rml::String rect;
    float left = 0.f, top = 0.f;
    float width = 0.f, height = 0.f;
    bool mirrored = false;
};

// One fighter's name under the gauges: centred on 55 units, bold, shrunk to them. Each theme puts
// it on its own side.
struct DuelWatchNameEntry
{
    Rml::String text;
    float textPx = 0.f;

    bool operator==(const DuelWatchNameEntry&) const = default;
};

// The spectator frame (CDuelWatchMainFrameWindow).
struct DuelWatchFrameRmlModel
{
    // The Hud layout's W/640 x H/480 stretch and its inverse for the counter-scaled names.
    float scaleX = 1.f, scaleY = 1.f;

    // A watched channel: the names, score marks and gauges; the frame and the exit button always.
    bool watching = false;
    Rml::String exitHint;
    DuelWatchNameEntry heroName, enemyName;
    std::vector<float> scoreMarks; // the marks' left edges, reference px
    std::vector<DuelWatchGaugeEntry> gauges;
};

// One spectator's box in the list (CDuelWatchUserListWindow).
struct DuelWatchSpectatorEntry
{
    Rml::String name;
    float top = 0.f; // the box's top, reference px
};

struct DuelWatchSpectatorsRmlModel
{
    float scaleX = 1.f, scaleY = 1.f;

    float panelX = 0.f;  // the list's left edge, reference px (m_Pos.x)
    float textPx = 0.f;  // the native normal text size, physical px
    float textTop = 0.f; // the name's top in its box, reference px
    std::vector<DuelWatchSpectatorEntry> spectators;
};
} // namespace mu::ui::window

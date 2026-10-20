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
    // A watched channel: the names, score marks and gauges; the frame and the exit button always.
    bool watching = false;
    Rml::String exitHint;
    DuelWatchNameEntry heroName, enemyName;
    // One entry per point; only the count matters, the theme lays out the marks.
    std::vector<int> heroMarks, enemyMarks;
    std::vector<DuelWatchGaugeEntry> gauges;
};

// The spectator list (CDuelWatchUserListWindow).
struct DuelWatchSpectatorsRmlModel
{
    float textPx = 0.f; // the native normal text size, physical px
    std::vector<Rml::String> spectators; // the first one on top
};
} // namespace mu::ui::window

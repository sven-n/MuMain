#pragma once

#include <RmlUi/Core/Types.h>

namespace UI::Hud
{
// The short status texts over the world above the HUD (hud_status.rml): what each says, and the
// macro cooldown's fill. The theme places and colours them.
struct HudStatusRmlModel
{
    float textPx = 0.f; // the native text size in physical px

    // The siege crown switches' holders, one line per switch; empty while nobody holds it.
    Rml::String switchFirst, switchSecond;

    // A chat macro's cooldown: shown while it runs, its bar emptying from 1 to 0, centred on the
    // world the open docks leave uncovered (worldCentre, the HUD board's reference px).
    bool macroVisible = false;
    float macroFraction = 0.f;
    float worldCentre = 320.f;

    // An event's entry countdown (Blood Castle, Chaos Castle, Devil Square and the rest).
    Rml::String countdown;
};
} // namespace UI::Hud

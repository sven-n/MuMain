#pragma once

#include <RmlUi/Core/Types.h>

namespace UI::Hud
{
// A guild war's or battle soccer's time and result (match_status.rml). The theme places and
// colours them.
struct MatchStatusRmlModel
{
    float textPx = 0.f;    // the native text sizes in physical px
    float bigTextPx = 0.f;

    // The time left, while ten minutes or less remain; "soon" in the last minute or while the
    // match is about to start.
    Rml::String countdown;
    bool countdownSoon = false;
    float worldCentre = 320.f; // the uncovered world's centre, the HUD board's reference px

    // The result panel, until OK closes it.
    bool resultShown = false;
    Rml::String title, vs, team1, team2, score1, score2, outcome1, outcome2, tieText;
    bool tie = false;
    bool team1Won = false;
};
} // namespace UI::Hud

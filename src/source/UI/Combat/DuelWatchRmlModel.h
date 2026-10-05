#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One colosseum of the list: a running duel's two players, or "No duel on.", and its Watch button
// (enabled only for a running duel open to spectators).
struct DuelWatchRoomEntry
{
    int index = 0;
    Rml::String heading; // "Colosseum # n"
    bool running = false;
    Rml::String player1, player2;
    // The names' sizes: the native one, shrunk to fit their 70-unit boxes as RenderText() did.
    float player1Px = 0.f, player2Px = 0.f;
    bool joinable = false;
};

struct DuelWatchRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;     // native normal text size in physical px (RmlRootTransform.h)
    float boldTextPx = 0.f; // native bold text size

    Rml::String title;      // Doorkeeper Titus
    Rml::String subtitle;   // Select a colosseum ...
    float subtitlePx = 0.f; // bold, shrunk to its 190-unit box
    Rml::String vsText;
    Rml::String noDuelText;
    Rml::String watchText;
    // CButton::Render(): the label centred on its native line height, 23 / 2 - h / 2 whole units
    // down (reference px), its line box in physical px.
    float labelLinePx = 0.f;
    std::vector<DuelWatchRoomEntry> rooms;
};
} // namespace mu::ui::window

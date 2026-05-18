#pragma once

#ifdef _EDITOR

// Namespace lives under MuEditor::* (not UI::*) because this code is editor-
// only - it never ships in a player build, depends on ImGui rather than the
// game's legacy renderer, and shares no game-state assumptions. The game's
// player-facing skill tooltip stays in UI::Skills::Tooltip; the two
// renderers consume the same Model but live in different layer namespaces.
namespace MuEditor::Skills::Tooltip
{
    // Renders an ImGui tooltip with skill information for editor row hovers.
    // Mirrors the player hover tooltip's content (name, costs, requirements,
    // mastery type) using raw BMD values - no character context, no class-
    // specific damage calculations or red/green requirement-met coloring.
    //
    // Caller is responsible for the hover check; this unconditionally opens a
    // tooltip via ImGui::BeginTooltip/EndTooltip.
    void Render(int skillIndex);
}

#endif // _EDITOR

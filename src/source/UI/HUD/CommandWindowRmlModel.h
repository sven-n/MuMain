#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One of the twelve command buttons. `selected` is the armed command: the original drew its
// button pressed (frame 2) with a bold label until the command ran or was cancelled.
struct CommandButtonEntry
{
    Rml::String label;
    int index = 0; // COMMAND_TYPE
    bool selected = false;
    // CButton::Render() centred the label on the button with the native text's own height:
    // its top in reference px, its line box and size in physical px.
    float labelTop = 0.f;
    float labelLinePx = 0.f;
    float labelTextPx = 0.f;
};

struct CommandWindowRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;    // native normal text size in physical px (RmlRootTransform.h)
    float bigTextPx = 0.f; // native big text size (the target name at the pointer)
    // The title, bold, shrunk to its 72-unit box like the original's, on the native line height.
    float titleTextPx = 0.f;
    float titleLinePx = 0.f;

    Rml::String titleText;
    Rml::String exitTooltip;
    std::vector<CommandButtonEntry> buttons;

    // The target box the original drew at the pointer while a command is armed and a player is
    // under it: reference px relative to the panel, like everything else in it.
    bool targetVisible = false;
    float targetLeft = 0.f;
    float targetTop = 0.f;
    Rml::String targetName;
    bool targetInRange = false; // white name; red when the command cannot reach the player
};
} // namespace mu::ui::window

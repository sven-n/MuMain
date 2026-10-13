#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One party member's card. The flags are the meaning behind the original's tints and text
// colours; each theme owns the colours.
struct PartyListCardEntry
{
    Rml::String name;
    float nameTextPx = 0.f; // the native size, shrunk to the name box like the original's
    bool leader = false;    // first card: flag, orange name
    bool absent = false;    // not on this map (Party[].index == -1): red tint, dimmed name
    bool defenseBuff = false;
    bool selected = false; // the card under the pointer, the skill target
    bool showLeave = false;
    float healthLength = 0.f; // reference px of the health bar's texture shown
    int index = 0;            // position in Party[]
};

struct PartyListRmlModel
{
    std::vector<PartyListCardEntry> cards;
};
} // namespace mu::ui::window

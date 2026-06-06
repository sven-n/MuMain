#pragma once

// Mouse picking: each frame this resolves what the cursor is over (monster,
// player, NPC, item, operable object) and updates the Selected* globals that
// the rest of the client reads. Extracted from ZzzInterface.cpp.
namespace Input::Selection
{
    void SelectObjects();
}

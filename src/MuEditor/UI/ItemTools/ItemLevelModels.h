#pragma once

#ifdef _EDITOR

#include <optional>
#include <vector>

namespace MuEditor::Items
{
// A run of + levels at which the inventory draws an item with the same model
// (Render::Items::Display::FindInventoryModel).
struct LevelModelRun
{
    int firstLevel = 0;
    int lastLevel = 0;
    // The model slot drawn; nothing when the item is not drawn at these levels.
    std::optional<int> modelType;
    // Drawn small (the archangel weapons of the Weapon of Archangel).
    bool small = false;
};

// The models the inventory draws for the item from +0 to +15, as runs.
std::vector<LevelModelRun> FindLevelModelRuns(int itemType);

// Whether the game draws the item at some + level with another model than
// its own (a level variant drawn by code, e.g. the Wizard's Ring, which has no
// model entry) or with none.
bool IsDrawnByCodeAtSomeLevel(int itemType);
} // namespace MuEditor::Items

#endif // _EDITOR

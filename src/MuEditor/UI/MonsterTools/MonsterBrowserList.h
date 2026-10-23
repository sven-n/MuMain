#pragma once

#ifdef _EDITOR

#include <vector>

namespace MuEditor::Monsters
{
// The rows of the Monster Browser: number, name in the editor language and
// kind of each listed monster, the selected one highlighted.
class MonsterBrowserList
{
public:
    // Draws the rows; returns the number that was clicked (or moved onto with
    // the arrow keys), or -1. `reveal` scrolls to the selected row once (it
    // was selected elsewhere).
    int Render(const std::vector<int>& numbers, int selectedNumber, bool reveal);
};
} // namespace MuEditor::Monsters

#endif // _EDITOR

#pragma once

#ifdef _EDITOR

#include "Data/GameData/MonsterData/MonsterKind.h"

#include <vector>

namespace MuEditor::Monsters
{
// The rows of the Monster Browser: number, name in the editor language and
// kind of each listed monster, the selected one highlighted. Each row's kind
// is a combo of its own.
class MonsterBrowserList
{
public:
    struct Result
    {
        // The number that was clicked (or moved onto with the arrow keys), or -1.
        int clicked = -1;
        // The number whose kind combo picked another kind, or -1, and that kind.
        int kindChanged = -1;
        Data::Monsters::MonsterKind kind = Data::Monsters::MonsterKind::Monster;
    };

    // Draws the rows that are in view. `reveal` scrolls to the selected row
    // once (it was selected elsewhere).
    Result Render(const std::vector<int>& numbers, int selectedNumber, bool reveal);
};
} // namespace MuEditor::Monsters

#endif // _EDITOR

#pragma once

#ifdef _EDITOR

#include <vector>

namespace MuEditor::Items
{
// The rows of the item browser: number, name as the game shows it and model
// file of each listed item, the selected one highlighted.
class CItemBrowserList
{
public:
    // Draws the rows; returns the item type that was clicked, or -1. `reveal`
    // scrolls to the selected row once (it was selected elsewhere).
    int Render(const std::vector<int>& itemTypes, int selectedItemType, bool reveal);
};
} // namespace MuEditor::Items

#endif // _EDITOR

#pragma once

#ifdef _EDITOR

#include "imgui.h"

#include <vector>

namespace MuEditor::Items
{
// The rows of Item Rules: number, name and a checkbox for each rule flag of
// the listed items. A checkbox changes its item at once (one undo step).
// Rows are selected with a click, Ctrl and Shift click for the buttons that
// change many items.
class CItemRulesTable
{
public:
    // Draws the rows; returns the item type whose row was clicked, or -1.
    // The row of `markedItemType` (the item the tools show) is marked;
    // `reveal` scrolls to it once (it was selected elsewhere).
    int Render(const std::vector<int>& itemTypes, int markedItemType, bool reveal);

    int GetSelectedCount() const
    {
        return m_selection.Size;
    }

    bool IsSelected(int itemType) const;

    // Drops the selected rows that are no longer listed (a filter or the
    // items changed), so the count and the buttons only take rows that are
    // shown.
    void KeepListedRows(const std::vector<int>& itemTypes);

private:
    int RenderRows(const std::vector<int>& itemTypes, int markedItemType, bool reveal);
    bool RenderRow(int row, int itemType, bool marked);

    // Keeps item types, not row indexes, so a selected row stays selected
    // when rows above it come or go.
    ImGuiSelectionBasicStorage m_selection;
    // The rows as last drawn: ImGui keeps the row a Shift click starts from as
    // an index into them.
    std::vector<int> m_rows;
};
} // namespace MuEditor::Items

#endif // _EDITOR

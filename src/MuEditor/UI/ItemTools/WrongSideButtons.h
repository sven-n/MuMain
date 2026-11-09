#pragma once

#ifdef _EDITOR

#include "KnownDifferences.h"

#include <optional>

// The "Wrong in client" and "Wrong in server" buttons of Item Sync's lists: two radio buttons in
// two table columns, at most one of them selected; clicking the selected one again clears it.
namespace MuEditor::Items::WrongSideButtons
{
// The columns SetupColumns adds.
constexpr int ColumnCount = 2;
// The two columns of a table.
void SetupColumns();
// The two buttons in the table's next two columns, showing `side`; the side a click picks
// (WrongSide::None when it clears the selected one), or nothing. `ofItem`: `side` is the mark of
// the field's item as a whole, shown faint; it is cleared on the item's row, so a click on it
// picks nothing.
std::optional<WrongSide> Render(WrongSide side, bool ofItem = false);
} // namespace MuEditor::Items::WrongSideButtons

#endif // _EDITOR

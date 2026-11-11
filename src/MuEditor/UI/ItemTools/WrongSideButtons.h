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
// Whose mark the buttons show.
enum class Shown : uint8_t
{
    // The row's own.
    Own,
    // A field's item's, shown faint: it is cleared on the item's row, so a click on it picks
    // nothing.
    OfItem,
    // A changed item's own, shown faint while some of its fields have their own of the other
    // side: a click on it marks every field with it.
    OfItemWithMarkedFields,
};
// The two buttons in the table's next two columns, showing `side`; the side a click picks
// (WrongSide::None when it clears the selected one), or nothing.
std::optional<WrongSide> Render(WrongSide side, Shown shown = Shown::Own);
} // namespace MuEditor::Items::WrongSideButtons

#endif // _EDITOR

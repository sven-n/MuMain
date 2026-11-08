#pragma once

#ifdef _EDITOR

#include "KnownDifferences.h"

#include <optional>

// The "Wrong in client" and "Wrong in server" buttons of Item Sync's lists: two radio buttons in
// two table columns, at most one of them selected; clicking the selected one again clears it.
namespace MuEditor::Items::WrongSideButtons
{
// The two columns of a table.
void SetupColumns();
// The two buttons in the table's next two columns, showing `side`; the side a click picks
// (WrongSide::None when it clears the selected one), or nothing.
std::optional<WrongSide> Render(WrongSide side);
} // namespace MuEditor::Items::WrongSideButtons

#endif // _EDITOR

#pragma once

#ifdef _EDITOR

#include "ItemCodeReferences.h"

namespace MuEditor::Items
{
// The lines of the sources the editor was built from that name an item. Read
// on the first call, so the game does not pay for it unless an item tool asks;
// not loaded when the sources are gone.
const ItemCodeReferences& GetItemCodeReferences();
} // namespace MuEditor::Items

#endif // _EDITOR

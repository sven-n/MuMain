#pragma once

#ifdef _EDITOR

#include "ItemNumberHolderView.h"
#include "ItemUsage.h"

namespace MuEditor::Items
{
// The dialog that moves an item, with its model, to a free number. What uses
// the item by its number (code, tags of other items, game data) stays there,
// so it is listed first.
class CItemMoveDialog
{
public:
    void Open(int itemType);
    void Render();

private:
    bool m_open = false;
    int m_itemType = -1;
    int m_group = 0;
    int m_number = 0;
    ItemUsage m_usage;
    CItemNumberHolderView m_numberHolder;
};
} // namespace MuEditor::Items

#endif // _EDITOR

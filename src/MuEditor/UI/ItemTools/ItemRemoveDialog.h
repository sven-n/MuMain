#pragma once

#ifdef _EDITOR

#include "ItemUsage.h"

namespace MuEditor::Items
{
// The dialog that removes an item and its model entry, after it listed what
// uses the item (section 7). The removal can be undone.
class CItemRemoveDialog
{
public:
    void Open(int itemType);
    void Render();

private:
    bool m_open = false;
    int m_itemType = -1;
    ItemUsage m_usage;
};
} // namespace MuEditor::Items

#endif // _EDITOR

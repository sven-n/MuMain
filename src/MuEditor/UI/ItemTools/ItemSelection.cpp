#include "stdafx.h"

#ifdef _EDITOR

#include "ItemSelection.h"

namespace MuEditor::Items
{
ItemSelection& ItemSelection::GetInstance()
{
    static ItemSelection instance;
    return instance;
}

void ItemSelection::Select(int itemType)
{
    m_itemType = itemType;
    ++m_serial;
}
} // namespace MuEditor::Items

#endif // _EDITOR

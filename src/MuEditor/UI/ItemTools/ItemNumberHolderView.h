#pragma once

#ifdef _EDITOR

#include "ItemUsage.h"

namespace MuEditor::Items
{
// What takes a number that is not free (UsageView::RenderNumberHolder), for a
// dialog drawn every frame: it is found again only when the number or the item
// data changed. A dialog that opens again starts with a new one, so it also
// sees game data that came in meanwhile.
class CItemNumberHolderView
{
public:
    void Render(int itemType);

private:
    int m_itemType = -1;
    long long m_dataVersion = -1;
    ItemNumberHolder m_holder;
    ItemUsage m_usage;
};
} // namespace MuEditor::Items

#endif // _EDITOR

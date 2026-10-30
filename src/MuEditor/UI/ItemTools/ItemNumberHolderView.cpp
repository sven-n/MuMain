#include "stdafx.h"

#ifdef _EDITOR

#include "ItemNumberHolderView.h"

#include "ItemToolWindow.h"
#include "ItemUsageView.h"

namespace MuEditor::Items
{
void CItemNumberHolderView::Render(int itemType)
{
    const long long dataVersion = ToolWindow::GetDataVersion();
    if (itemType != m_itemType || dataVersion != m_dataVersion)
    {
        m_holder = FindItemNumberHolder(itemType);
        // An item on the number is named alone.
        m_usage = m_holder.item.empty() ? FindItemUsage(itemType) : ItemUsage{};
        m_itemType = itemType;
        m_dataVersion = dataVersion;
    }
    UsageView::RenderNumberHolder(m_holder, m_usage);
}
} // namespace MuEditor::Items

#endif // _EDITOR

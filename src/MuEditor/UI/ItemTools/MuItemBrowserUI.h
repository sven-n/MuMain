#pragma once

#ifdef _EDITOR

#include "ItemAddDialog.h"
#include "ItemBrowserList.h"
#include "ItemMoveDialog.h"
#include "ItemRemoveDialog.h"

#include "../MuEditor/UI/ItemEditor/ItemListModel.h"

#include <string>

// The item browser (section 9): the items with a search and filters (group,
// tag, rule flag, missing model or name), where items are added, duplicated,
// removed and moved, and saved, imported and exported. A click selects the
// item for all item tools.
class CMuItemBrowserUI
{
public:
    static CMuItemBrowserUI& GetInstance();

    void Render(bool* open);

private:
    CMuItemBrowserUI() = default;

    void RenderFileActions();
    void RenderFilters();
    void RenderTagFilter();
    void RenderRuleFilter();
    void RenderMissingFilter();
    void RenderItemActions(int selectedItemType);
    // When the selection changed elsewhere: clears the filters that hide the
    // item; returns whether the list scrolls to it.
    bool RevealSelection(int selectedItemType);

    CItemListModel m_list{CItemListModel::NumbersWithoutItem::InAllItems};
    CItemListModel::Filter m_filter;
    std::string m_search;
    MuEditor::Items::CItemBrowserList m_rows;
    MuEditor::Items::CItemAddDialog m_addDialog;
    MuEditor::Items::CItemRemoveDialog m_removeDialog;
    MuEditor::Items::CItemMoveDialog m_moveDialog;
    int m_shownSerial = -1;
};

#define g_MuItemBrowserUI CMuItemBrowserUI::GetInstance()

#endif // _EDITOR

#pragma once

#ifdef _EDITOR

#include "ItemRulesTable.h"

#include "../MuEditor/UI/ItemEditor/ItemListModel.h"

#include <cstddef>
#include <string>
#include <vector>

// The item rules tool (section 9): the rule flags of the items as a matrix,
// items by tradable, droppable, storable, sellable and the others. A checkbox
// changes one item; the selected rows are changed together for one flag. A
// click selects the item for all item tools.
class CMuItemRulesUI
{
public:
    static CMuItemRulesUI& GetInstance();

    void Render(bool* open);

private:
    CMuItemRulesUI() = default;

    void RenderFilters();
    void RenderBulkActions(const std::vector<int>& itemTypes);
    void RenderBulkRuleCombo();
    // Sets the bulk rule of the selected rows that differ, as one undo step.
    void SetBulkRule(const std::vector<int>& itemTypes, bool allowed);
    // When the selection changed elsewhere: clears the filters that hide the
    // item; returns whether the table scrolls to it.
    bool RevealSelection(int selectedItemType);
    // The listed items; the table's selection drops the rows that left them.
    const std::vector<int>& UpdateRows();

    CItemListModel m_list;
    CItemListModel::Filter m_filter;
    std::string m_search;
    MuEditor::Items::CItemRulesTable m_table;
    // The flag the bulk buttons set, an index into GetItemRuleFlags().
    size_t m_bulkRule = 0;
    // The list the table's selection was last matched to.
    CItemListModel::Filter m_listedFilter;
    long long m_listedVersion = -1;
    int m_shownSerial = -1;
};

#define g_MuItemRulesUI CMuItemRulesUI::GetInstance()

#endif // _EDITOR

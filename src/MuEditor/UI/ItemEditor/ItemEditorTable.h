#pragma once

#ifdef _EDITOR

#include <string>
#include <vector>
#include <map>

// Forward declaration
class CItemEditorColumns;

// Handles the main table rendering for the Item Editor
// Delegates column rendering to CItemEditorColumns
class CItemEditorTable
{
public:
    CItemEditorTable();
    ~CItemEditorTable();

    // Main render function
    void Render(const std::string& searchFilter,
                std::map<std::string, bool>& columnVisibility,
                int& selectedRow,
                bool freezeColumns);

    // Selects the item and scrolls to it at the next Render; the selection is
    // set even when the search filter hides the item.
    static void RequestScrollToIndex(int index);

    // Whether the table lists the item with this (lowercase) search filter:
    // items with a name that contains it.
    static bool IsListed(int itemIndex, const std::string& searchFilter);

    // Force rebuild of filtered list (call after data changes)
    void InvalidateFilter();

private:
    // Filter state
    std::vector<int> m_filteredItems;
    std::string m_lastSearchFilter;
    bool m_isInitialized;

    // Column renderer
    CItemEditorColumns* m_pColumns;

    // Scroll request state
    static int s_scrollToIndex;
};

#endif // _EDITOR

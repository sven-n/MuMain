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

private:
    // Filter state
    std::vector<int> m_filteredItems;
    std::string m_lastSearchFilter;
    bool m_isInitialized;

    // Column renderer
    CItemEditorColumns* m_pColumns;
};

#endif // _EDITOR

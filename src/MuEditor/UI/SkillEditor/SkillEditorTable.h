#pragma once

#ifdef _EDITOR

#include <string>
#include <vector>
#include <map>

// Forward declaration
class CSkillEditorColumns;

// Handles the main table rendering for the Skill Editor
// Delegates column rendering to CSkillEditorColumns
class CSkillEditorTable
{
public:
    CSkillEditorTable();
    ~CSkillEditorTable();

    // Main render function
    void Render(const std::string& searchFilter,
                std::map<std::string, bool>& columnVisibility,
                int& selectedRow,
                bool freezeColumns);

    // Request scroll to a specific skill index
    static void RequestScrollToIndex(int index);

    // Force rebuild of filtered list (call after data changes)
    void InvalidateFilter();

private:
    // Filter state
    std::vector<int> m_filteredSkills;
    std::string m_lastSearchFilter;
    bool m_isInitialized;

    // Column renderer
    CSkillEditorColumns* m_pColumns;

    // Scroll request state
    static int s_scrollToIndex;
};

#endif // _EDITOR

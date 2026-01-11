#pragma once

#ifdef _EDITOR

#include <string>
#include <map>

// Forward declarations
class CSkillEditorTable;

class CMuSkillEditorUI
{
public:
    static CMuSkillEditorUI& GetInstance();

    void Render(bool& showEditor);
    void ClearSearch() { m_szSkillSearchBuffer[0] = '\0'; }
    void SaveColumnPreferences();
    void LoadColumnPreferences();

private:
    CMuSkillEditorUI();
    ~CMuSkillEditorUI();

    void RenderSearchBar();
    void RenderColumnVisibilityMenu();

    char m_szSkillSearchBuffer[256];

    // Column visibility state (cached from config)
    std::map<std::string, bool> m_columnVisibility;

    // Selected row tracking
    int m_selectedRow;

    // Column freezing state
    bool m_bFreezeColumns;

    // Table renderer
    CSkillEditorTable* m_pTable;
};

#define g_MuSkillEditorUI CMuSkillEditorUI::GetInstance()

#endif // _EDITOR

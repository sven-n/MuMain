#pragma once

#ifdef _EDITOR

#include <string>
#include <map>

// Forward declarations
class CItemEditorTable;

class CMuItemEditorUI
{
public:
    static CMuItemEditorUI& GetInstance();

    void Render(bool& showEditor);
    void ClearSearch() { m_szItemSearchBuffer[0] = '\0'; }
    void SaveColumnPreferences();
    void LoadColumnPreferences();

private:
    CMuItemEditorUI();
    ~CMuItemEditorUI();

    void RenderSearchBar();
    void RenderColumnVisibilityMenu();

    char m_szItemSearchBuffer[256];

    // Column visibility state (cached from config)
    std::map<std::string, bool> m_columnVisibility;

    // Selected row tracking
    int m_selectedRow;

    // Column freezing state
    bool m_bFreezeColumns;

    // Table renderer
    CItemEditorTable* m_pTable;
};

#define g_MuItemEditorUI CMuItemEditorUI::GetInstance()

#endif // _EDITOR

#pragma once

#ifdef _EDITOR

#include <string>
#include <map>

class CMuItemEditor
{
public:
    static CMuItemEditor& GetInstance();

    void Render(bool& showEditor);
    void ClearSearch() { m_szItemSearchBuffer[0] = '\0'; }
    void SaveColumnPreferences();
    void LoadColumnPreferences();

private:
    CMuItemEditor();
    ~CMuItemEditor();

    void RenderSearchBar();
    void RenderItemTable(const std::string& searchLower);
    void RenderSaveButton();
    void RenderColumnVisibilityMenu();

    char m_szItemSearchBuffer[256];

    // Column visibility state
    std::map<std::string, bool> m_columnVisibility;

    // Config file path
    const char* muitemeditor_columns_cfg = "MuEditor\\MuItemEditorColumns.cfg";

    // Selected row tracking
    int m_selectedRow;

    // Column freezing state
    bool m_bFreezeColumns;
};

#define g_MuItemEditor CMuItemEditor::GetInstance()

#endif // _EDITOR

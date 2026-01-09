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

private:
    CMuItemEditor();
    ~CMuItemEditor() = default;

    void RenderSearchBar();
    void RenderItemTable(const std::string& searchLower);
    void RenderSaveButton();
    void RenderColumnVisibilityMenu();

    char m_szItemSearchBuffer[256];

    // Column visibility state
    std::map<std::string, bool> m_columnVisibility;

    // Selected row tracking
    int m_selectedRow;
};

#define g_MuItemEditor CMuItemEditor::GetInstance()

#endif // _EDITOR

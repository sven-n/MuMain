#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemTranslationsUI.h"

#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorActions.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorPopups.h"
#include "../MuEditor/UI/ItemTools/ItemToolWindow.h"
#include "I18N/All.h"
#include "imgui.h"

CMuItemTranslationsUI& CMuItemTranslationsUI::GetInstance()
{
    static CMuItemTranslationsUI instance;
    return instance;
}

void CMuItemTranslationsUI::Render(bool* open)
{
    m_grid.BeginFrame();

    // One of the item tools: their docking, undo keys and selection.
    if (MuEditor::Items::ToolWindow::Begin(I18N::Editor::ItemTranslations, MuEditor::WindowIds::ItemTranslations, open))
    {
        m_grid.UpdateCountsAndLabels(m_rows);
        CItemEditorActions::RenderSaveButton();
        CItemEditorPopups::RenderAll();
        ImGui::SameLine();
        m_grid.RenderToolbar(m_rows);
        ImGui::Separator();
        m_grid.RenderTable(m_rows, "##ItemTranslations");
    }
    MuEditor::Items::ToolWindow::End();
    m_grid.CommitEditNotDrawn(m_rows);
}

#endif // _EDITOR

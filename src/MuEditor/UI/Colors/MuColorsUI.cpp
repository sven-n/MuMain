#include "stdafx.h"

#ifdef _EDITOR

#include "MuColorsUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorToolWindows.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/ItemTools/ItemToolWindow.h"
#include "I18N/All.h"
#include "imgui.h"

using MuEditor::Colors::ColorsTab;

namespace
{
// The tabs' ids, which stay when their titles change with the language.
constexpr char GlowTabId[] = "###Glow";
} // namespace

CMuColorsUI& CMuColorsUI::GetInstance()
{
    static CMuColorsUI instance;
    return instance;
}

void CMuColorsUI::Render(bool* open)
{
    if (MuEditor::Items::ToolWindow::Begin(I18N::Editor::Colors, MuEditor::WindowIds::Colors, open) &&
        ImGui::BeginTabBar("##Tabs"))
    {
        if (BeginTab(ColorsTab::Glow, I18N::Editor::Glow, GlowTabId))
        {
            m_glow.Render();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
        m_select.reset();
    }
    MuEditor::Items::ToolWindow::End();
}

void CMuColorsUI::Show(ColorsTab tab)
{
    g_MuEditorCore.GetToolWindows().colors = true;
    m_select = tab;
    m_shown = tab;
    ImGui::SetWindowFocus(MuEditor::WindowIds::Colors);
}

bool CMuColorsUI::BeginTab(ColorsTab tab, const char* label, const char* id)
{
    char title[MuEditor::WindowIds::TitleCapacity];
    const ImGuiTabItemFlags flags = m_select == tab ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
    if (!ImGui::BeginTabItem(MuEditor::WindowIds::Title(title, label, id), nullptr, flags))
    {
        return false;
    }
    m_shown = tab;
    return true;
}

#endif // _EDITOR

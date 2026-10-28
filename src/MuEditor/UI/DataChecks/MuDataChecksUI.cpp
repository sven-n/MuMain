#include "stdafx.h"

#ifdef _EDITOR

#include "MuDataChecksUI.h"

#include "FileCheckArea.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Common/SavedDataFiles.h"
#include "../MuEditor/UI/Common/ToolWindowFrame.h"
#include "../MuEditor/UI/Common/UndoTarget.h"
#include "../MuEditor/UI/ItemTools/ItemCheckArea.h"
#include "../MuEditor/UI/ItemTools/ItemToolWindow.h"
#include "../MuEditor/UI/MonsterTools/MonsterCheckArea.h"
#include "../MuEditor/UI/MonsterTools/MonsterToolWindow.h"
#include "I18N/All.h"
#include "Render/Look/LookTuning.h"
#include "imgui.h"

#include <utility>

namespace MuEditor::DataChecks
{
namespace
{
using Data::Items::ItemDataIssue;

std::vector<std::unique_ptr<DataCheckArea>> CreateAllAreas()
{
    std::vector<std::unique_ptr<DataCheckArea>> areas;
    areas.push_back(Items::CreateItemCheckArea());
    areas.push_back(Monsters::CreateMonsterCheckArea());
    areas.push_back(Monsters::CreateMonsterModelCheckArea());
    areas.push_back(CreateModelFileCheckArea());
    areas.push_back(CreateServerListCheckArea());
    areas.push_back(CreateEffectCheckArea());
    areas.push_back(CreateLookCheckArea());
    return areas;
}

void RenderNotes()
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("%s", I18N::Editor::DataChecksWhatNote);
    ImGui::TextWrapped("%s", I18N::Editor::DataChecksSaveNote);
    ImGui::TextWrapped("%s", I18N::Editor::DataChecksClickNote);
    ImGui::PopStyleColor();
}
} // namespace

MuDataChecksUI& MuDataChecksUI::GetInstance()
{
    static MuDataChecksUI instance(CreateAllAreas());
    return instance;
}

MuDataChecksUI::MuDataChecksUI(std::vector<std::unique_ptr<DataCheckArea>> areas) : m_areas(std::move(areas))
{
    for (const std::unique_ptr<DataCheckArea>& area : m_areas)
    {
        m_areaPointers.push_back(area.get());
    }
}

void MuDataChecksUI::Render(bool* open)
{
    if (BeginWindow(open))
    {
        if (!m_checked)
        {
            RunChecks();
        }
        RenderCheckLine();
        RenderAreaBoxes();
        RenderNotes();
        ImGui::Separator();
        m_issueTable.RenderTable(m_areaPointers);
    }
    ImGui::End();
}

// A tool window with the editor's bar, docked beside the item tools until it
// has a place of its own. It changes no data: Ctrl+Z and Ctrl+Y act on the
// tool worked in last, as the bar's Undo and Redo do.
bool MuDataChecksUI::BeginWindow(bool* open)
{
    char title[WindowIds::TitleCapacity];
    Layout::DockBesideItemTools(WindowIds::DataChecks);
    Layout::PrepareToolWindow();
    if (!ImGui::Begin(WindowIds::Title(title, I18N::Editor::DataChecks, WindowIds::DataChecks), open,
                      ImGuiWindowFlags_NoCollapse | Layout::ToolWindowBarFlags(WindowIds::DataChecks)))
    {
        return false;
    }
    Layout::RenderToolWindowBar();
    ToolWindowFrame::NoteHovered();
    ToolWindowFrame::HandleUndoKeys(UndoTargets::GetCurrent(Items::ToolWindow::GetUndoTarget()));
    return true;
}

void MuDataChecksUI::RunChecks()
{
    std::vector<AreaIssue> found;
    std::vector<ItemDataIssue> issues;
    for (size_t area = 0; area < m_areas.size(); ++area)
    {
        issues.clear();
        m_areas[area]->Check(issues);
        for (ItemDataIssue& issue : issues)
        {
            found.push_back({std::move(issue), static_cast<int>(area)});
        }
    }
    m_issueTable.SetIssues(std::move(found));
    m_checked = true;
    m_checkedItemVersion = Items::ToolWindow::GetDataVersion();
    m_checkedMonsterVersion = Monsters::ToolWindow::GetDataVersion();
    m_checkedFileSaves = SavedDataFiles::GetSaveCount();
    m_checkedLookSaves = Render::Look::SaveCount();
}

void MuDataChecksUI::RenderCheckLine()
{
    if (ImGui::Button(I18N::Editor::CheckAllData))
    {
        RunChecks();
    }
    ImGui::SameLine();
    m_issueTable.RenderKindBoxes();
    // Checking again on every change would cost tens of milliseconds each
    // time: Check all does.
    if (DataChangedSinceCheck())
    {
        IssueTable::RenderChangedNote(I18N::Editor::TheDataChangedSinceTheLastCheckCheckAgain);
    }
}

void MuDataChecksUI::RenderAreaBoxes()
{
    ImGui::TextDisabled("%s", I18N::Editor::Areas);
    for (size_t area = 0; area < m_areas.size(); ++area)
    {
        ImGui::SameLine();
        bool shown = m_issueTable.IsAreaShown(static_cast<int>(area));
        ImGui::PushID(static_cast<int>(area));
        if (ImGui::Checkbox(m_areas[area]->GetName(), &shown))
        {
            m_issueTable.SetAreaShown(static_cast<int>(area), shown);
        }
        ImGui::PopID();
    }
}

// The items and the monsters change in their tools; of the other areas' files
// the editor writes the server list and the look files. Changes made outside
// the editor are not seen.
bool MuDataChecksUI::DataChangedSinceCheck() const
{
    return Items::ToolWindow::GetDataVersion() != m_checkedItemVersion ||
           Monsters::ToolWindow::GetDataVersion() != m_checkedMonsterVersion ||
           SavedDataFiles::GetSaveCount() != m_checkedFileSaves || Render::Look::SaveCount() != m_checkedLookSaves;
}
} // namespace MuEditor::DataChecks

#endif // _EDITOR

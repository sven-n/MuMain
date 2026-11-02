#include "stdafx.h"

#ifdef _EDITOR

#include "MuSoundEditorUI.h"

#include "SoundEditorEdits.h"
#include "SoundFilePicker.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "Data/GameData/SoundData/SoundFileDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cfloat>

MuSoundEditorUI g_MuSoundEditorUI;

namespace
{
using MuEditor::SoundEditor::RowProblem;
using MuEditor::SoundEditor::RowSource;
using MuEditor::SoundEditor::SoundRow;

constexpr int Columns = 7;
constexpr float FilterWidth = 220.0f;
constexpr float NameWidth = 200.0f;
constexpr float ChannelsWidth = 46.0f;
constexpr float PickerWidth = 420.0f;
constexpr float PickerHeight = 300.0f;
const ImVec4 ProblemColor(0.94f, 0.58f, 0.58f, 1.0f);

const char* ProblemText(RowProblem problem)
{
    switch (problem)
    {
    case RowProblem::MissingFile:
        return I18N::Editor::SoundEditorMissingFile;
    case RowProblem::SharedFile:
        return I18N::Editor::SoundEditorSharedFile;
    case RowProblem::PlayedNotLoaded:
        return I18N::Editor::SoundEditorPlayedNotLoaded;
    case RowProblem::None:
        break;
    }
    return "";
}

void Tooltip(const char* text)
{
    if (text != nullptr && text[0] != '\0' && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", text);
    }
}

bool Editable(const SoundRow& row)
{
    return row.source != RowSource::NotLoaded;
}
} // namespace

void MuSoundEditorUI::Render(bool* open)
{
    using namespace MuEditor;
    char title[WindowIds::TitleCapacity];
    Layout::PrepareToolWindow();
    if (ImGui::Begin(WindowIds::Title(title, I18N::Editor::SoundEditorTitle, WindowIds::SoundEditor), open,
                     ImGuiWindowFlags_NoCollapse))
    {
        // What Save and Revert compare with: the files as the game read them.
        static bool taken = false;
        if (!taken)
        {
            g_SoundEdits.TakeSaved();
            taken = true;
        }
        RenderToolbar();
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled("%s", I18N::Editor::SoundEditorNote);
        ImGui::PopTextWrapPos();
        const std::vector<SoundRow> rows = SoundEditor::BuildRows();
        std::vector<const SoundRow*> shown;
        for (const SoundRow& row : rows)
        {
            if (SoundEditor::Matches(row, m_filterText.data()) && (!m_onlyProblems || row.problem != RowProblem::None))
            {
                shown.push_back(&row);
            }
        }
        RenderFilterBar(rows, shown.size());
        RenderTable(shown);
    }
    ImGui::End();
}

void MuSoundEditorUI::RenderToolbar()
{
    ImGui::BeginDisabled(!g_SoundEdits.IsModified());
    if (ImGui::Button(I18N::Editor::Save))
    {
        std::vector<Data::Items::ItemDataIssue> issues;
        SetStatus(g_SoundEdits.Save(issues), I18N::Editor::SoundEditorSaved, issues);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Revert))
    {
        std::vector<Data::Items::ItemDataIssue> issues;
        SetStatus(g_SoundEdits.Revert(issues), I18N::Editor::SoundEditorReverted, issues);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!g_SoundEdits.CanUndo());
    if (ImGui::Button(I18N::Editor::Undo))
    {
        g_SoundEdits.Undo();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!g_SoundEdits.CanRedo());
    if (ImGui::Button(I18N::Editor::Redo))
    {
        g_SoundEdits.Redo();
    }
    ImGui::EndDisabled();
    if (g_SoundEdits.IsModified())
    {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", I18N::Editor::NotSavedYet);
    }
    if (!m_status.empty())
    {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled("%s", m_status.c_str());
        ImGui::PopTextWrapPos();
    }
}

void MuSoundEditorUI::SetStatus(bool done, const char* doneText, const std::vector<Data::Items::ItemDataIssue>& issues)
{
    m_status = done ? doneText : "";
    for (const Data::Items::ItemDataIssue& issue : issues)
    {
        if (issue.severity == Data::Items::ItemDataIssueSeverity::Fatal)
        {
            m_status += (m_status.empty() ? "" : "\n") + issue.ToString();
        }
    }
}

void MuSoundEditorUI::RenderFilterBar(const std::vector<SoundRow>& rows, size_t shown)
{
    ImGui::SetNextItemWidth(FilterWidth * g_MuEditorCore.GetUIScale());
    ImGui::InputTextWithHint("##Filter", I18N::Editor::SoundEditorFilterHint, m_filterText.data(), m_filterText.size());
    ImGui::SameLine();
    const auto problems =
        std::count_if(rows.begin(), rows.end(), [](const SoundRow& row) { return row.problem != RowProblem::None; });
    ImGui::Checkbox(I18N::Format(I18N::Editor::SoundEditorOnlyProblems0, {std::to_string(problems)}).c_str(),
                    &m_onlyProblems);
    ImGui::SameLine();
    ImGui::TextDisabled(
        "%s",
        I18N::Format(I18N::Editor::SoundEditorShown01, {std::to_string(shown), std::to_string(rows.size())}).c_str());
}

void MuSoundEditorUI::RenderTable(const std::vector<const SoundRow*>& rows)
{
    constexpr ImGuiTableFlags Flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY |
                                      ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Sounds", Columns, Flags, ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
    {
        return;
    }
    const float scale = g_MuEditorCore.GetUIScale();
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnName, ImGuiTableColumnFlags_WidthFixed, NameWidth * scale);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnId);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnFile, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnChannels);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumn3D);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnLoads);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnLivesIn);
    ImGui::TableHeadersRow();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(rows.size()));
    while (clipper.Step())
    {
        for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
        {
            RenderRow(*rows[static_cast<size_t>(index)]);
        }
    }
    ImGui::EndTable();
}

void MuSoundEditorUI::RenderRow(const SoundRow& row)
{
    using MuEditor::SoundEditor::RowValues;
    ImGui::PushID(row.key.c_str());
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    RenderNameCell(row);
    ImGui::TableNextColumn();
    ImGui::TextDisabled("%s", MuEditor::SoundEditor::IdText(row).c_str());
    ImGui::TableNextColumn();
    RenderFileCell(row);
    ImGui::TableNextColumn();
    RowValues values = MuEditor::SoundEditor::ValuesOf(row);
    ImGui::BeginDisabled(!Editable(row));
    ImGui::SetNextItemWidth(ChannelsWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::InputInt("##Channels", &values.channels, 0))
    {
        values.channels = std::clamp(values.channels, 1, Data::Sounds::MaxSoundChannels);
        g_SoundEdits.Change(row, values);
    }
    Tooltip(I18N::Editor::SoundEditorChannelsTip);
    ImGui::EndDisabled();
    ImGui::TableNextColumn();
    ImGui::BeginDisabled(!Editable(row) || row.source == RowSource::MapObject);
    bool spatial = row.spatial;
    if (ImGui::Checkbox("##3D", &spatial))
    {
        values = MuEditor::SoundEditor::ValuesOf(row);
        values.spatial = spatial;
        g_SoundEdits.Change(row, values);
    }
    Tooltip(I18N::Editor::SoundEditor3DTip);
    ImGui::EndDisabled();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(row.loads.c_str());
    ImGui::TableNextColumn();
    ImGui::TextDisabled("%s", row.livesIn.c_str());
    ImGui::PopID();
}

void MuSoundEditorUI::RenderNameCell(const SoundRow& row)
{
    if (!Editable(row))
    {
        ImGui::TextColored(ProblemColor, "%s", MuEditor::SoundEditor::ShownName(row).c_str());
        Tooltip(ProblemText(row.problem));
        return;
    }
    std::string text = m_nameEditKey == row.key ? m_nameEdit : row.name;
    // Without a name: what the row shows instead.
    SoundRow unnamed = row;
    unnamed.name.clear();
    const std::string hint = MuEditor::SoundEditor::ShownName(unnamed);
    ImGui::SetNextItemWidth(-FLT_MIN);
    MuEditor::Items::InputStringWithHint("##Name", hint.c_str(), text);
    if (ImGui::IsItemActive())
    {
        m_nameEditKey = row.key;
        m_nameEdit = text;
    }
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(row);
        values.name = text;
        g_SoundEdits.Change(row, values);
        m_nameEditKey.clear();
    }
    else if (!ImGui::IsItemActive() && m_nameEditKey == row.key)
    {
        m_nameEditKey.clear();
    }
}

void MuSoundEditorUI::RenderFileCell(const SoundRow& row)
{
    const bool problem = row.problem != RowProblem::None;
    if (problem)
    {
        ImGui::TextColored(ProblemColor, "%s", row.file.c_str());
        Tooltip(ProblemText(row.problem));
    }
    else
    {
        ImGui::TextUnformatted(row.file.c_str());
    }
    if (!Editable(row))
    {
        return;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("..."))
    {
        m_pickerKey = row.key;
        m_pickerText.fill('\0');
        ImGui::OpenPopup("##Picker");
    }
    Tooltip(I18N::Editor::SoundEditorPickTip);
    if (m_pickerKey == row.key)
    {
        RenderPicker(row);
    }
}

void MuSoundEditorUI::RenderPicker(const SoundRow& row)
{
    if (!ImGui::BeginPopup("##Picker"))
    {
        return;
    }
    const float scale = g_MuEditorCore.GetUIScale();
    ImGui::TextUnformatted(
        I18N::Format(I18N::Editor::SoundEditorPickFor0, {MuEditor::SoundEditor::ShownName(row)}).c_str());
    if (ImGui::IsWindowAppearing())
    {
        ImGui::SetKeyboardFocusHere();
    }
    ImGui::SetNextItemWidth(PickerWidth * scale);
    ImGui::InputTextWithHint("##Search", I18N::Editor::SoundEditorFilterHint, m_pickerText.data(), m_pickerText.size());
    const std::vector<std::string>& files = MuEditor::SoundEditor::ClientSoundFiles();
    const std::vector<std::string> shown = MuEditor::SoundEditor::FilterFiles(files, m_pickerText.data());
    ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::SoundEditorFiles01,
                                           {std::to_string(shown.size()), std::to_string(files.size())})
                                  .c_str());
    if (ImGui::BeginChild("##Files", ImVec2(PickerWidth * scale, PickerHeight * scale), ImGuiChildFlags_Borders))
    {
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(shown.size()));
        while (clipper.Step())
        {
            for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
            {
                const std::string& file = shown[static_cast<size_t>(index)];
                ImGui::PushID(index);
                if (ImGui::SmallButton(I18N::Editor::PlayThisSound))
                {
                    MuEditor::SoundEditor::PreviewFile(file, row.channels, row.spatial);
                }
                ImGui::SameLine();
                if (ImGui::Selectable(file.c_str(), file == row.file))
                {
                    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(row);
                    values.file = file;
                    g_SoundEdits.Change(row, values);
                    m_pickerKey.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopID();
            }
        }
    }
    ImGui::EndChild();
    ImGui::EndPopup();
}

#endif // _EDITOR

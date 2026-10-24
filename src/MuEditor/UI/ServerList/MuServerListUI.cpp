#include "stdafx.h"

#ifdef _EDITOR

#include "MuServerListUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ServerListData/ServerListJson.h"
#include "I18N/All.h"
#include "Network/Server/ServerListManager.h"
#include "Scenes/SceneCore.h"
#include "UI/Windows/ServerSelWin.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <map>
#include <optional>
#include <string>

using Data::ServerList::ChannelType;
using Data::ServerList::ServerGroup;

namespace
{
// Pixel widths before the editor UI scale.
constexpr float GroupListWidth = 220.0f;
constexpr float IdFieldWidth = 120.0f;
constexpr float TextFieldWidth = 320.0f;
constexpr float TypeComboWidth = 140.0f;
constexpr int ChannelTableColumns = 4;
constexpr ImVec4 ErrorColor{1.0f, 0.4f, 0.4f, 1.0f};

const char* GetChannelTypeLabel(ChannelType type)
{
    switch (type)
    {
    case ChannelType::NonPvp:
        return I18N::Editor::NonPvP;
    case ChannelType::GoldPvp:
        return I18N::Editor::GoldPvP;
    case ChannelType::Gold:
        return I18N::Editor::Gold;
    case ChannelType::Normal:
    default:
        return I18N::Editor::Normal;
    }
}

// A grey line that wraps at the window's edge.
void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}

void RenderError(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ErrorColor);
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(text);
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
}

bool BeginChannelTable()
{
    constexpr ImGuiTableFlags TableFlags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Channels", ChannelTableColumns, TableFlags))
    {
        return false;
    }
    ImGui::TableSetupColumn(I18N::Editor::Channel);
    ImGui::TableSetupColumn(I18N::Editor::Type);
    ImGui::TableSetupColumn(I18N::Editor::OnTheLoginScreen);
    ImGui::TableSetupColumn(I18N::Editor::ConnectServer);
    ImGui::TableHeadersRow();
    return true;
}

// The type picked in the combo, when it is another one than `type`.
std::optional<ChannelType> PickChannelType(ChannelType type)
{
    ImGui::SetNextItemWidth(TypeComboWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::BeginCombo("##Type", GetChannelTypeLabel(type)))
    {
        return std::nullopt;
    }
    std::optional<ChannelType> picked;
    for (int value = 0; value < Data::ServerList::ChannelTypeCount; ++value)
    {
        const ChannelType option = static_cast<ChannelType>(value);
        if (ImGui::Selectable(GetChannelTypeLabel(option), option == type) && option != type)
        {
            picked = option;
        }
    }
    ImGui::EndCombo();
    return picked;
}

// One channel: its number, type, name on the login screen and the load the
// connect server sent for it (sentChannels: the group's, or nullptr). Returns
// the type picked for it; the caller sets it.
std::optional<ChannelType> RenderChannelRow(const ServerGroup& group, int channel,
                                            const std::map<int, int>* sentChannels)
{
    ImGui::PushID(channel);
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("%d", channel + 1);

    ImGui::TableNextColumn();
    const std::optional<ChannelType> picked = PickChannelType(group.channels[static_cast<size_t>(channel)]);
    const ChannelType type = picked.value_or(group.channels[static_cast<size_t>(channel)]);

    ImGui::TableNextColumn();
    ImGui::TextDisabled("%s-%d%s", group.name.c_str(), channel + 1, Data::ServerList::GetChannelSuffix(type));

    ImGui::TableNextColumn();
    if (sentChannels != nullptr)
    {
        const auto load = sentChannels->find(channel);
        if (load != sentChannels->end())
        {
            ImGui::TextUnformatted(I18N::Format(I18N::Editor::Sent0Load, {std::to_string(load->second)}).c_str());
        }
    }
    ImGui::PopID();
    return picked;
}

CMuServerListUI::SentChannels FindSentChannels()
{
    CMuServerListUI::SentChannels sent;
    for (const CServerListManager::ReceivedServer& server : g_ServerListManager->GetReceivedServers())
    {
        sent[server.connectIndex / Data::ServerList::MaxChannelsPerGroup]
            [server.connectIndex % Data::ServerList::MaxChannelsPerGroup] = server.percent;
    }
    return sent;
}

std::string ListServerIds(const std::map<int, int>& channels, int groupId)
{
    std::string text;
    for (const auto& [channel, load] : channels)
    {
        text += (text.empty() ? "" : ", ") + std::to_string(groupId * Data::ServerList::MaxChannelsPerGroup + channel);
    }
    return text;
}
} // namespace

CMuServerListUI& CMuServerListUI::GetInstance()
{
    static CMuServerListUI instance;
    return instance;
}

void CMuServerListUI::Render(bool* open)
{
    if (!m_loaded)
    {
        LoadFile();
    }
    if (!BeginWindow(open))
    {
        return;
    }
    if (m_sentVersion != g_ServerListManager->GetReceivedServersVersion())
    {
        m_sent = FindSentChannels();
        m_sentVersion = g_ServerListManager->GetReceivedServersVersion();
    }
    RenderToolbar();
    RenderStatus();
    ImGui::Separator();
    const bool changed = RenderPanes();
    ImGui::End();
    AfterEdits(changed);
}

bool CMuServerListUI::BeginWindow(bool* open)
{
    char title[MuEditor::WindowIds::TitleCapacity];
    MuEditor::Layout::PrepareToolWindow();
    if (!ImGui::Begin(
            MuEditor::WindowIds::Title(title, I18N::Editor::ServerList, MuEditor::WindowIds::ServerList), open,
            ImGuiWindowFlags_NoCollapse | MuEditor::Layout::ToolWindowBarFlags(MuEditor::WindowIds::ServerList)))
    {
        ImGui::End();
        return false;
    }
    // Before the bar: in this window's own bar it shows the Server List's undo then.
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        MuEditor::UndoTargets::Activate(*this);
    }
    MuEditor::Layout::RenderToolWindowBar();
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
    {
        g_MuEditorCore.SetHoveringUI(true);
    }
    HandleUndoKeys();
    return true;
}

bool CMuServerListUI::RenderPanes()
{
    bool changed = false;
    const ImVec2 listSize(GroupListWidth * g_MuEditorCore.GetUIScale(), 0.0f);
    if (ImGui::BeginChild("##Groups", listSize, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
    {
        changed |= RenderGroupList();
        changed |= RenderListButtons();
        changed |= RenderGroupsNotInList();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    if (ImGui::BeginChild("##Group"))
    {
        changed |= RenderSelectedGroup();
    }
    ImGui::EndChild();
    return changed;
}

void CMuServerListUI::AfterEdits(bool changed)
{
    if (changed)
    {
        m_uncommitted = true;
        ShowInGame();
    }
    // A text field changes the groups with each key; its whole edit is one
    // step, taken when it is left (CommitWhenFieldLeft) or, should it go away
    // unseen, once nothing is active.
    if (m_uncommitted && !ImGui::IsAnyItemActive())
    {
        CommitPending();
    }
}

void CMuServerListUI::CommitPending()
{
    m_edits.Commit();
    m_uncommitted = false;
}

void CMuServerListUI::CommitWhenFieldLeft()
{
    // Also when a button or box took the activation from the field: it
    // changes the groups only after this, on its release.
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        CommitPending();
    }
}

void CMuServerListUI::LoadFile()
{
    Data::ServerList::ServerListLoadResult loaded =
        Data::ServerList::LoadServerListFile(Data::ServerList::ServerListFile);
    m_edits.Reset(std::move(loaded.groups), !Data::Items::HasErrors(loaded.issues));
    m_issues = std::move(loaded.issues);
    m_status.clear();
    m_statusIsError = false;
    m_loaded = true;
    m_uncommitted = false;
    m_takenId = -1;
    Select(m_edits.GetGroups().empty() ? -1
                                       : std::clamp(m_selected, 0, static_cast<int>(m_edits.GetGroups().size()) - 1));
    ShowInGame();
}

void CMuServerListUI::Save()
{
    if (!m_edits.IsComplete())
    {
        return;
    }
    m_edits.Commit();
    m_issues.clear();
    if (!Data::ServerList::SaveServerListFile(Data::ServerList::ServerListFile, m_edits.GetGroups(), m_issues))
    {
        m_status = I18N::Editor::NotSavedSeeTheProblemsBelow;
        m_statusIsError = true;
        return;
    }
    m_edits.MarkSaved();
    m_status = I18N::Format(I18N::Editor::SavedTo0, {Data::ServerList::ServerListFile});
    m_statusIsError = false;
}

void CMuServerListUI::RenderToolbar()
{
    // A save after a read with errors would remove the groups the read left out.
    ImGui::BeginDisabled(!m_edits.IsComplete());
    if (ImGui::Button(I18N::Editor::Save))
    {
        Save();
    }
    ImGui::EndDisabled();
    if (!m_edits.IsComplete() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::SaveOffAfterReadErrors);
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Revert))
    {
        LoadFile();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip(
            "%s",
            I18N::Format(I18N::Editor::Reads0AgainUnsavedChangesAreLost, {Data::ServerList::ServerListFile}).c_str());
    }
    if (m_edits.HasUnsavedChanges())
    {
        ImGui::SameLine();
        ImGui::TextUnformatted(I18N::Editor::UnsavedChanges);
    }
}

bool CMuServerListUI::IsOpen() const
{
    return g_MuEditorCore.GetToolWindows().serverList;
}

bool CMuServerListUI::CanUndo() const
{
    return m_edits.CanUndo() || m_uncommitted;
}

bool CMuServerListUI::CanRedo() const
{
    return m_edits.CanRedo();
}

std::string CMuServerListUI::GetUndoLabel() const
{
    return m_edits.GetUndoLabel();
}

std::string CMuServerListUI::GetRedoLabel() const
{
    return m_edits.GetRedoLabel();
}

void CMuServerListUI::Undo()
{
    m_edits.Undo();
    AfterHistoryStep();
}

void CMuServerListUI::Redo()
{
    m_edits.Redo();
    AfterHistoryStep();
}

void CMuServerListUI::AfterHistoryStep()
{
    m_uncommitted = false;
    m_takenId = -1;
    Select(std::min(m_selected, static_cast<int>(m_edits.GetGroups().size()) - 1));
    ShowInGame();
}

void CMuServerListUI::HandleUndoKeys()
{
    // Only while the keyboard is the editor's: ImGui keeps the focus here when
    // the game takes the keyboard back, e.g. to type into the login screen. A
    // text field being edited takes the keys first.
    if (!g_MuEditorCore.IsKeyboardWithEditor())
    {
        return;
    }
    if (CanUndo() && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z))
    {
        Undo();
    }
    if (CanRedo() &&
        (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y) || ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z)))
    {
        Redo();
    }
}

void CMuServerListUI::RenderStatus()
{
    RenderNote(I18N::Format(I18N::Editor::ServerListOrderNote, {Data::ServerList::ServerListFile}).c_str());
    if (!m_status.empty())
    {
        if (m_statusIsError)
        {
            RenderError(m_status.c_str());
        }
        else
        {
            ImGui::TextUnformatted(m_status.c_str());
        }
    }
    if (!m_edits.IsComplete())
    {
        RenderError(I18N::Editor::SaveOffAfterReadErrors);
    }
    if (m_issues.empty())
    {
        return;
    }
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::ProblemsIn0, {Data::ServerList::ServerListFile}).c_str());
    for (const Data::Items::ItemDataIssue& issue : m_issues)
    {
        const std::string line = issue.field + ": " + issue.message;
        if (issue.severity == Data::Items::ItemDataIssueSeverity::Error)
        {
            RenderError(line.c_str());
        }
        else
        {
            RenderNote(line.c_str());
        }
    }
}

bool CMuServerListUI::RenderGroupList()
{
    const std::vector<ServerGroup>& groups = m_edits.GetGroups();
    for (int index = 0; index < static_cast<int>(groups.size()); ++index)
    {
        const ServerGroup& group = groups[index];
        ImGui::PushID(index);
        const std::string label = (group.name.empty() ? std::string("?") : group.name) + " (" +
                                  std::to_string(group.id) + ")" + (group.testServer ? " *" : "");
        // The name is drawn as plain text: as the row's label, a "##" in it
        // would start the ImGui ID and hide the rest.
        const ImVec2 textPosition = ImGui::GetCursorScreenPos();
        if (ImGui::Selectable("##Group", index == m_selected, MuEditor::KeyboardSelection::RowFlags))
        {
            Select(index);
        }
        ImGui::GetWindowDrawList()->AddText(textPosition, ImGui::GetColorU32(ImGuiCol_Text), label.c_str());
        ImGui::PopID();
    }
    return false;
}

bool CMuServerListUI::RenderListButtons()
{
    bool changed = false;
    if (ImGui::Button(I18N::Editor::Add))
    {
        CommitPending();
        if (const std::optional<size_t> added = m_edits.AddGroup(I18N::Editor::NewGroup))
        {
            Select(static_cast<int>(*added));
            changed = true;
        }
    }
    ImGui::SameLine();
    const bool hasSelection = m_selected >= 0;
    ImGui::BeginDisabled(!hasSelection);
    if (ImGui::Button(I18N::Editor::Remove))
    {
        CommitPending();
        m_edits.RemoveGroup(static_cast<size_t>(m_selected));
        Select(std::min(m_selected, static_cast<int>(m_edits.GetGroups().size()) - 1));
        changed = true;
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(!hasSelection || m_selected == 0);
    if (ImGui::Button(I18N::Editor::MoveUp))
    {
        CommitPending();
        if (m_edits.MoveGroup(static_cast<size_t>(m_selected), -1))
        {
            Select(m_selected - 1);
            changed = true;
        }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!hasSelection || m_selected + 1 >= static_cast<int>(m_edits.GetGroups().size()));
    if (ImGui::Button(I18N::Editor::MoveDown))
    {
        CommitPending();
        if (m_edits.MoveGroup(static_cast<size_t>(m_selected), 1))
        {
            Select(m_selected + 1);
            changed = true;
        }
    }
    ImGui::EndDisabled();
    return changed;
}

bool CMuServerListUI::RenderGroupsNotInList()
{
    bool headerShown = false;
    bool changed = false;
    for (const auto& [groupId, channels] : m_sent)
    {
        if (m_edits.IsIdUsed(groupId))
        {
            continue;
        }
        if (!headerShown)
        {
            ImGui::Separator();
            RenderNote(I18N::Editor::GroupsTheListDoesNotHave);
            headerShown = true;
        }
        ImGui::PushID(groupId);
        ImGui::TextUnformatted(
            I18N::Format(I18N::Editor::Group0Servers1, {std::to_string(groupId), ListServerIds(channels, groupId)})
                .c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::Add))
        {
            CommitPending();
            if (const std::optional<size_t> added = m_edits.AddGroup(I18N::Editor::NewGroup, groupId))
            {
                Select(static_cast<int>(*added));
                changed = true;
            }
        }
        ImGui::PopID();
    }
    return changed;
}

bool CMuServerListUI::RenderSelectedGroup()
{
    std::vector<ServerGroup>& groups = m_edits.EditGroups();
    if (m_selected < 0 || m_selected >= static_cast<int>(groups.size()))
    {
        ImGui::TextDisabled("%s", I18N::Editor::PickAGroupInTheList);
        return false;
    }
    ServerGroup& group = groups[static_cast<size_t>(m_selected)];
    bool changed = RenderGroupId(group);
    changed |= RenderGroupFields(group);
    ImGui::Separator();
    changed |= RenderChannels(group);
    return changed;
}

bool CMuServerListUI::RenderGroupId(ServerGroup& group)
{
    int id = group.id;
    ImGui::SetNextItemWidth(IdFieldWidth * g_MuEditorCore.GetUIScale());
    bool changed = false;
    // Typed only: a step button would stop at an ID another group has.
    if (ImGui::InputInt(I18N::Editor::GroupID, &id, 0, 0))
    {
        id = std::clamp(id, 0, Data::ServerList::MaxServerGroupId);
        // An ID another group has stays with it; the field says so.
        const bool taken = id != group.id && m_edits.IsIdUsed(id);
        m_takenId = taken ? id : -1;
        changed = m_takenId < 0 && id != group.id;
        if (changed)
        {
            group.id = id;
        }
    }
    CommitWhenFieldLeft();
    const int firstServer = group.id * Data::ServerList::MaxChannelsPerGroup;
    ImGui::SameLine();
    ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::Servers0To1,
                                           {std::to_string(firstServer),
                                            std::to_string(firstServer + Data::ServerList::MaxChannelsPerGroup - 1)})
                                  .c_str());
    if (m_takenId >= 0)
    {
        RenderError(I18N::Format(I18N::Editor::AnotherGroupHasTheID0, {std::to_string(m_takenId)}).c_str());
    }
    return changed;
}

bool CMuServerListUI::RenderGroupFields(ServerGroup& group)
{
    const float scale = g_MuEditorCore.GetUIScale();
    ImGui::SetNextItemWidth(TextFieldWidth * scale);
    bool changed = false;
    if (MuEditor::Items::InputString(I18N::Editor::Name, group.name))
    {
        Core::Text::CutToUtf16Length(group.name, Data::ServerList::MaxServerGroupNameLength);
        changed = true;
    }
    CommitWhenFieldLeft();
    if (group.name.empty())
    {
        RenderError(I18N::Editor::AGroupNeedsAName);
    }
    ImGui::SetNextItemWidth(TextFieldWidth * scale);
    if (MuEditor::Items::InputString(I18N::Editor::Description, group.description))
    {
        Core::Text::CutToUtf16Length(group.description, Data::ServerList::MaxServerGroupDescriptionLength);
        changed = true;
    }
    CommitWhenFieldLeft();
    RenderNote(I18N::Editor::DescriptionNote);
    bool testServer = group.testServer;
    if (ImGui::Checkbox(I18N::Editor::TestServer, &testServer))
    {
        CommitPending();
        group.testServer = testServer;
        changed = true;
    }
    RenderNote(I18N::Editor::TestServerNote);
    return changed;
}

bool CMuServerListUI::RenderChannels(ServerGroup& group)
{
    if (!BeginChannelTable())
    {
        return false;
    }
    const auto sentGroup = m_sent.find(group.id);
    const std::map<int, int>* sentChannels = sentGroup != m_sent.end() ? &sentGroup->second : nullptr;
    bool changed = false;
    for (int channel = 0; channel < Data::ServerList::MaxChannelsPerGroup; ++channel)
    {
        if (const std::optional<ChannelType> picked = RenderChannelRow(group, channel, sentChannels))
        {
            CommitPending();
            group.channels[static_cast<size_t>(channel)] = *picked;
            changed = true;
        }
    }
    ImGui::EndTable();
    return changed;
}

void CMuServerListUI::ShowInGame()
{
    g_ServerListManager->SetServerGroups(m_edits.GetGroups());
    // Only the login screen shows the groups; elsewhere the next list the
    // connect server sends gets the new names.
    if (SceneFlag != LOG_IN_SCENE)
    {
        return;
    }
    // The login screen knows its open group by its button, which a group
    // moved or removed in the list changes; it is found again by its ID.
    const int openGroupId = g_ServerSelWin.GetOpenGroupId();
    g_ServerListManager->RebuildServerGroups();
    // Also while the server select window is hidden (after a server was
    // picked): it keeps a pointer to the open group, which the rebuild deleted.
    g_ServerSelWin.ShowRebuiltGroups(openGroupId);
}

void CMuServerListUI::Select(int index)
{
    if (index != m_selected)
    {
        m_takenId = -1;
    }
    m_selected = index;
}

#endif // _EDITOR

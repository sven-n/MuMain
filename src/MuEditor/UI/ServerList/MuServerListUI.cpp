#include "stdafx.h"

#ifdef _EDITOR

#include "MuServerListUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/IssueLevels.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Common/SavedDataFiles.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ServerListData/ChannelNameBuilder.h"
#include "Data/GameData/ServerListData/ServerListJson.h"
#include "I18N/All.h"
#include "Network/Server/ServerListManager.h"
#include "Scenes/SceneCore.h"
#include "ServerListStepLabel.h"
#include "UI/Windows/ServerSelWin.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <map>
#include <optional>
#include <span>
#include <string>

using Data::LocalizedString;
using Data::ServerList::ChannelNamePlace;
using Data::ServerList::ChannelSettings;
using Data::ServerList::ServerGroup;
using Data::ServerList::ServerListData;
using Data::ServerList::ThemeColor;
using Data::ServerList::VipType;

namespace
{
// Pixel widths before the editor UI scale.
constexpr float GroupListWidth = 220.0f;
constexpr float IdFieldWidth = 120.0f;
constexpr float TextFieldWidth = 320.0f;
constexpr float VipComboWidth = 140.0f;
constexpr int ChannelTableColumns = 5;
constexpr int PreviewTableColumns = 2 + Data::ServerList::ChannelNamePlaceCount;
constexpr ImVec4 ErrorColor{1.0f, 0.4f, 0.4f, 1.0f};
// {character} in the preview.
constexpr const char* PreviewCharacter = "Hero";

std::string_view GetEditorLocale()
{
    const char* locale = I18N::GetCurrentLocale();
    return locale != nullptr ? std::string_view(locale) : LocalizedString::NeutralLocale;
}

// A VIP type's name in `locale`; None for no type.
std::string GetVipTypeLabel(const VipType* vipType, std::string_view locale)
{
    return vipType != nullptr ? vipType->name.Get(locale) : std::string(I18N::Editor::None);
}

const char* GetThemeColorLabel(ThemeColor color)
{
    switch (color)
    {
    case ThemeColor::Gray:
        return I18N::Editor::Gray;
    case ThemeColor::Orange:
        return I18N::Editor::Orange;
    case ThemeColor::Silver:
        return I18N::Editor::Silver;
    case ThemeColor::Default:
    default:
        return I18N::Editor::Default;
    }
}

// The shade the legacy theme draws (themes/legacy/base.rcss), for the swatches.
ImVec4 GetThemeColorSwatch(ThemeColor color)
{
    switch (color)
    {
    case ThemeColor::Gray:
        return {0xE2 / 255.0f, 0xE2 / 255.0f, 0xE2 / 255.0f, 1.0f};
    case ThemeColor::Orange:
        return {0xFF / 255.0f, 0xB4 / 255.0f, 0x00 / 255.0f, 1.0f};
    case ThemeColor::Silver:
        return {0xB9 / 255.0f, 0xC6 / 255.0f, 0xD3 / 255.0f, 1.0f};
    case ThemeColor::Default:
    default:
        return {0xFF / 255.0f, 0xFF / 255.0f, 0x79 / 255.0f, 1.0f};
    }
}

void RenderSwatch(ThemeColor color)
{
    const float size = ImGui::GetTextLineHeight();
    ImGui::ColorButton("##Color", GetThemeColorSwatch(color),
                       ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoDragDrop,
                       ImVec2(size, size));
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
    ImGui::TableSetupColumn(I18N::Editor::PvP);
    ImGui::TableSetupColumn(I18N::Editor::VIP);
    ImGui::TableSetupColumn(I18N::Editor::OnTheLoginScreen);
    ImGui::TableSetupColumn(I18N::Editor::ConnectServer);
    ImGui::TableHeadersRow();
    return true;
}

// The VIP type picked in the combo, when it is another one than `vip`: its id,
// or empty for none.
std::optional<std::string> PickVipType(const ServerListData& list, const std::string& vip)
{
    ImGui::SetNextItemWidth(VipComboWidth * g_MuEditorCore.GetUIScale());
    const std::string_view locale = GetEditorLocale();
    const VipType* current = vip.empty() ? nullptr : list.FindVipType(vip);
    if (!ImGui::BeginCombo("##Vip", GetVipTypeLabel(current, locale).c_str()))
    {
        return std::nullopt;
    }
    std::optional<std::string> picked;
    if (ImGui::Selectable(I18N::Editor::None, current == nullptr) && current != nullptr)
    {
        picked = std::string();
    }
    for (size_t index = 0; index < list.vipTypes.size(); ++index)
    {
        const VipType& vipType = list.vipTypes[index];
        ImGui::PushID(static_cast<int>(index));
        if (ImGui::Selectable(GetVipTypeLabel(&vipType, locale).c_str(), &vipType == current) && &vipType != current)
        {
            picked = vipType.id;
        }
        ImGui::PopID();
    }
    ImGui::EndCombo();
    return picked;
}

// One channel: its number, PvP box, VIP type, button text on the login screen
// and the load the connect server sent for it (sentChannels: the group's, or
// nullptr). Returns its settings when they were changed; the caller sets them.
std::optional<ChannelSettings> RenderChannelRow(const ServerListData& list, const ServerGroup& group, int channel,
                                                const std::map<int, int>* sentChannels)
{
    ImGui::PushID(channel);
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("%d", channel + 1);

    ChannelSettings settings = group.channels[static_cast<size_t>(channel)];
    bool changed = false;
    ImGui::TableNextColumn();
    bool pvp = settings.pvp;
    if (ImGui::Checkbox("##PvP", &pvp))
    {
        settings.pvp = pvp;
        changed = true;
    }
    ImGui::TableNextColumn();
    if (std::optional<std::string> picked = PickVipType(list, settings.vip))
    {
        settings.vip = std::move(*picked);
        changed = true;
    }

    ImGui::TableNextColumn();
    Data::ServerList::ChannelNameValues values;
    values.name = group.name;
    values.channel = channel + 1;
    const std::string name =
        Data::ServerList::BuildChannelName(list, ChannelNamePlace::Button, settings, values, GetEditorLocale());
    ImGui::TextDisabled("%s", name.c_str());

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
    return changed ? std::optional<ChannelSettings>(std::move(settings)) : std::nullopt;
}

// What a template shows as written: a brace problem, or a placeholder its place does not have.
void RenderTemplateProblems(std::string_view text, ChannelNamePlace place)
{
    const Data::Items::PlaceholderScan scan = Data::Items::ScanPlaceholders(text);
    if (!scan.error.empty())
    {
        RenderError(scan.error.c_str());
        return;
    }
    const std::span<const std::string_view> known = Data::ServerList::GetChannelNamePlaceholders(place);
    for (const std::string& name : scan.names)
    {
        if (std::find(known.begin(), known.end(), name) == known.end())
        {
            RenderError(I18N::Format(I18N::Editor::NotAPlaceholderOfThisTemplate0, {"{" + name + "}"}).c_str());
        }
    }
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
    bool changed = false;
    if (ImGui::BeginTabBar("##ServerListTabs"))
    {
        // "###": the tab keeps its ID when the editor's language changes.
        const std::string groupsTab = std::string(I18N::Editor::Groups) + "###Groups";
        if (ImGui::BeginTabItem(groupsTab.c_str()))
        {
            changed |= RenderPanes();
            ImGui::EndTabItem();
        }
        const std::string namesTab = std::string(I18N::Editor::ChannelNames) + "###ChannelNames";
        if (ImGui::BeginTabItem(namesTab.c_str()))
        {
            changed |= RenderChannelNamesTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
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
        Data::ServerList::LoadServerListFile(Data::ServerList::ServerListFile, I18N::GetAvailableLocales());
    m_edits.Reset(std::move(loaded.list), !Data::Items::HasFatal(loaded.issues));
    m_issues = std::move(loaded.issues);
    m_status.clear();
    m_statusIsError = false;
    m_loaded = true;
    m_uncommitted = false;
    m_takenId = -1;
    Select(m_edits.GetGroups().empty() ? -1
                                       : std::clamp(m_selected, 0, static_cast<int>(m_edits.GetGroups().size()) - 1));
    const int vipTypes = static_cast<int>(m_edits.GetList().vipTypes.size());
    SelectVipType(vipTypes == 0 ? -1 : std::clamp(m_selectedVipType, 0, vipTypes - 1));
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
    if (!Data::ServerList::SaveServerListFile(Data::ServerList::ServerListFile, m_edits.GetList(), m_issues))
    {
        m_status = I18N::Editor::NotSavedSeeTheProblemsBelow;
        m_statusIsError = true;
        return;
    }
    m_edits.MarkSaved();
    MuEditor::SavedDataFiles::NoteSaved();
    m_status = I18N::Format(I18N::Editor::SavedTo0, {Data::ServerList::ServerListFile});
    m_statusIsError = false;
}

void CMuServerListUI::RenderToolbar()
{
    // A save after a read with fatal problems would remove the groups the read left out.
    ImGui::BeginDisabled(!m_edits.IsComplete());
    if (ImGui::Button(I18N::Editor::Save))
    {
        Save();
    }
    ImGui::EndDisabled();
    if (!m_edits.IsComplete() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::SaveOffAfterFatalReadProblems);
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
    SelectVipType(std::min(m_selectedVipType, static_cast<int>(m_edits.GetList().vipTypes.size()) - 1));
    m_removeVipType = -1;
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
        RenderError(I18N::Editor::SaveOffAfterFatalReadProblems);
    }
    if (m_issues.empty())
    {
        return;
    }
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::ProblemsIn0, {Data::ServerList::ServerListFile}).c_str());
    for (const Data::Items::ItemDataIssue& issue : m_issues)
    {
        const std::string line = issue.field + ": " + issue.message;
        MuEditor::IssueLevels::RenderLine(issue.severity, line.c_str());
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
        if (std::optional<ChannelSettings> picked = RenderChannelRow(m_edits.GetList(), group, channel, sentChannels))
        {
            CommitPending();
            group.channels[static_cast<size_t>(channel)] = std::move(*picked);
            changed = true;
        }
    }
    ImGui::EndTable();
    return changed;
}

bool CMuServerListUI::RenderChannelNamesTab()
{
    const float scale = g_MuEditorCore.GetUIScale();
    bool changed = false;
    const ImVec2 listSize(GroupListWidth * scale, 0.0f);
    if (ImGui::BeginChild("##VipTypes", listSize, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
    {
        ImGui::TextUnformatted(I18N::Editor::VIPTypes);
        changed |= RenderVipTypeList();
        changed |= RenderVipTypeButtons();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    if (ImGui::BeginChild("##Names", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::SetNextItemWidth(IdFieldWidth * 1.5f * scale);
        if (ImGui::BeginCombo(I18N::Editor::Language, I18N::GetLanguageDisplayName(m_textLocale.c_str())))
        {
            for (const char* locale : I18N::GetAvailableLocales())
            {
                if (ImGui::Selectable(I18N::GetLanguageDisplayName(locale), m_textLocale == locale))
                {
                    m_textLocale = locale;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Separator();
        changed |= RenderSelectedVipType();
        ImGui::Separator();
        changed |= RenderTexts();
        ImGui::Separator();
        RenderPreview();
    }
    ImGui::EndChild();
    return changed;
}

bool CMuServerListUI::RenderVipTypeList()
{
    const std::vector<VipType>& vipTypes = m_edits.GetList().vipTypes;
    const std::string_view locale = GetEditorLocale();
    for (int index = 0; index < static_cast<int>(vipTypes.size()); ++index)
    {
        const VipType& vipType = vipTypes[static_cast<size_t>(index)];
        ImGui::PushID(index);
        RenderSwatch(vipType.themeColor);
        ImGui::SameLine();
        const std::string label = GetVipTypeLabel(&vipType, locale) + " (" + vipType.id + ")";
        // Drawn as plain text, as the group names are: a "##" in it would start the ImGui ID.
        const ImVec2 textPosition = ImGui::GetCursorScreenPos();
        if (ImGui::Selectable("##VipType", index == m_selectedVipType, MuEditor::KeyboardSelection::RowFlags))
        {
            SelectVipType(index);
        }
        ImGui::GetWindowDrawList()->AddText(textPosition, ImGui::GetColorU32(ImGuiCol_Text), label.c_str());
        ImGui::PopID();
    }
    return false;
}

bool CMuServerListUI::RenderVipTypeButtons()
{
    bool changed = false;
    if (ImGui::Button(I18N::Editor::Add))
    {
        CommitPending();
        SelectVipType(static_cast<int>(m_edits.AddVipType(I18N::Editor::NewVIPType)));
        changed = true;
    }
    ImGui::SameLine();
    const int count = static_cast<int>(m_edits.GetList().vipTypes.size());
    const bool hasSelection = m_selectedVipType >= 0 && m_selectedVipType < count;
    ImGui::BeginDisabled(!hasSelection);
    if (ImGui::Button(I18N::Editor::Remove))
    {
        const VipType& vipType = m_edits.GetList().vipTypes[static_cast<size_t>(m_selectedVipType)];
        if (m_edits.FindChannelsWithVipType(vipType.id).empty())
        {
            CommitPending();
            m_edits.RemoveVipType(static_cast<size_t>(m_selectedVipType));
            SelectVipType(std::min(m_selectedVipType, count - 2));
            changed = true;
        }
        else
        {
            m_removeVipType = m_selectedVipType;
            ImGui::OpenPopup("###RemoveVipType");
        }
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(!hasSelection || m_selectedVipType == 0);
    if (ImGui::Button(I18N::Editor::MoveUp))
    {
        CommitPending();
        if (m_edits.MoveVipType(static_cast<size_t>(m_selectedVipType), -1))
        {
            SelectVipType(m_selectedVipType - 1);
            changed = true;
        }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!hasSelection || m_selectedVipType + 1 >= count);
    if (ImGui::Button(I18N::Editor::MoveDown))
    {
        CommitPending();
        if (m_edits.MoveVipType(static_cast<size_t>(m_selectedVipType), 1))
        {
            SelectVipType(m_selectedVipType + 1);
            changed = true;
        }
    }
    ImGui::EndDisabled();
    // In the ID scope of the Remove button, which opens it.
    changed |= RenderRemoveVipTypePopup();
    return changed;
}

bool CMuServerListUI::RenderRemoveVipTypePopup()
{
    const std::vector<VipType>& vipTypes = m_edits.GetList().vipTypes;
    if (m_removeVipType < 0 || m_removeVipType >= static_cast<int>(vipTypes.size()))
    {
        return false;
    }
    const VipType& vipType = vipTypes[static_cast<size_t>(m_removeVipType)];
    const std::string title =
        I18N::Format(I18N::Editor::RemoveVIPType0, {vipType.name.Get(GetEditorLocale())}) + "###RemoveVipType";
    bool changed = false;
    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        const std::vector<std::string> channels = m_edits.FindChannelsWithVipType(vipType.id);
        ImGui::TextUnformatted(I18N::Format(I18N::Editor::VIPTypeUsedNote, {std::to_string(channels.size())}).c_str());
        std::string list;
        for (const std::string& channel : channels)
        {
            list += (list.empty() ? "" : ", ") + channel;
        }
        ImGui::PushTextWrapPos(TextFieldWidth * 1.5f * g_MuEditorCore.GetUIScale());
        ImGui::TextUnformatted(list.c_str());
        ImGui::PopTextWrapPos();
        if (ImGui::Button(I18N::Editor::Remove))
        {
            CommitPending();
            m_edits.RemoveVipType(static_cast<size_t>(m_removeVipType));
            SelectVipType(std::min(m_selectedVipType, static_cast<int>(m_edits.GetList().vipTypes.size()) - 1));
            m_removeVipType = -1;
            changed = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(I18N::Editor::Cancel))
        {
            m_removeVipType = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return changed;
}

bool CMuServerListUI::RenderLocalizedField(const char* label, LocalizedString& text, size_t maxUnits)
{
    const bool english = m_textLocale == LocalizedString::NeutralLocale;
    std::string value = english ? text.GetNeutral() : text.GetOwn(m_textLocale);
    ImGui::SetNextItemWidth(TextFieldWidth * g_MuEditorCore.GetUIScale());
    const bool edited = english ? MuEditor::Items::InputString(label, value)
                                : MuEditor::Items::InputStringWithHint(label, text.GetNeutral().c_str(), value);
    if (edited)
    {
        Core::Text::CutToUtf16Length(value, maxUnits);
        text.Set(m_textLocale, std::move(value));
    }
    CommitWhenFieldLeft();
    return edited;
}

bool CMuServerListUI::RenderSelectedVipType()
{
    std::vector<VipType>& vipTypes = m_edits.EditList().vipTypes;
    if (m_selectedVipType < 0 || m_selectedVipType >= static_cast<int>(vipTypes.size()))
    {
        ImGui::TextDisabled("%s", I18N::Editor::PickAVIPTypeInTheList);
        return false;
    }
    const size_t index = static_cast<size_t>(m_selectedVipType);
    const float scale = g_MuEditorCore.GetUIScale();
    bool changed = false;

    // The ID changes the type only while it is a free VIP type id; what is typed stays in the field until it is left.
    if (!m_vipTypeIdActive)
    {
        m_vipTypeIdDraft = vipTypes[index].id;
    }
    ImGui::SetNextItemWidth(IdFieldWidth * 1.5f * scale);
    if (MuEditor::Items::InputString(I18N::Editor::VIPTypeID, m_vipTypeIdDraft))
    {
        const std::string before = vipTypes[index].id;
        changed = m_edits.ChangeVipTypeId(index, m_vipTypeIdDraft) && vipTypes[index].id != before;
    }
    m_vipTypeIdActive = ImGui::IsItemActive();
    CommitWhenFieldLeft();
    if (m_vipTypeIdDraft != vipTypes[index].id)
    {
        RenderError(Data::ServerList::IsVipTypeId(m_vipTypeIdDraft)
                        ? I18N::Format(I18N::Editor::AnotherVIPTypeHasTheID0, {m_vipTypeIdDraft}).c_str()
                        : I18N::Editor::NotAVIPTypeID);
    }
    RenderNote(I18N::Editor::VIPTypeIDNote);

    VipType& vipType = vipTypes[index];
    changed |= RenderLocalizedField(I18N::Editor::Name, vipType.name, Data::ServerList::MaxVipTypeNameLength);
    if (vipType.name.GetNeutral().empty())
    {
        RenderError(I18N::Editor::AVIPTypeNeedsAName);
    }

    ImGui::SetNextItemWidth(VipComboWidth * scale);
    if (ImGui::BeginCombo(I18N::Editor::ThemeColor, GetThemeColorLabel(vipType.themeColor)))
    {
        for (int value = 0; value < Data::ServerList::ThemeColorCount; ++value)
        {
            const ThemeColor color = static_cast<ThemeColor>(value);
            ImGui::PushID(value);
            RenderSwatch(color);
            ImGui::PopID();
            ImGui::SameLine();
            if (ImGui::Selectable(GetThemeColorLabel(color), color == vipType.themeColor) &&
                color != vipType.themeColor)
            {
                CommitPending();
                vipType.themeColor = color;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    RenderNote(I18N::Editor::ThemeColorNote);
    return changed;
}

bool CMuServerListUI::RenderTexts()
{
    Data::ServerList::ChannelNames& names = m_edits.EditList().names;
    ImGui::TextUnformatted(I18N::Editor::Texts);
    bool changed = RenderLocalizedField(I18N::Editor::PvPText, names.pvp, Data::ServerList::MaxChannelNameTextLength);
    changed |= RenderLocalizedField(I18N::Editor::NonPvPText, names.nonPvp, Data::ServerList::MaxChannelNameTextLength);
    for (int value = 0; value < Data::ServerList::ChannelNamePlaceCount; ++value)
    {
        const ChannelNamePlace place = static_cast<ChannelNamePlace>(value);
        LocalizedString& text = names.Template(place);
        changed |= RenderLocalizedField(MuEditor::ServerList::StepLabel::GetPlaceLabel(place), text,
                                        Data::ServerList::MaxChannelNameTextLength);
        RenderTemplateProblems(text.Get(m_textLocale), place);
    }
    RenderNote(I18N::Editor::TextsNote);
    return changed;
}

void CMuServerListUI::RenderPreview()
{
    ImGui::TextUnformatted(I18N::Editor::Preview);
    RenderNote(I18N::Editor::PreviewNote);
    constexpr ImGuiTableFlags TableFlags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Preview", PreviewTableColumns, TableFlags))
    {
        return;
    }
    ImGui::TableSetupColumn(I18N::Editor::PvP);
    ImGui::TableSetupColumn(I18N::Editor::VIP);
    for (int value = 0; value < Data::ServerList::ChannelNamePlaceCount; ++value)
    {
        ImGui::TableSetupColumn(MuEditor::ServerList::StepLabel::GetPlaceLabel(static_cast<ChannelNamePlace>(value)));
    }
    ImGui::TableHeadersRow();

    const ServerListData& list = m_edits.GetList();
    const std::vector<ServerGroup>& groups = list.groups;
    const bool hasGroup = m_selected >= 0 && m_selected < static_cast<int>(groups.size());
    const std::string groupName = hasGroup ? groups[static_cast<size_t>(m_selected)].name : std::string("Valhalla");
    const std::string load = Core::Text::ToUtf8(I18N::Game::Connect);
    Data::ServerList::ChannelNameValues values;
    values.name = groupName;
    values.channel = 1;
    values.load = load;
    values.character = PreviewCharacter;

    // No VIP type first, then each one; each with PvP on and off.
    std::vector<const VipType*> vipTypes = {nullptr};
    for (const VipType& vipType : list.vipTypes)
    {
        vipTypes.push_back(&vipType);
    }
    for (const VipType* vipType : vipTypes)
    {
        for (const bool pvp : {true, false})
        {
            ChannelSettings channel;
            channel.pvp = pvp;
            channel.vip = vipType != nullptr ? vipType->id : std::string();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted((pvp ? list.names.pvp : list.names.nonPvp).Get(m_textLocale).c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(GetVipTypeLabel(vipType, m_textLocale).c_str());
            for (int value = 0; value < Data::ServerList::ChannelNamePlaceCount; ++value)
            {
                ImGui::TableNextColumn();
                const std::string name = Data::ServerList::BuildChannelName(list, static_cast<ChannelNamePlace>(value),
                                                                            channel, values, m_textLocale);
                ImGui::TextUnformatted(name.c_str());
            }
        }
    }
    ImGui::EndTable();
}

void CMuServerListUI::ShowInGame()
{
    g_ServerListManager->SetServerList(m_edits.GetList());
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

void CMuServerListUI::SelectVipType(int index)
{
    if (index != m_selectedVipType)
    {
        m_vipTypeIdActive = false;
    }
    m_selectedVipType = index;
}

#endif // _EDITOR

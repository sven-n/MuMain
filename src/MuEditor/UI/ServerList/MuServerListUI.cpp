#include "stdafx.h"

#ifdef _EDITOR

#include "MuServerListUI.h"

#include "../MuEditor/Config/MuEditorConfig.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/IssueLevels.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Common/SavedDataFiles.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ServerListData/ButtonColors.h"
#include "Data/GameData/ServerListData/ChannelNameBuilder.h"
#include "Data/GameData/ServerListData/LoadBar.h"
#include "Data/GameData/ServerListData/ServerListIds.h"
#include "Data/GameData/ServerListData/ServerListJson.h"
#include "I18N/All.h"
#include "Network/Server/ServerListManager.h"
#include "Scenes/SceneCore.h"
#include "ServerListColors.h"
#include "ServerListLoadUI.h"
#include "ServerListStepLabel.h"
#include "../MuEditor/UI/Colors/MuColorsUI.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Windows/ServerSelWin.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <map>
#include <optional>
#include <span>
#include <string>

using Data::LocalizedString;
using Data::ServerList::ButtonColorRule;
using Data::ServerList::ChannelNamePlace;
using Data::ServerList::ChannelSettings;
using Data::ServerList::RuleVip;
using Data::ServerList::ServerGroup;
using Data::ServerList::ServerListData;
using Data::ServerList::VipType;
using namespace MuEditor::ServerList::Colors;

namespace
{
// Pixel widths before the editor UI scale.
constexpr float GroupListWidth = 220.0f;
constexpr float IdFieldWidth = 120.0f;
constexpr float TextFieldWidth = 320.0f;
constexpr float VipComboWidth = 140.0f;
// The ID fields and the language list, which hold a little more than a group ID.
constexpr float WideIdFieldWidth = IdFieldWidth * 1.5f;
// The PvP list of a button color rule: "Any", "On" or "Off".
constexpr float PvpComboWidth = VipComboWidth * 0.6f;
// Where the channel list of the remove VIP type question wraps.
constexpr float PopupWrapWidth = TextFieldWidth * 1.5f;
constexpr int ChannelTableColumns = 5;
// The load slider of the offline servers' preview.
constexpr float OfflineLoadWidth = 140.0f;
constexpr int PreviewTableColumns = 3 + Data::ServerList::ChannelNamePlaceCount;
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

std::string GetRuleVipLabel(const ServerListData& list, const ButtonColorRule& rule, std::string_view locale)
{
    switch (rule.vipMatch)
    {
    case RuleVip::AnyVipType:
        return I18N::Editor::AnyVIPType;
    case RuleVip::NoVipType:
        return I18N::Editor::None;
    case RuleVip::VipType:
    {
        const VipType* vipType = list.FindVipType(rule.vip);
        return vipType != nullptr ? GetVipTypeLabel(vipType, locale) : rule.vip;
    }
    case RuleVip::Any:
    default:
        return I18N::Editor::Any;
    }
}

const char* GetRulePvpLabel(std::optional<bool> pvp)
{
    if (!pvp)
    {
        return I18N::Editor::Any;
    }
    return *pvp ? I18N::Editor::On : I18N::Editor::Off;
}

// The rule with another VIP setting picked in the combo, in `rule`; true when one was.
bool PickRuleVip(const ServerListData& list, ButtonColorRule& rule)
{
    const std::string_view locale = GetEditorLocale();
    ImGui::SetNextItemWidth(VipComboWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::BeginCombo("##RuleVip", GetRuleVipLabel(list, rule, locale).c_str()))
    {
        return false;
    }
    bool picked = false;
    int option = 0;
    const auto offer = [&](RuleVip vipMatch, const std::string& vip, const std::string& label)
    {
        ButtonColorRule candidate = rule;
        candidate.vipMatch = vipMatch;
        candidate.vip = vip;
        const bool current = candidate.FitsAsMuchAs(rule);
        ImGui::PushID(option++);
        if (SelectableText("##RuleVipOption", label, current) && !current)
        {
            rule = std::move(candidate);
            picked = true;
        }
        ImGui::PopID();
    };
    offer(RuleVip::Any, {}, I18N::Editor::Any);
    offer(RuleVip::AnyVipType, {}, I18N::Editor::AnyVIPType);
    offer(RuleVip::NoVipType, {}, I18N::Editor::None);
    for (const VipType& vipType : list.vipTypes)
    {
        offer(RuleVip::VipType, vipType.id, GetVipTypeLabel(&vipType, locale));
    }
    ImGui::EndCombo();
    return picked;
}

bool PickRulePvp(ButtonColorRule& rule)
{
    ImGui::SetNextItemWidth(PvpComboWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::BeginCombo("##RulePvp", GetRulePvpLabel(rule.pvp)))
    {
        return false;
    }
    bool picked = false;
    int option = 0;
    for (const std::optional<bool> pvp : {std::optional<bool>{}, std::optional<bool>{true}, std::optional<bool>{false}})
    {
        ImGui::PushID(option++);
        if (ImGui::Selectable(GetRulePvpLabel(pvp), pvp == rule.pvp) && pvp != rule.pvp)
        {
            rule.pvp = pvp;
            picked = true;
        }
        ImGui::PopID();
    }
    ImGui::EndCombo();
    return picked;
}

bool PickRuleColor(const ServerListData& list, ButtonColorRule& rule)
{
    const std::string_view theme = GetGameTheme();
    return PickColor("##RuleColor", list, rule.color, VipComboWidth, [&list, theme](std::string_view color)
                     { return GetButtonColorSwatch(Data::ServerList::ResolveButtonColor(list, color, theme), theme); });
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
        if (SelectableText("##VipOption", GetVipTypeLabel(&vipType, locale), &vipType == current) &&
            &vipType != current)
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
    // In the color the login screen draws it in.
    const std::string_view theme = GetGameTheme();
    ImGui::TextColored(GetButtonColorSwatch(Data::ServerList::GetButtonColor(list, settings, theme), theme), "%s",
                       name.c_str());

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
        const ImGuiTabItemFlags namesFlags =
            m_selectChannelNames ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
        m_selectChannelNames = false;
        if (ImGui::BeginTabItem(namesTab.c_str(), nullptr, namesFlags))
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
    // A read that left something out (ReadServerListJson) keeps Save off: it would remove that from the file.
    m_edits.Reset(std::move(loaded.list), loaded.complete && !Data::Items::HasFatal(loaded.issues));
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
    m_refusedRule = -1;
    m_themes = UI::RmlBridge::DiscoverAvailableThemes();
    // A theme's tokens.ini may have changed since; its shades are read again.
    ForgetThemeTokens();
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
    // A save after a read that left something out would remove it from the file.
    ImGui::BeginDisabled(!m_edits.IsComplete());
    if (ImGui::Button(I18N::Editor::Save))
    {
        Save();
    }
    ImGui::EndDisabled();
    if (!m_edits.IsComplete() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::SaveOffAfterAReadThatLeftSomethingOut);
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
    RenderOfflinePreview();
}

void CMuServerListUI::RenderOfflinePreview()
{
    bool show = g_ServerListManager->IsShowingOfflineServers();
    int percent = g_ServerListManager->GetOfflineServerPercent();
    bool changed = ImGui::Checkbox(I18N::Editor::ShowOfflineServersInTheGame, &show);
    bool finished = changed;
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::OfflineServersNote);
    }
    if (show)
    {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(OfflineLoadWidth * g_MuEditorCore.GetUIScale());
        changed |= ImGui::SliderInt(I18N::Editor::TheirLoad, &percent, 0, CServerListManager::MaxOfflineServerPercent,
                                    "%d %%");
        finished |= ImGui::IsItemDeactivatedAfterEdit();
    }
    if (changed)
    {
        g_ServerListManager->SetOfflinePreview(show, percent);
        ShowInGame();
    }
    // The setting is kept when a change is done, not at each step of the slider.
    if (finished)
    {
        g_MuEditorConfig.SetOfflineServers(show, percent);
        g_MuEditorConfig.Save();
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
    m_refusedRule = -1;
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
        RenderError(I18N::Editor::SaveOffAfterAReadThatLeftSomethingOut);
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
        if (SelectableText("##Group", label, index == m_selected, MuEditor::KeyboardSelection::RowFlags))
        {
            Select(index);
        }
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
        ImGui::SetNextItemWidth(WideIdFieldWidth * scale);
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
        changed |= RenderButtonColorRules();
        ImGui::Separator();
        changed |= MuEditor::ServerList::LoadUI::RenderLoadSettings(m_edits.EditList(), [this] { CommitPending(); });
        ImGui::Separator();
        RenderEditColorsButton();
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
        const std::string label = GetVipTypeLabel(&vipType, locale) + " (" + vipType.id + ")";
        if (SelectableText("##VipType", label, index == m_selectedVipType, MuEditor::KeyboardSelection::RowFlags))
        {
            SelectVipType(index);
        }
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
        // A type no channel and no rule names goes without asking.
        if (m_edits.FindChannelsWithVipType(vipType.id).empty() && m_edits.CountRulesForVipType(vipType.id) == 0)
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
        if (!channels.empty())
        {
            ImGui::TextUnformatted(
                I18N::Format(I18N::Editor::VIPTypeUsedNote, {std::to_string(channels.size())}).c_str());
            std::string list;
            for (const std::string& channel : channels)
            {
                list += (list.empty() ? "" : ", ") + channel;
            }
            ImGui::PushTextWrapPos(PopupWrapWidth * g_MuEditorCore.GetUIScale());
            ImGui::TextUnformatted(list.c_str());
            ImGui::PopTextWrapPos();
        }
        if (const size_t rules = m_edits.CountRulesForVipType(vipType.id); rules > 0)
        {
            ImGui::TextUnformatted(I18N::Format(I18N::Editor::VIPTypeRulesNote, {std::to_string(rules)}).c_str());
        }
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
    ImGui::SetNextItemWidth(WideIdFieldWidth * scale);
    if (MuEditor::Items::InputString(I18N::Editor::VIPTypeID, m_vipTypeIdDraft))
    {
        const std::string before = vipTypes[index].id;
        changed = m_edits.ChangeVipTypeId(index, m_vipTypeIdDraft) && vipTypes[index].id != before;
    }
    m_vipTypeIdActive = ImGui::IsItemActive();
    CommitWhenFieldLeft();
    if (m_vipTypeIdDraft != vipTypes[index].id)
    {
        RenderError(Data::ServerList::IsListId(m_vipTypeIdDraft)
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
    Data::ServerList::LoadSettings& load = m_edits.EditList().load;
    changed |= RenderLocalizedField(I18N::Editor::FullText, load.full.text, Data::ServerList::MaxLoadTextLength);
    changed |=
        RenderLocalizedField(I18N::Editor::OverloadedText, load.overloaded.text, Data::ServerList::MaxLoadTextLength);
    RenderNote(I18N::Editor::TextsNote);
    return changed;
}

bool CMuServerListUI::RenderButtonColorRules()
{
    ImGui::TextUnformatted(I18N::Editor::ButtonColors);
    RenderNote(I18N::Editor::ButtonColorsNote);
    bool changed = false;
    std::vector<ButtonColorRule>& rules = m_edits.EditList().buttonColors;
    std::optional<size_t> remove;
    constexpr ImGuiTableFlags TableFlags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
    if (!rules.empty() && ImGui::BeginTable("##ButtonColorRules", 4, TableFlags))
    {
        ImGui::TableSetupColumn(I18N::Editor::VIP);
        ImGui::TableSetupColumn(I18N::Editor::PvP);
        ImGui::TableSetupColumn(I18N::Editor::Color);
        ImGui::TableSetupColumn("##Remove");
        ImGui::TableHeadersRow();
        for (size_t index = 0; index < rules.size(); ++index)
        {
            ImGui::PushID(static_cast<int>(index));
            ImGui::TableNextRow();
            ButtonColorRule edited = rules[index];
            ImGui::TableNextColumn();
            PickRuleVip(m_edits.GetList(), edited);
            ImGui::TableNextColumn();
            PickRulePvp(edited);
            ImGui::TableNextColumn();
            PickRuleColor(m_edits.GetList(), edited);
            ImGui::TableNextColumn();
            if (ImGui::SmallButton(I18N::Editor::Remove))
            {
                remove = index;
            }
            if (edited != rules[index])
            {
                // A rule that would fit the same channels as another is refused.
                if (!edited.FitsAsMuchAs(rules[index]) && m_edits.HasRuleLike(edited, index))
                {
                    m_refusedRule = static_cast<int>(index);
                }
                else
                {
                    CommitPending();
                    rules[index] = std::move(edited);
                    m_refusedRule = -1;
                    changed = true;
                }
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (m_refusedRule >= 0)
    {
        RenderError(I18N::Editor::AnotherRuleFitsTheSameChannels);
    }
    if (remove)
    {
        CommitPending();
        m_edits.RemoveButtonColorRule(*remove);
        m_refusedRule = -1;
        changed = true;
    }
    // Asked each frame: a VIP type added or removed frees or takes settings.
    const bool anyFree = m_edits.FindFreeRule().has_value();
    ImGui::BeginDisabled(!anyFree);
    if (ImGui::Button(I18N::Editor::AddRule))
    {
        CommitPending();
        changed = m_edits.AddButtonColorRule().has_value();
        m_refusedRule = -1;
    }
    ImGui::EndDisabled();
    if (!anyFree)
    {
        RenderNote(I18N::Editor::EveryVIPTypeAndPvPSettingHasARule);
    }
    return changed;
}

void CMuServerListUI::RenderEditColorsButton()
{
    ImGui::TextUnformatted(I18N::Editor::Colors);
    RenderNote(I18N::Editor::ColorsNote);
    if (ImGui::Button(I18N::Editor::EditColors))
    {
        CommitPending();
        g_MuColorsUI.Show(MuEditor::Colors::ColorsRoot::ServerList);
    }
}

void CMuServerListUI::EnsureLoaded()
{
    if (!m_loaded)
    {
        LoadFile();
    }
}

void CMuServerListUI::AfterColorEdit()
{
    m_edits.Commit();
    m_uncommitted = false;
    ShowInGame();
}

namespace
{
// A part of the load that names a color, and the two halves of its label.
struct LoadColorPart
{
    const std::string* color;
    const char* group;
    const char* part;
};

// The parts of the load that can name the color `id`. A bar part set to "default" takes its own token, not the
// button color "default", so the bar's parts don't count for it.
std::array<LoadColorPart, 5> GetLoadColorParts(const Data::ServerList::LoadSettings& load, std::string_view id)
{
    const bool barParts = id != Data::ServerList::GetThemeColorName(Data::ServerList::ThemeColor::Default);
    return {{
        {barParts ? &load.bar.base : nullptr, I18N::Editor::LoadBar, I18N::Editor::BaseColor},
        {barParts ? &load.bar.full : nullptr, I18N::Editor::LoadBar, I18N::Editor::FullColor},
        {barParts ? &load.bar.empty : nullptr, I18N::Editor::LoadBar, I18N::Editor::UnlitColor},
        {&load.full.color, I18N::Editor::Load, I18N::Editor::FullServer},
        {&load.overloaded.color, I18N::Editor::Load, I18N::Editor::OverloadedServer},
    }};
}

// The bar's parts and their labels, in the editor's language now.
std::array<std::pair<Data::ServerList::LoadBarPart, const char*>, 3> GetBarParts()
{
    return {{
        {Data::ServerList::LoadBarPart::Base, I18N::Editor::BaseColor},
        {Data::ServerList::LoadBarPart::Full, I18N::Editor::FullColor},
        {Data::ServerList::LoadBarPart::Empty, I18N::Editor::UnlitColor},
    }};
}

// Whether the bar's part is drawn in the theme's own token `token`: it is set to "default".
bool DrawsInThemeToken(const Data::ServerList::LoadBarColors& bar, Data::ServerList::LoadBarPart part,
                       std::string_view token)
{
    return token == Data::ServerList::GetLoadBarToken(part) &&
           Data::ServerList::GetLoadBarColor(bar, part) ==
               Data::ServerList::GetThemeColorName(Data::ServerList::ThemeColor::Default);
}
} // namespace

std::vector<std::string> CMuServerListUI::DescribeColorUsers(std::string_view id) const
{
    const ServerListData& list = m_edits.GetList();
    const std::string_view locale = GetEditorLocale();
    std::vector<std::string> users;
    for (const ButtonColorRule& rule : list.buttonColors)
    {
        if (rule.color == id)
        {
            users.push_back(std::string(I18N::Editor::ButtonColors) + ": " + I18N::Editor::VIP + " " +
                            GetRuleVipLabel(list, rule, locale) + ", " + I18N::Editor::PvP + " " +
                            GetRulePvpLabel(rule.pvp));
        }
    }
    for (const LoadColorPart& part : GetLoadColorParts(list.load, id))
    {
        if (part.color != nullptr && *part.color == id)
        {
            users.push_back(std::string(part.group) + ": " + part.part);
        }
    }
    return users;
}

int CMuServerListUI::CountColorUsers(std::string_view id) const
{
    const ServerListData& list = m_edits.GetList();
    int count = 0;
    for (const ButtonColorRule& rule : list.buttonColors)
    {
        count += rule.color == id ? 1 : 0;
    }
    for (const LoadColorPart& part : GetLoadColorParts(list.load, id))
    {
        count += part.color != nullptr && *part.color == id ? 1 : 0;
    }
    return count;
}

std::vector<std::string> CMuServerListUI::DescribeThemeBarPartUsers(std::string_view token) const
{
    const Data::ServerList::LoadBarColors& bar = m_edits.GetList().load.bar;
    std::vector<std::string> users;
    for (const auto& [part, label] : GetBarParts())
    {
        if (DrawsInThemeToken(bar, part, token))
        {
            users.push_back(std::string(I18N::Editor::LoadBar) + ": " + label);
        }
    }
    return users;
}

int CMuServerListUI::CountThemeBarPartUsers(std::string_view token) const
{
    const Data::ServerList::LoadBarColors& bar = m_edits.GetList().load.bar;
    int count = 0;
    for (const auto& [part, label] : GetBarParts())
    {
        count += DrawsInThemeToken(bar, part, token) ? 1 : 0;
    }
    return count;
}

void CMuServerListUI::ShowChannelNames()
{
    g_MuEditorCore.GetToolWindows().serverList = true;
    m_selectChannelNames = true;
    ImGui::SetWindowFocus(MuEditor::WindowIds::ServerList);
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
    ImGui::TableSetupColumn(I18N::Editor::Color);
    for (int value = 0; value < Data::ServerList::ChannelNamePlaceCount; ++value)
    {
        ImGui::TableSetupColumn(MuEditor::ServerList::StepLabel::GetPlaceLabel(static_cast<ChannelNamePlace>(value)));
    }
    ImGui::TableHeadersRow();

    const ServerListData& list = m_edits.GetList();
    const std::vector<ServerGroup>& groups = list.groups;
    const bool hasGroup = m_selected >= 0 && m_selected < static_cast<int>(groups.size());
    const std::string groupName = hasGroup ? groups[static_cast<size_t>(m_selected)].name : std::string("Valhalla");
    // A server that can be joined: {load} stays empty.
    Data::ServerList::ChannelNameValues values;
    values.name = groupName;
    values.channel = 1;
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
            // The button's color in each theme, named on hover.
            ImGui::TableNextColumn();
            ImGui::PushID(vipType != nullptr ? vipType->id.c_str() : "##None");
            ImGui::PushID(pvp ? 1 : 0);
            for (const std::string& theme : m_themes)
            {
                ImGui::PushID(theme.c_str());
                RenderSwatch(GetButtonColorSwatch(Data::ServerList::GetButtonColor(list, channel, theme), theme));
                ImGui::SetItemTooltip("%s", UI::RmlBridge::GetThemeDisplayName(theme).c_str());
                ImGui::PopID();
                ImGui::SameLine();
            }
            ImGui::PopID();
            ImGui::PopID();
            ImGui::NewLine();
            const std::string_view gameTheme = GetGameTheme();
            const ImVec4 buttonColor =
                GetButtonColorSwatch(Data::ServerList::GetButtonColor(list, channel, gameTheme), gameTheme);
            for (int value = 0; value < Data::ServerList::ChannelNamePlaceCount; ++value)
            {
                ImGui::TableNextColumn();
                const ChannelNamePlace place = static_cast<ChannelNamePlace>(value);
                const std::string name = Data::ServerList::BuildChannelName(list, place, channel, values, m_textLocale);
                // The button in its color, as the login screen draws it.
                if (place == ChannelNamePlace::Button)
                {
                    ImGui::TextColored(buttonColor, "%s", name.c_str());
                }
                else
                {
                    ImGui::TextUnformatted(name.c_str());
                }
            }
        }
    }
    ImGui::EndTable();
    const ChannelSettings firstChannel =
        hasGroup ? groups[static_cast<size_t>(m_selected)].channels.front() : ChannelSettings{};
    MuEditor::ServerList::LoadUI::RenderLoadPreview(list, firstChannel, groupName, m_textLocale, m_themes);
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

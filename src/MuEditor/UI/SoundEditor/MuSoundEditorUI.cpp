#include "stdafx.h"

#ifdef _EDITOR

#include "MuSoundEditorUI.h"

#include "SoundEditorEdits.h"
#include "SoundEventEditing.h"
#include "SoundFilePicker.h"
#include "SoundFolderTree.h"
#include "SoundSelection.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "../MuEditor/UI/SoundMixer/MuSoundMixerUI.h"
#include "Audio/SoundLoads.h"
#include "Audio/SoundMixer.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "Data/GameData/SoundData/SoundEventDatabase.h"
#include "Data/GameData/SoundData/SoundFileDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cfloat>
#include <functional>
#include <set>
#include <unordered_map>

MuSoundEditorUI g_MuSoundEditorUI;

namespace
{
using MuEditor::SoundEditor::RowProblem;
using MuEditor::SoundEditor::RowSource;
using MuEditor::SoundEditor::SoundRow;

constexpr int Columns = 10;
constexpr float TypeWidth = 130.0f;
const ImVec4 SelectedRowColor(0.26f, 0.42f, 0.66f, 0.55f);
constexpr float FilterWidth = 220.0f;
constexpr float NameWidth = 200.0f;
constexpr float FileWidth = 320.0f;
constexpr float ChannelsWidth = 46.0f;
constexpr float PickerWidth = 420.0f;
constexpr float PickerHeight = 300.0f;
const ImVec4 ProblemColor(0.94f, 0.58f, 0.58f, 1.0f);
const ImVec4 FolderColor(0.80f, 0.88f, 1.0f, 1.0f);
// What a row's drag carries: its key.
constexpr const char* SoundPayload = "MU_SOUND_KEY";
constexpr float FolderNameWidth = 220.0f;

void CollectKeys(const std::vector<Data::Sounds::SoundFolder>& folders, std::set<std::string>& keys)
{
    for (const Data::Sounds::SoundFolder& folder : folders)
    {
        keys.insert(folder.sounds.begin(), folder.sounds.end());
        CollectKeys(folder.folders, keys);
    }
}

// The shown rows in the folder and below.
size_t CountShown(const Data::Sounds::SoundFolder& folder,
                  const std::unordered_map<std::string, const SoundRow*>& shown)
{
    size_t count = 0;
    for (const std::string& key : folder.sounds)
    {
        count += shown.contains(key) ? 1 : 0;
    }
    for (const Data::Sounds::SoundFolder& child : folder.folders)
    {
        count += CountShown(child, shown);
    }
    return count;
}

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

std::string HeardText(int number)
{
    std::string text;
    for (const Audio::SoundKind kind : Audio::Mixer::KindsHeard(number))
    {
        text += (text.empty() ? "" : ", ") + std::string(MuEditor::SoundMixer::KindName(kind));
    }
    return text;
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
        if (ImGui::BeginTabBar("##SoundEditorTabs"))
        {
            if (ImGui::BeginTabItem(I18N::Editor::SoundEditorTabSounds))
            {
                ImGui::PushTextWrapPos(0.0f);
                ImGui::TextDisabled("%s", I18N::Editor::SoundEditorNote);
                ImGui::PopTextWrapPos();
                const std::vector<SoundRow> rows = SoundEditor::BuildRows();
                std::vector<const SoundRow*> shown;
                for (const SoundRow& row : rows)
                {
                    if (SoundEditor::Matches(row, m_filterText.data()) &&
                        (!m_onlyProblems || row.problem != RowProblem::None))
                    {
                        shown.push_back(&row);
                    }
                }
                RenderFolderBar(rows);
                RenderFilterBar(rows, shown.size());
                RenderTable(shown);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(I18N::Editor::SoundEditorTabEvents))
            {
                RenderEvents();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void MuSoundEditorUI::RenderEvents()
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::SoundEditorEventsNote);
    ImGui::PopTextWrapPos();
    // A copy: a change replaces the table while it draws.
    const std::vector<Data::Sounds::SoundEvent> events = g_SoundEventDatabase.GetAll();
    for (size_t index = 0; index < events.size(); ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        if (ImGui::CollapsingHeader(MuEditor::SoundEditor::EventTitle(events[index].event).c_str(),
                                    ImGuiTreeNodeFlags_DefaultOpen))
        {
            RenderEventRules(events, index);
        }
        ImGui::PopID();
    }
}

void MuSoundEditorUI::RenderEventRules(const std::vector<Data::Sounds::SoundEvent>& events, size_t eventIndex)
{
    using Data::Sounds::SoundEventRule;
    const Data::Sounds::SoundEvent& event = events[eventIndex];
    const std::string label =
        I18N::Format(I18N::Editor::SoundEventStep0, {MuEditor::SoundEditor::EventTitle(event.event)});
    const auto change = [&](size_t rule, const std::optional<SoundEventRule>& after, int move = 0)
    {
        std::vector<Data::Sounds::SoundEvent> all = events;
        std::vector<SoundEventRule>& rules = all[eventIndex].rules;
        if (!after)
        {
            if (!MuEditor::SoundEditor::RemoveRule(all[eventIndex], rule))
            {
                return;
            }
        }
        else if (rule >= rules.size())
        {
            rules.push_back(*after);
        }
        else
        {
            rules[rule] = *after;
            const auto to = static_cast<std::ptrdiff_t>(rule) + move;
            if (move != 0 && to >= 0 && to < static_cast<std::ptrdiff_t>(rules.size()))
            {
                std::swap(rules[rule], rules[static_cast<size_t>(to)]);
            }
        }
        g_SoundEdits.ChangeEvents(label, std::move(all));
    };
    constexpr ImGuiTableFlags Flags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
    // Only the fields the event's code fills in (Q-UI1): a click knows no world.
    const MuEditor::SoundEditor::EventFields fields = MuEditor::SoundEditor::FieldsOf(event);
    const int columns = 3 + static_cast<int>(fields.worlds) + static_cast<int>(fields.tiles) +
                        static_cast<int>(fields.safeZone) + static_cast<int>(fields.holds) +
                        static_cast<int>(fields.kinds);
    if (ImGui::BeginTable("##Rules", columns, Flags))
    {
        ImGui::TableSetupColumn("#");
        const std::pair<bool, const char*> headers[] = {{fields.worlds, I18N::Editor::SoundEventColumnWorlds},
                                                        {fields.tiles, I18N::Editor::SoundEventColumnTiles},
                                                        {fields.safeZone, I18N::Editor::SoundEventColumnSafeZone},
                                                        {fields.holds, I18N::Editor::SoundEventColumnHolds},
                                                        {fields.kinds, I18N::Editor::SoundEventColumnKinds}};
        for (const auto& [shown, header] : headers)
        {
            if (shown)
            {
                ImGui::TableSetupColumn(header);
            }
        }
        ImGui::TableSetupColumn(I18N::Editor::SoundEventColumnPlays, ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##Buttons");
        ImGui::TableHeadersRow();
        for (size_t index = 0; index < event.rules.size(); ++index)
        {
            const SoundEventRule& rule = event.rules[index];
            const auto changeRule = [&](const SoundEventRule& after) { change(index, after); };
            ImGui::PushID(static_cast<int>(index));
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%zu", index + 1);
            if (fields.worlds)
            {
                ImGui::TableNextColumn();
                if (const auto worlds = RenderNumbersField("##Worlds", I18N::Editor::SoundEventColumnWorlds,
                                                           rule.worlds, Data::Sounds::MaxEventWorld))
                {
                    SoundEventRule after = rule;
                    after.worlds = *worlds;
                    changeRule(after);
                }
                Tooltip(I18N::Editor::SoundEventWorldsTip);
            }
            if (fields.tiles)
            {
                ImGui::TableNextColumn();
                if (const auto tiles = RenderNumbersField("##Tiles", I18N::Editor::SoundEventColumnTiles, rule.tiles,
                                                          Data::Sounds::MaxEventTile))
                {
                    SoundEventRule after = rule;
                    after.tiles = *tiles;
                    changeRule(after);
                }
                Tooltip(I18N::Editor::SoundEventTilesTip);
            }
            if (fields.safeZone)
            {
                ImGui::TableNextColumn();
                RenderSafeZoneCell(rule, changeRule);
            }
            if (fields.holds)
            {
                ImGui::TableNextColumn();
                RenderHoldsCell(rule, changeRule);
            }
            if (fields.kinds)
            {
                ImGui::TableNextColumn();
                if (const auto text = RenderListField("##Kinds", MuEditor::SoundEditor::KindsText(rule.kinds)))
                {
                    // Text that does not read is not taken, and the status
                    // line says why.
                    const auto kinds = MuEditor::SoundEditor::ParseKinds(*text);
                    m_status = kinds ? std::string() : std::string(I18N::Editor::SoundEventKindsNotRead);
                    if (kinds)
                    {
                        SoundEventRule after = rule;
                        after.kinds = *kinds;
                        changeRule(after);
                    }
                }
                Tooltip(I18N::Editor::SoundEventKindsTip);
            }
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(MuEditor::SoundEditor::PlaysText(rule).c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("..."))
            {
                m_pickerText.fill('\0');
                ImGui::OpenPopup("##Plays");
            }
            Tooltip(I18N::Editor::SoundEventPlaysTip);
            if (ImGui::BeginPopup("##Plays"))
            {
                RenderPlaysPicker(rule, changeRule);
                ImGui::EndPopup();
            }
            ImGui::TableNextColumn();
            if (ImGui::SmallButton(I18N::Editor::SoundEditorMoveUp))
            {
                change(index, rule, -1);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton(I18N::Editor::SoundEditorMoveDown))
            {
                change(index, rule, 1);
            }
            ImGui::SameLine();
            ImGui::BeginDisabled(event.rules.size() == 1);
            if (ImGui::SmallButton(I18N::Editor::SoundEditorRemoveFolder))
            {
                change(index, std::nullopt);
            }
            ImGui::EndDisabled();
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (ImGui::Button(I18N::Editor::SoundEventAddRule) && !event.rules.empty())
    {
        // A copy of the last rule, to change.
        change(event.rules.size(), event.rules.back());
    }
}

void MuSoundEditorUI::RenderSafeZoneCell(const Data::Sounds::SoundEventRule& rule,
                                         const std::function<void(const Data::Sounds::SoundEventRule&)>& change)
{
    using Data::Sounds::SoundEventRule;
    const char* zone = !rule.safeZone   ? I18N::Editor::SoundEventAny
                       : *rule.safeZone ? I18N::Editor::SoundEventInSafeZone
                                        : I18N::Editor::SoundEventOutsideSafeZone;
    ImGui::SetNextItemWidth(TypeWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::BeginCombo("##SafeZone", zone))
    {
        const std::pair<const char*, std::optional<bool>> choices[] = {
            {I18N::Editor::SoundEventAny, std::nullopt},
            {I18N::Editor::SoundEventInSafeZone, true},
            {I18N::Editor::SoundEventOutsideSafeZone, false}};
        for (const auto& [text, value] : choices)
        {
            if (ImGui::Selectable(text, rule.safeZone == value))
            {
                SoundEventRule after = rule;
                after.safeZone = value;
                change(after);
            }
        }
        ImGui::EndCombo();
    }
}

void MuSoundEditorUI::RenderHoldsCell(const Data::Sounds::SoundEventRule& rule,
                                      const std::function<void(const Data::Sounds::SoundEventRule&)>& change)
{
    using Data::Sounds::SoundEventRule;
    const std::string holds =
        rule.holds ? std::string(Data::Sounds::HeldWeaponWord(*rule.holds)) : I18N::Editor::SoundEventAny;
    ImGui::SetNextItemWidth(TypeWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::BeginCombo("##Holds", holds.c_str()))
    {
        if (ImGui::Selectable(I18N::Editor::SoundEventAny, !rule.holds))
        {
            SoundEventRule after = rule;
            after.holds.reset();
            change(after);
        }
        for (const Data::Sounds::HeldWeapon weapon :
             {Data::Sounds::HeldWeapon::None, Data::Sounds::HeldWeapon::Bow, Data::Sounds::HeldWeapon::Crossbow,
              Data::Sounds::HeldWeapon::MagicCrossbow, Data::Sounds::HeldWeapon::SaberOrSpear,
              Data::Sounds::HeldWeapon::Other})
        {
            if (ImGui::Selectable(std::string(Data::Sounds::HeldWeaponWord(weapon)).c_str(), rule.holds == weapon))
            {
                SoundEventRule after = rule;
                after.holds = weapon;
                change(after);
            }
        }
        ImGui::EndCombo();
    }
}

std::optional<std::string> MuSoundEditorUI::RenderListField(const char* id, const std::string& shown)
{
    const std::string key = std::string(id) + "@" + std::to_string(ImGui::GetID(id));
    std::string text = m_numbersEditKey == key ? m_numbersEdit : shown;
    ImGui::SetNextItemWidth(TypeWidth * g_MuEditorCore.GetUIScale());
    MuEditor::Items::InputStringWithHint(id, I18N::Editor::SoundEventAny, text);
    if (ImGui::IsItemActive())
    {
        m_numbersEditKey = key;
        m_numbersEdit = text;
    }
    std::optional<std::string> edited;
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        edited = text;
        m_numbersEditKey.clear();
    }
    else if (!ImGui::IsItemActive() && m_numbersEditKey == key)
    {
        m_numbersEditKey.clear();
    }
    return edited;
}

std::optional<std::vector<int>> MuSoundEditorUI::RenderNumbersField(const char* id, const char* label,
                                                                    const std::vector<int>& numbers, int highest)
{
    const std::optional<std::string> text = RenderListField(id, MuEditor::SoundEditor::NumbersText(numbers));
    if (!text)
    {
        return std::nullopt;
    }
    std::optional<std::vector<int>> read = MuEditor::SoundEditor::ParseNumbers(*text, highest);
    m_status =
        read ? std::string() : I18N::Format(I18N::Editor::SoundEventNumbersNotRead01, {label, std::to_string(highest)});
    return read;
}

void MuSoundEditorUI::RenderPlaysPicker(const Data::Sounds::SoundEventRule& rule,
                                        const std::function<void(const Data::Sounds::SoundEventRule&)>& change)
{
    // The sounds it plays, each taken out with its button.
    for (size_t index = 0; index < rule.plays.size(); ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        if (ImGui::SmallButton("x"))
        {
            // A copy only for the click.
            Data::Sounds::SoundEventRule after = rule;
            if (MuEditor::SoundEditor::RemovePlay(after, index))
            {
                change(after);
            }
        }
        ImGui::SameLine();
        const std::string name =
            MuEditor::SoundEditor::NameOfNumber(index < rule.numbers.size() ? rule.numbers[index] : -1);
        ImGui::TextUnformatted((name.empty() ? rule.plays[index] : name + " (" + rule.plays[index] + ")").c_str());
        ImGui::PopID();
    }
    ImGui::Separator();
    ImGui::SetNextItemWidth(PickerWidth * g_MuEditorCore.GetUIScale());
    ImGui::InputTextWithHint("##Search", I18N::Editor::SoundEditorFilterHint, m_pickerText.data(), m_pickerText.size());
    if (ImGui::BeginChild("##Sounds",
                          ImVec2(PickerWidth * g_MuEditorCore.GetUIScale(), PickerHeight * g_MuEditorCore.GetUIScale()),
                          ImGuiChildFlags_Borders))
    {
        const bool filtered = m_pickerText[0] != '\0';
        for (const Data::Sounds::SoundFile& sound : g_SoundFileDatabase.GetAll())
        {
            if (filtered)
            {
                MuEditor::SoundEditor::SoundRow probe;
                probe.id = sound.id;
                probe.name = sound.name;
                probe.file = sound.file;
                if (!MuEditor::SoundEditor::Matches(probe, m_pickerText.data()))
                {
                    continue;
                }
            }
            const std::string shown = sound.name.empty() ? sound.id : sound.name + " (" + sound.id + ")";
            if (ImGui::Selectable(shown.c_str()))
            {
                // A copy only for the click.
                Data::Sounds::SoundEventRule after = rule;
                if (MuEditor::SoundEditor::AddPlay(after, sound.id))
                {
                    change(after);
                }
            }
        }
    }
    ImGui::EndChild();
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
        m_status = g_SoundEdits.WasLastRefused() ? I18N::Editor::SoundEditorStepRefused : "";
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!g_SoundEdits.CanRedo());
    if (ImGui::Button(I18N::Editor::Redo))
    {
        g_SoundEdits.Redo();
        m_status = g_SoundEdits.WasLastRefused() ? I18N::Editor::SoundEditorStepRefused : "";
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
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::SoundEditorReadFilesAgain))
    {
        MuEditor::SoundEditor::RefreshClientSoundFiles();
    }
    Tooltip(I18N::Editor::SoundEditorReadFilesAgainTip);
}

void MuSoundEditorUI::RenderTable(const std::vector<const SoundRow*>& rows)
{
    // Every column has a width of its own, so a column's border moves only that column; the
    // table scrolls sideways when they are wider than the window, the names stay.
    constexpr ImGuiTableFlags Flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                                      ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Sounds", Columns, Flags, ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
    {
        return;
    }
    const float scale = g_MuEditorCore.GetUIScale();
    // Play and the names stay when the table scrolls sideways (Q-SE7); the folders indent the
    // names, not the play arrows.
    ImGui::TableSetupScrollFreeze(2, 1);
    ImGui::TableSetupColumn("##Play",
                            ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize |
                                ImGuiTableColumnFlags_IndentDisable,
                            ImGui::GetFrameHeight());
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnName,
                            ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_IndentEnable, NameWidth * scale);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnId);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnFile, ImGuiTableColumnFlags_WidthFixed, FileWidth * scale);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnChannels);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumn3D);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnType);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnHeardAs);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnLoads);
    ImGui::TableSetupColumn(I18N::Editor::SoundEditorColumnLivesIn);
    ImGui::TableHeadersRow();
    std::unordered_map<std::string, const SoundRow*> shown;
    for (const SoundRow* row : rows)
    {
        shown.emplace(row->key, row);
    }
    // The selection changed since the table last showed it: its folders open
    // and the table scrolls to it once.
    m_follow = m_seenSelection != MuEditor::SoundSelection::GetVersion();
    m_openPath.clear();
    if (m_follow)
    {
        for (const SoundRow* row : rows)
        {
            if (row->number >= 0 && row->number == MuEditor::SoundSelection::Get())
            {
                m_openPath = Data::Sounds::FindSoundFolder(g_SoundEdits.Folders(), row->key)
                                 .value_or(MuEditor::SoundEditor::FolderPath{});
                break;
            }
        }
    }
    // A copy: a drop or a toolbar button changes the folders while they draw.
    const std::vector<Data::Sounds::SoundFolder> folders = g_SoundEdits.Folders();
    for (const Data::Sounds::SoundFolder& folder : folders)
    {
        RenderFolder(folder, {folder.name}, shown);
    }
    std::set<std::string> sorted;
    CollectKeys(folders, sorted);
    std::vector<const SoundRow*> unsorted;
    for (const SoundRow* row : rows)
    {
        if (!sorted.contains(row->key))
        {
            unsorted.push_back(row);
        }
    }
    RenderNotSorted(unsorted);
    ImGui::EndTable();
    m_seenSelection = MuEditor::SoundSelection::GetVersion();
}

void MuSoundEditorUI::RenderFolder(const Data::Sounds::SoundFolder& folder,
                                   const MuEditor::SoundEditor::FolderPath& path,
                                   const std::unordered_map<std::string, const SoundRow*>& shown)
{
    const size_t count = CountShown(folder, shown);
    const bool filtered = m_filterText[0] != '\0' || m_onlyProblems;
    if (filtered && count == 0)
    {
        return;
    }
    ImGui::PushID(folder.name.c_str());
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    const bool onPath = !m_openPath.empty() && m_openPath.size() >= path.size() &&
                        std::equal(path.begin(), path.end(), m_openPath.begin());
    if (filtered || onPath)
    {
        ImGui::SetNextItemOpen(true);
    }
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_OpenOnArrow;
    if (m_selectedFolder == path)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    ImGui::PushStyleColor(ImGuiCol_Text, FolderColor);
    const bool open = ImGui::TreeNodeEx("##Folder", flags, "%s (%zu)", folder.name.c_str(), count);
    ImGui::PopStyleColor();
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        m_selectedFolder = path;
    }
    DropSoundOn(path);
    if (open)
    {
        for (const Data::Sounds::SoundFolder& child : folder.folders)
        {
            MuEditor::SoundEditor::FolderPath childPath = path;
            childPath.push_back(child.name);
            RenderFolder(child, childPath, shown);
        }
        for (const std::string& key : folder.sounds)
        {
            if (const auto row = shown.find(key); row != shown.end())
            {
                RenderRow(*row->second);
            }
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void MuSoundEditorUI::RenderNotSorted(const std::vector<const SoundRow*>& rows)
{
    const bool filtered = m_filterText[0] != '\0' || m_onlyProblems;
    if (rows.empty() && filtered)
    {
        return;
    }
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    const bool selectedHere = m_follow && std::any_of(rows.begin(), rows.end(), [](const SoundRow* row)
                                                      { return row->number == MuEditor::SoundSelection::Get(); });
    if (filtered || selectedHere)
    {
        ImGui::SetNextItemOpen(true);
    }
    const bool open = ImGui::TreeNodeEx("##NotSorted", ImGuiTreeNodeFlags_SpanAllColumns, "%s (%zu)",
                                        I18N::Editor::SoundEditorNotSorted, rows.size());
    DropSoundOn({});
    if (open)
    {
        for (const SoundRow* row : rows)
        {
            RenderRow(*row);
        }
        ImGui::TreePop();
    }
}

void MuSoundEditorUI::DropSoundOn(const MuEditor::SoundEditor::FolderPath& path)
{
    if (!ImGui::BeginDragDropTarget())
    {
        return;
    }
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(SoundPayload))
    {
        const std::string key(static_cast<const char*>(payload->Data));
        std::vector<Data::Sounds::SoundFolder> after = g_SoundEdits.Folders();
        if (MuEditor::SoundEditor::MoveSound(after, key, path))
        {
            g_SoundEdits.ChangeFolders(I18N::Editor::SoundEditorStepFolders, std::move(after));
        }
    }
    ImGui::EndDragDropTarget();
}

void MuSoundEditorUI::RenderFolderBar(const std::vector<SoundRow>& rows)
{
    using MuEditor::SoundEditor::FolderPath;
    const auto change = [](std::vector<Data::Sounds::SoundFolder> after)
    { g_SoundEdits.ChangeFolders(I18N::Editor::SoundEditorStepFolders, std::move(after)); };
    // A file that was not read is left alone (ChangeFolders refuses).
    const std::vector<Data::Items::ItemDataIssue>& problems = g_SoundEdits.FolderProblems();
    if (!problems.empty())
    {
        const std::string text =
            I18N::Format(I18N::Editor::SoundEditorFoldersUnreadable0, {problems.front().ToString()});
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextColored(ProblemColor, "%s", text.c_str());
        ImGui::PopTextWrapPos();
    }
    ImGui::BeginDisabled(!problems.empty());
    if (ImGui::Button(I18N::Editor::SoundEditorNewFolder))
    {
        std::vector<Data::Sounds::SoundFolder> after = g_SoundEdits.Folders();
        const FolderPath added =
            MuEditor::SoundEditor::AddFolder(after, m_selectedFolder, I18N::Editor::SoundEditorNewFolderName);
        if (!added.empty())
        {
            change(std::move(after));
            m_selectedFolder = added;
        }
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_selectedFolder.empty());
    if (ImGui::Button(I18N::Editor::SoundEditorRenameFolder))
    {
        m_folderName = m_selectedFolder.back();
        ImGui::OpenPopup("##RenameFolder");
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::SoundEditorRemoveFolder))
    {
        std::vector<Data::Sounds::SoundFolder> after = g_SoundEdits.Folders();
        if (MuEditor::SoundEditor::RemoveFolder(after, m_selectedFolder))
        {
            change(std::move(after));
            m_selectedFolder.pop_back();
        }
    }
    Tooltip(I18N::Editor::SoundEditorRemoveFolderTip);
    ImGui::SameLine();
    for (const auto& [label, by] :
         {std::pair{I18N::Editor::SoundEditorMoveUp, -1}, std::pair{I18N::Editor::SoundEditorMoveDown, 1}})
    {
        if (ImGui::Button(label))
        {
            std::vector<Data::Sounds::SoundFolder> after = g_SoundEdits.Folders();
            if (MuEditor::SoundEditor::MoveFolder(after, m_selectedFolder, by))
            {
                change(std::move(after));
            }
        }
        ImGui::SameLine();
    }
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    ImGui::TextDisabled("%s", I18N::Editor::SoundEditorDragTip);
    // Entries of sounds that are gone stay until the user drops them: a sound
    // that comes back (another window's undo) finds its folder. Counted
    // again only when the rows or the folders change.
    const auto keysOf = [&rows]
    {
        std::set<std::string> keys;
        for (const SoundRow& row : rows)
        {
            keys.insert(row.key);
        }
        return keys;
    };
    const GoneCountKey counted{g_SoundFileDatabase.GetVersion(),
                               g_MonsterSoundSetDatabase.GetVersion(),
                               g_MapObjectSoundDatabase.GetVersion(),
                               g_SoundEdits.GetFoldersVersion(),
                               rows.size(),
                               Audio::SoundLoads::PlayedUnloaded().size()};
    if (m_goneCountKey != counted)
    {
        m_goneCountKey = counted;
        m_goneCount = MuEditor::SoundEditor::CountStaleSounds(g_SoundEdits.Folders(), keysOf());
    }
    if (m_goneCount > 0)
    {
        ImGui::TextDisabled("%s",
                            I18N::Format(I18N::Editor::SoundEditorGoneSounds0, {std::to_string(m_goneCount)}).c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::SoundEditorDropGoneSounds))
        {
            const std::set<std::string> keys = keysOf();
            std::vector<Data::Sounds::SoundFolder> after = g_SoundEdits.Folders();
            MuEditor::SoundEditor::DropStaleSounds(after, std::vector<std::string>(keys.begin(), keys.end()));
            change(std::move(after));
        }
        Tooltip(I18N::Editor::SoundEditorDropGoneSoundsTip);
    }
    if (ImGui::BeginPopup("##RenameFolder"))
    {
        if (ImGui::IsWindowAppearing())
        {
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::SetNextItemWidth(FolderNameWidth * g_MuEditorCore.GetUIScale());
        const bool entered =
            MuEditor::Items::InputString("##FolderName", m_folderName, ImGuiInputTextFlags_EnterReturnsTrue);
        if (entered || ImGui::Button(I18N::Editor::SoundEditorRenameFolder))
        {
            std::vector<Data::Sounds::SoundFolder> after = g_SoundEdits.Folders();
            if (MuEditor::SoundEditor::RenameFolder(after, m_selectedFolder, m_folderName))
            {
                change(std::move(after));
                m_selectedFolder.back() = m_folderName;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void MuSoundEditorUI::RenderRow(const SoundRow& row)
{
    using MuEditor::SoundEditor::RowValues;
    ImGui::PushID(row.key.c_str());
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    // Listen first: the file once at full level, past the Sound Mixer (Q-SE5, Q-SE7).
    ImGui::BeginDisabled(!MuEditor::SoundEditor::CanPlay(row));
    if (ImGui::ArrowButton("##Play", ImGuiDir_Right))
    {
        MuEditor::SoundEditor::PreviewFile(row.file, row.channels, row.spatial);
    }
    ImGui::EndDisabled();
    Tooltip(I18N::Editor::SoundEditorPlayTip);
    ImGui::TableNextColumn();
    RenderNameCell(row);
    if (row.number >= 0 && row.number == MuEditor::SoundSelection::Get())
    {
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(SelectedRowColor));
    }
    ImGui::TableNextColumn();
    // The id's cell selects the row, here and in the Sound Mixer (Q-SE3); the
    // fields over the row stay clickable.
    const std::string id = MuEditor::SoundEditor::IdText(row);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    const bool selected = row.number >= 0 && row.number == MuEditor::SoundSelection::Get();
    if (ImGui::Selectable((id + "##Select").c_str(), selected,
                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
    {
        MuEditor::SoundSelection::Set(row.number);
    }
    ImGui::PopStyleColor();
    if (selected && m_follow)
    {
        ImGui::SetScrollHereY(0.5f);
    }
    // Dragged onto a folder (or Not sorted), the sound moves there (Q-SE4).
    if (Editable(row) && ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload(SoundPayload, row.key.c_str(), row.key.size() + 1);
        ImGui::TextUnformatted(MuEditor::SoundEditor::ShownName(row).c_str());
        ImGui::EndDragDropSource();
    }
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
    RenderTypeCell(row);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(HeardText(row.number).c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(row.loads.c_str());
    ImGui::TableNextColumn();
    ImGui::TextDisabled("%s", row.livesIn.c_str());
    ImGui::PopID();
}

void MuSoundEditorUI::RenderTypeCell(const SoundRow& row)
{
    if (row.source != RowSource::Code)
    {
        return;
    }
    const char* shown = row.type ? MuEditor::SoundMixer::KindName(*row.type) : I18N::Editor::SoundEditorNoType;
    ImGui::SetNextItemWidth(TypeWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::BeginCombo("##Type", shown))
    {
        MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(row);
        if (ImGui::Selectable(I18N::Editor::SoundEditorNoType, !row.type))
        {
            values.type.reset();
            g_SoundEdits.Change(row, values);
        }
        for (int kind = 0; kind < Audio::SoundKindCount; ++kind)
        {
            const auto type = static_cast<Audio::SoundKind>(kind);
            if (ImGui::Selectable(MuEditor::SoundMixer::KindName(type), row.type == type))
            {
                values.type = type;
                g_SoundEdits.Change(row, values);
            }
        }
        ImGui::EndCombo();
    }
    Tooltip(I18N::Editor::SoundEditorTypeTip);
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

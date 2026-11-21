#include "stdafx.h"

#ifdef _EDITOR

#include "MuWarpListUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/IssueLevels.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Common/SavedDataFiles.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Core/Text/AsciiText.h"
#include "Core/Text/Utf8.h"
#include "Data/DataHandler/MapData/WarpStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/MapData/WarpDatabase.h"
#include "Data/GameData/MapData/WarpMessages.h"
#include "Data/GameData/MapData/WarpsJson.h"
#include "I18N/All.h"
#include "Network/MoveCommandData.h"
#include "imgui.h"

#include <algorithm>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>

using Data::LocalizedString;
using Data::Maps::ClassLevelDiscount;
using Data::Maps::Warp;
using Data::Maps::WarpItemMatch;
using Data::Maps::WarpList;

namespace
{
// Pixel widths before the editor UI scale.
constexpr float WarpListWidth = 220.0f;
constexpr float TextFieldWidth = 240.0f;
constexpr float NumberFieldWidth = 110.0f;
constexpr float SmallNumberFieldWidth = 60.0f;
constexpr float ComboWidth = 160.0f;
constexpr ImVec4 ErrorColor{1.0f, 0.4f, 0.4f, 1.0f};
// The items a tag's tooltip lists before "And {0} more".
constexpr size_t MaxTooltipItems = 40;
constexpr int MaxWholeNumber = std::numeric_limits<int>::max();

std::string_view GetEditorLocale()
{
    const char* locale = I18N::GetCurrentLocale();
    return locale != nullptr ? std::string_view(locale) : LocalizedString::NeutralLocale;
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

void RenderSpaceWarning(const std::string& name)
{
    if (name.find(' ') != std::string::npos)
    {
        MuEditor::IssueLevels::RenderLine(Data::Items::ItemDataIssueSeverity::Warning, I18N::Editor::HasASpaceNote);
    }
}

// "23 Icarus", as the list shows a warp.
std::string GetRowLabel(const Warp& warp, std::string_view locale)
{
    return std::to_string(warp.index) + " " + warp.names.Get(locale);
}

bool MatchesSearch(const Warp& warp, std::string_view locale, const std::string& search)
{
    if (search.empty())
    {
        return true;
    }
    const std::string needle = Core::Text::ToLowerAscii(search);
    return Core::Text::ToLowerAscii(GetRowLabel(warp, locale)).find(needle) != std::string::npos ||
           Core::Text::ToLowerAscii(warp.names.GetNeutral()).find(needle) != std::string::npos ||
           Core::Text::ToLowerAscii(warp.command).find(needle) != std::string::npos;
}

// The items of the tag `tag` without those of `exceptTag` (none when empty), as the rules match them; from the
// item list's own lists of each tag.
std::vector<const Data::Items::ItemDefinition*> FindTagItems(const std::string& tag, const std::string& exceptTag)
{
    std::vector<const Data::Items::ItemDefinition*> items;
    Data::Items::ItemTag value{};
    if (!Data::Items::FindEnumValue(tag, value))
    {
        return items;
    }
    Data::Items::ItemTag except{};
    const bool hasExcept = !exceptTag.empty() && Data::Items::FindEnumValue(exceptTag, except);
    for (const int type : g_ItemDatabase.GetItemTypesWithTag(value))
    {
        const Data::Items::ItemDefinition* item = g_ItemDatabase.Find(type);
        if (item != nullptr && !(hasExcept && g_ItemDatabase.HasTag(type, except)))
        {
            items.push_back(item);
        }
    }
    return items;
}

// "(N items)" after a tag, the items listed on hover.
void RenderTagItemCount(const std::string& tag, const std::string& exceptTag)
{
    const std::vector<const Data::Items::ItemDefinition*> items = FindTagItems(tag, exceptTag);
    ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::TagItems0, {std::to_string(items.size())}).c_str());
    if (items.empty() || !ImGui::IsItemHovered() || !ImGui::BeginTooltip())
    {
        return;
    }
    for (size_t index = 0; index < items.size() && index < MaxTooltipItems; ++index)
    {
        const Data::Items::ItemDefinition& item = *items[index];
        ImGui::Text("%s (%d, %d)", Core::Text::ToUtf8(item.name.c_str()).c_str(), item.group, item.number);
    }
    if (items.size() > MaxTooltipItems)
    {
        ImGui::TextDisabled(
            "%s", I18N::Format(I18N::Editor::And0More, {std::to_string(items.size() - MaxTooltipItems)}).c_str());
    }
    ImGui::EndTooltip();
}

// A combo of the item tags; `allowNone` offers None (empty). True when another one was picked.
bool PickTag(const char* id, std::string& tag, bool allowNone)
{
    ImGui::SetNextItemWidth(ComboWidth * g_MuEditorCore.GetUIScale());
    const char* preview = tag.empty() ? I18N::Editor::None : tag.c_str();
    if (!ImGui::BeginCombo(id, preview))
    {
        return false;
    }
    bool picked = false;
    if (allowNone && ImGui::Selectable(I18N::Editor::None, tag.empty()) && !tag.empty())
    {
        tag.clear();
        picked = true;
    }
    for (const Data::Items::EnumName<Data::Items::ItemTag>& name : Data::Items::GetEnumNames(Data::Items::ItemTag{}))
    {
        const bool current = tag == name.name;
        if (ImGui::Selectable(name.name, current) && !current)
        {
            tag = name.name;
            picked = true;
        }
    }
    ImGui::EndCombo();
    return picked;
}

const char* GetKindLabel(WarpItemMatch::Kind kind)
{
    switch (kind)
    {
    case WarpItemMatch::Kind::Tag:
        return I18N::Editor::ItemTag;
    case WarpItemMatch::Kind::Wings:
        return I18N::Editor::AnyWings;
    case WarpItemMatch::Kind::Item:
    default:
        return I18N::Editor::Item;
    }
}

// A combo of the three kinds of a rule entry; true when another one was picked (the entry then has the new kind with
// the first tag for a tag).
bool PickKind(WarpItemMatch& match)
{
    ImGui::SetNextItemWidth(ComboWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::BeginCombo("##Kind", GetKindLabel(match.kind)))
    {
        return false;
    }
    bool picked = false;
    for (const WarpItemMatch::Kind kind :
         {WarpItemMatch::Kind::Item, WarpItemMatch::Kind::Tag, WarpItemMatch::Kind::Wings})
    {
        if (ImGui::Selectable(GetKindLabel(kind), kind == match.kind) && kind != match.kind)
        {
            WarpItemMatch changed;
            changed.kind = kind;
            if (kind == WarpItemMatch::Kind::Tag)
            {
                changed.tag = Data::Items::GetEnumNames(Data::Items::ItemTag{}).front().name;
            }
            match = std::move(changed);
            picked = true;
        }
    }
    ImGui::EndCombo();
    return picked;
}

std::string GetClassName(int playerClass)
{
    const std::span<const char* const> keys = Data::Items::GetClassKeys();
    return playerClass >= 0 && playerClass < static_cast<int>(keys.size()) ? keys[static_cast<size_t>(playerClass)]
                                                                           : std::to_string(playerClass);
}

bool HasDiscountFor(const WarpList& list, int playerClass)
{
    return std::any_of(list.classLevelDiscounts.begin(), list.classLevelDiscounts.end(),
                       [playerClass](const ClassLevelDiscount& discount)
                       { return discount.playerClass == playerClass; });
}
} // namespace

CMuWarpListUI& CMuWarpListUI::GetInstance()
{
    static CMuWarpListUI instance;
    return instance;
}

void CMuWarpListUI::Render(bool* open)
{
    if (!m_loaded)
    {
        LoadFile();
    }
    m_namesGrid.BeginFrame();
    if (!BeginWindow(open))
    {
        m_namesGrid.CommitEditNotDrawn(m_nameRows);
        return;
    }
    bool changed = RenderToolbar();
    RenderStatus();
    ImGui::Separator();
    if (ImGui::BeginTabBar("##WarpListTabs"))
    {
        // "###": the tab keeps its ID when the editor's language changes.
        const std::string warpsTab = std::string(I18N::Editor::Warps) + "###WarpsTab";
        const ImGuiTabItemFlags warpsFlags = m_showWarpsTab ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
        m_showWarpsTab = false;
        if (ImGui::BeginTabItem(warpsTab.c_str(), nullptr, warpsFlags))
        {
            changed |= RenderWarpsTab();
            ImGui::EndTabItem();
        }
        const std::string translationsTab = std::string(I18N::Editor::Translations) + "###TranslationsTab";
        if (ImGui::BeginTabItem(translationsTab.c_str()))
        {
            m_namesGrid.UpdateCountsAndLabels(m_nameRows);
            m_namesGrid.RenderToolbar(m_nameRows);
            ImGui::Separator();
            m_namesGrid.RenderTable(m_nameRows, "##WarpNames");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
    m_namesGrid.CommitEditNotDrawn(m_nameRows);
    AfterEdits(changed);
}

bool CMuWarpListUI::RenderWarpsTab()
{
    bool changed = false;
    const float scale = g_MuEditorCore.GetUIScale();
    const float panesHeight = std::max(ImGui::GetContentRegionAvail().y - m_bottomHeight, ImGui::GetFrameHeight());
    if (ImGui::BeginChild("##Warps", ImVec2(WarpListWidth * scale, panesHeight),
                          ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
    {
        changed |= RenderWarpList();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    if (ImGui::BeginChild("##Warp", ImVec2(0.0f, panesHeight)))
    {
        changed |= RenderSelectedWarp();
    }
    ImGui::EndChild();

    const float bottomStart = ImGui::GetCursorPosY();
    changed |= RenderClassLevelDiscount();
    m_bottomHeight = ImGui::GetCursorPosY() - bottomStart;
    return changed;
}

bool CMuWarpListUI::BeginWindow(bool* open)
{
    char title[MuEditor::WindowIds::TitleCapacity];
    MuEditor::Layout::PrepareToolWindow();
    if (!ImGui::Begin(MuEditor::WindowIds::Title(title, I18N::Editor::WarpList, MuEditor::WindowIds::WarpList), open,
                      ImGuiWindowFlags_NoCollapse |
                          MuEditor::Layout::ToolWindowBarFlags(MuEditor::WindowIds::WarpList)))
    {
        ImGui::End();
        return false;
    }
    // Before the bar: in this window's own bar it shows the Warp List's undo then.
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

bool CMuWarpListUI::RenderToolbar()
{
    // A save of a list that is not the file's would replace what the file has.
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
        ImGui::SetTooltip("%s", I18N::Format(I18N::Editor::Reads0AgainUnsavedChangesAreLost,
                                             {Data::Maps::GetWarpsFile().generic_string()})
                                    .c_str());
    }

    bool changed = false;
    const int count = static_cast<int>(m_edits.GetList().warps.size());
    const bool hasSelection = m_selected >= 0 && m_selected < count;
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::AddWarp))
    {
        CommitPending();
        const size_t position = hasSelection ? static_cast<size_t>(m_selected) + 1 : static_cast<size_t>(count);
        Select(static_cast<int>(m_edits.AddWarp(position)));
        changed = true;
    }
    ImGui::BeginDisabled(!hasSelection);
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Copy))
    {
        CommitPending();
        Select(static_cast<int>(m_edits.CopyWarp(static_cast<size_t>(m_selected))));
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Remove))
    {
        CommitPending();
        m_edits.RemoveWarp(static_cast<size_t>(m_selected));
        Select(std::min(m_selected, static_cast<int>(m_edits.GetList().warps.size()) - 1));
        changed = true;
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(!hasSelection || m_selected == 0);
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::MoveUp))
    {
        CommitPending();
        if (m_edits.MoveWarp(static_cast<size_t>(m_selected), -1))
        {
            Select(m_selected - 1);
            changed = true;
        }
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(!hasSelection || m_selected + 1 >= count);
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::MoveDown))
    {
        CommitPending();
        if (m_edits.MoveWarp(static_cast<size_t>(m_selected), 1))
        {
            Select(m_selected + 1);
            changed = true;
        }
    }
    ImGui::EndDisabled();
    if (m_edits.HasUnsavedChanges())
    {
        ImGui::SameLine();
        ImGui::TextUnformatted(I18N::Editor::UnsavedChanges);
    }
    return changed;
}

void CMuWarpListUI::RenderStatus()
{
    const std::string file = Data::Maps::GetWarpsFile().generic_string();
    RenderNote(I18N::Format(I18N::Editor::WarpListNote, {file}).c_str());
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
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::ProblemsIn0, {file}).c_str());
    for (const Data::Items::ItemDataIssue& issue : m_issues)
    {
        const std::string line = issue.field + ": " + issue.message;
        MuEditor::IssueLevels::RenderLine(issue.severity, line.c_str());
    }
}

bool CMuWarpListUI::RenderWarpList()
{
    ImGui::SetNextItemWidth(-1.0f);
    MuEditor::Items::InputStringWithHint("##Search", I18N::Editor::SearchWarps, m_search);
    const std::string_view locale = GetEditorLocale();
    const std::vector<Warp>& warps = m_edits.GetList().warps;
    if (ImGui::BeginChild("##Rows"))
    {
        for (int position = 0; position < static_cast<int>(warps.size()); ++position)
        {
            const Warp& warp = warps[static_cast<size_t>(position)];
            if (!MatchesSearch(warp, locale, m_search))
            {
                continue;
            }
            ImGui::PushID(position);
            const std::string label = GetRowLabel(warp, locale) + "###Warp";
            if (ImGui::Selectable(label.c_str(), position == m_selected, MuEditor::KeyboardSelection::RowFlags))
            {
                Select(position);
            }
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
    return false;
}

bool CMuWarpListUI::RenderSelectedWarp()
{
    std::vector<Warp>& warps = m_edits.EditList().warps;
    if (m_selected < 0 || m_selected >= static_cast<int>(warps.size()))
    {
        ImGui::TextDisabled("%s", I18N::Editor::PickAWarpInTheList);
        return false;
    }
    Warp& warp = warps[static_cast<size_t>(m_selected)];
    bool changed = RenderIndex(warp);
    changed |= RenderNames(warp);
    changed |= RenderValues(warp);
    ImGui::Separator();
    changed |= RenderRules(warp);
    return changed;
}

bool CMuWarpListUI::RenderIndex(Warp& warp)
{
    int index = warp.index;
    bool changed = false;
    ImGui::SetNextItemWidth(NumberFieldWidth * g_MuEditorCore.GetUIScale());
    // Typed only: a step button would stop at an index another warp has.
    if (ImGui::InputInt(I18N::Editor::Index, &index, 0, 0))
    {
        index = std::clamp(index, 0, Data::Maps::MaxWarpIndex);
        // An index another warp has stays with it; the field says so.
        m_takenIndex = index != warp.index && m_edits.IsIndexUsed(index) ? index : -1;
        changed = m_takenIndex < 0 && index != warp.index;
        if (changed)
        {
            warp.index = index;
        }
    }
    CommitWhenFieldLeft();
    if (m_takenIndex >= 0)
    {
        RenderError(I18N::Format(I18N::Editor::AnotherWarpHasTheIndex0, {std::to_string(m_takenIndex)}).c_str());
    }
    return changed;
}

bool CMuWarpListUI::RenderNames(Warp& warp)
{
    const float scale = g_MuEditorCore.GetUIScale();
    bool changed = false;
    // The other languages are in the Translations tab.
    std::string english = warp.names.GetNeutral();
    const std::string label = std::string(I18N::GetLanguageDisplayName("en")) + "##Name_en";
    ImGui::SetNextItemWidth(TextFieldWidth * scale);
    if (MuEditor::Items::InputString(label.c_str(), english))
    {
        Core::Text::CutToUtf16Length(english, Data::Maps::MaxWarpNameLength);
        warp.names.Set(LocalizedString::NeutralLocale, std::move(english));
        changed = true;
    }
    CommitWhenFieldLeft();
    RenderSpaceWarning(warp.names.GetNeutral());
    RenderNote(I18N::Editor::OtherLanguagesInTheTranslationsTab);
    if (warp.names.GetNeutral().empty())
    {
        RenderError(I18N::Editor::AWarpNeedsAnEnglishName);
    }
    ImGui::SetNextItemWidth(TextFieldWidth * scale);
    if (MuEditor::Items::InputString(I18N::Editor::MoveCommandName, warp.command))
    {
        Core::Text::CutToUtf16Length(warp.command, Data::Maps::MaxWarpNameLength);
        changed = true;
    }
    CommitWhenFieldLeft();
    if (warp.command.empty())
    {
        RenderError(I18N::Editor::AWarpNeedsAMoveName);
    }
    RenderSpaceWarning(warp.command);
    return changed;
}

bool CMuWarpListUI::RenderValues(Warp& warp)
{
    bool changed = InputNumber(I18N::Editor::Level, warp.level, 0, Data::Maps::MaxWarpLevel);
    CommitWhenFieldLeft();
    changed |= InputNumber(I18N::Editor::Zen, warp.zen, 0, MaxWholeNumber);
    CommitWhenFieldLeft();
    changed |= InputNumber(I18N::Editor::Gate, warp.gate, 0, Data::Maps::MaxWarpGate);
    CommitWhenFieldLeft();
    ImGui::SameLine();
    ImGui::TextDisabled("%s", I18N::Editor::GateNote);
    changed |= Checkbox(I18N::Editor::NoClassDiscount, warp.noClassDiscount);
    changed |= InputNumber(I18N::Editor::MasterLevel, warp.minMasterLevel, 0, Data::Maps::MaxWarpLevel);
    CommitWhenFieldLeft();
    ImGui::SameLine();
    ImGui::TextDisabled("%s", I18N::Editor::ZeroIsNone);
    changed |= InputNumber(I18N::Editor::Resets, warp.minResets, 0, MaxWholeNumber);
    CommitWhenFieldLeft();
    ImGui::SameLine();
    ImGui::TextDisabled("%s", I18N::Editor::ZeroIsNone);
    return changed;
}

bool CMuWarpListUI::RenderRules(Warp& warp)
{
    ImGui::TextUnformatted(I18N::Editor::WarpRules);
    ImGui::TextUnformatted(I18N::Editor::NeedsOneOf);
    RenderNote(I18N::Editor::NeedsOneOfNote);
    bool changed = RenderItemMatches("##NeedsOneOf", warp.needsOneOf, false);
    ImGui::TextUnformatted(I18N::Editor::NotWith);
    RenderNote(I18N::Editor::NotWithNote);
    changed |= RenderItemMatches("##NotWith", warp.notWith, true);
    changed |= Checkbox(I18N::Editor::PvPServersOnly, warp.pvpServersOnly);
    changed |= Checkbox(I18N::Editor::GensMembersOnly, warp.gensOnly);
    changed |= RenderBlockedMessage(warp);
    return changed;
}

bool CMuWarpListUI::RenderItemMatches(const char* id, std::vector<WarpItemMatch>& matches, bool withExcept)
{
    const float scale = g_MuEditorCore.GetUIScale();
    bool changed = false;
    int removed = -1;
    ImGui::PushID(id);
    for (int entry = 0; entry < static_cast<int>(matches.size()); ++entry)
    {
        WarpItemMatch& match = matches[static_cast<size_t>(entry)];
        ImGui::PushID(entry);
        WarpItemMatch picked = match;
        if (PickKind(picked))
        {
            CommitPending();
            match = std::move(picked);
            changed = true;
        }
        switch (match.kind)
        {
        case WarpItemMatch::Kind::Item:
        {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(SmallNumberFieldWidth * scale);
            int group = match.group;
            if (ImGui::InputInt("##Group", &group, 0, 0))
            {
                match.group = std::clamp(group, 0, MAX_ITEM_TYPE - 1);
                changed = true;
            }
            CommitWhenFieldLeft();
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", I18N::Editor::Group);
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(SmallNumberFieldWidth * scale);
            int number = match.number;
            if (ImGui::InputInt("##Number", &number, 0, 0))
            {
                match.number = std::clamp(number, 0, MAX_ITEM_INDEX - 1);
                changed = true;
            }
            CommitWhenFieldLeft();
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", I18N::Editor::Number);
            }
            ImGui::SameLine();
            if (const Data::Items::ItemDefinition* item = g_ItemDatabase.FindEntry(match.group, match.number))
            {
                ImGui::TextUnformatted(Core::Text::ToUtf8(item->name.c_str()).c_str());
            }
            else
            {
                ImGui::TextColored(ErrorColor, "%s", I18N::Editor::UnknownItem);
            }
            break;
        }
        case WarpItemMatch::Kind::Tag:
        {
            ImGui::SameLine();
            std::string tag = match.tag;
            if (PickTag("##Tag", tag, false))
            {
                CommitPending();
                match.tag = std::move(tag);
                changed = true;
            }
            if (withExcept)
            {
                ImGui::SameLine();
                ImGui::TextUnformatted(I18N::Editor::Except);
                ImGui::SameLine();
                std::string exceptTag = match.exceptTag;
                if (PickTag("##Except", exceptTag, true))
                {
                    CommitPending();
                    match.exceptTag = std::move(exceptTag);
                    changed = true;
                }
            }
            ImGui::SameLine();
            RenderTagItemCount(match.tag, match.exceptTag);
            break;
        }
        case WarpItemMatch::Kind::Wings:
            break;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::Remove))
        {
            removed = entry;
        }
        ImGui::PopID();
    }
    if (removed >= 0)
    {
        CommitPending();
        matches.erase(matches.begin() + removed);
        changed = true;
    }
    if (ImGui::SmallButton(I18N::Editor::Add))
    {
        CommitPending();
        WarpItemMatch match;
        match.kind = WarpItemMatch::Kind::Wings;
        matches.push_back(std::move(match));
        changed = true;
    }
    ImGui::PopID();
    return changed;
}

bool CMuWarpListUI::RenderBlockedMessage(Warp& warp)
{
    ImGui::SetNextItemWidth(TextFieldWidth * g_MuEditorCore.GetUIScale());
    const char* preview = warp.blockedMessage.empty() ? I18N::Editor::GeneralWarpMessage : warp.blockedMessage.c_str();
    bool changed = false;
    if (ImGui::BeginCombo(I18N::Editor::BlockedMessage, preview))
    {
        if (ImGui::Selectable(I18N::Editor::GeneralWarpMessage, warp.blockedMessage.empty()) &&
            !warp.blockedMessage.empty())
        {
            CommitPending();
            warp.blockedMessage.clear();
            changed = true;
        }
        for (const std::string_view key : Data::Maps::GetWarpMessageKeys())
        {
            // The general text is the empty key.
            if (key == Data::Maps::GeneralWarpMessageKey)
            {
                continue;
            }
            const std::string name(key);
            const bool current = warp.blockedMessage == key;
            if (ImGui::Selectable(name.c_str(), current) && !current)
            {
                CommitPending();
                warp.blockedMessage = name;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    RenderNote(I18N::Editor::BlockedMessageNote);
    const std::string text = Core::Text::ToUtf8(Data::Maps::GetWarpMessageText(warp.blockedMessage));
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(("\"" + text + "\"").c_str());
    ImGui::PopTextWrapPos();
    return changed;
}

bool CMuWarpListUI::RenderClassLevelDiscount()
{
    if (!ImGui::CollapsingHeader((std::string(I18N::Editor::ClassLevelDiscount) + "###ClassLevelDiscount").c_str()))
    {
        return false;
    }
    ImGui::PushID("##Discounts");
    RenderNote(I18N::Editor::ClassLevelDiscountNote);
    std::vector<ClassLevelDiscount>& discounts = m_edits.EditList().classLevelDiscounts;
    const std::span<const char* const> classes = Data::Items::GetClassKeys();
    bool changed = false;
    int removed = -1;
    for (int position = 0; position < static_cast<int>(discounts.size()); ++position)
    {
        ClassLevelDiscount& discount = discounts[static_cast<size_t>(position)];
        ImGui::PushID(position);
        ImGui::SetNextItemWidth(ComboWidth * g_MuEditorCore.GetUIScale());
        if (ImGui::BeginCombo(I18N::Editor::Class, GetClassName(discount.playerClass).c_str()))
        {
            for (int playerClass = 0; playerClass < static_cast<int>(classes.size()); ++playerClass)
            {
                const bool current = playerClass == discount.playerClass;
                // Each class has one discount.
                const bool taken = !current && HasDiscountFor(m_edits.GetList(), playerClass);
                if (ImGui::Selectable(classes[static_cast<size_t>(playerClass)], current,
                                      taken ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None) &&
                    !current)
                {
                    CommitPending();
                    discount.playerClass = playerClass;
                    changed = true;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        changed |= InputNumber(I18N::Editor::Numerator, discount.numerator, 0, Data::Maps::MaxDiscountPart);
        CommitWhenFieldLeft();
        ImGui::SameLine();
        changed |= InputNumber(I18N::Editor::Denominator, discount.denominator, 1, Data::Maps::MaxDiscountPart);
        CommitWhenFieldLeft();
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::Remove))
        {
            removed = position;
        }
        ImGui::PopID();
    }
    if (removed >= 0)
    {
        CommitPending();
        discounts.erase(discounts.begin() + removed);
        changed = true;
    }
    int freeClass = 0;
    while (freeClass < static_cast<int>(classes.size()) && HasDiscountFor(m_edits.GetList(), freeClass))
    {
        ++freeClass;
    }
    ImGui::BeginDisabled(freeClass >= static_cast<int>(classes.size()));
    if (ImGui::SmallButton(I18N::Editor::AddClass))
    {
        CommitPending();
        ClassLevelDiscount discount;
        discount.playerClass = freeClass;
        discounts.push_back(discount);
        changed = true;
    }
    ImGui::EndDisabled();
    ImGui::PopID();
    return changed;
}

bool CMuWarpListUI::InputNumber(const char* label, int& value, int low, int high)
{
    int typed = value;
    ImGui::SetNextItemWidth(NumberFieldWidth * g_MuEditorCore.GetUIScale());
    // Typed only: a step button would make a step per click.
    if (!ImGui::InputInt(label, &typed, 0, 0))
    {
        return false;
    }
    typed = std::clamp(typed, low, high);
    if (typed == value)
    {
        return false;
    }
    value = typed;
    return true;
}

bool CMuWarpListUI::Checkbox(const char* label, bool& value)
{
    bool checked = value;
    if (!ImGui::Checkbox(label, &checked))
    {
        return false;
    }
    CommitPending();
    value = checked;
    return true;
}

void CMuWarpListUI::AfterEdits(bool changed)
{
    if (changed)
    {
        m_uncommitted = true;
        ShowInGame();
    }
    // A text field changes the list with each key; its whole edit is one step,
    // taken when it is left (CommitWhenFieldLeft) or, should it go away unseen,
    // once nothing is active.
    if (m_uncommitted && !ImGui::IsAnyItemActive())
    {
        CommitPending();
    }
}

void CMuWarpListUI::CommitPending()
{
    m_edits.Commit();
    m_uncommitted = false;
}

void CMuWarpListUI::CommitWhenFieldLeft()
{
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        CommitPending();
    }
}

void CMuWarpListUI::LoadFile()
{
    m_issues.clear();
    // The file into the game's list; on fatal problems the game keeps its list, which the window shows without Save.
    const bool read = Data::Maps::RevertWarps(m_issues);
    if (read)
    {
        Data::Maps::ValidateWarpItemsInGame(g_WarpDatabase.GetList(), Data::Maps::GetWarpsFile().generic_string(),
                                            m_issues);
    }
    m_edits.Reset(g_WarpDatabase.GetList(), read);
    m_status.clear();
    m_statusIsError = false;
    m_loaded = true;
    m_uncommitted = false;
    m_takenIndex = -1;
    const int count = static_cast<int>(m_edits.GetList().warps.size());
    Select(count == 0 ? -1 : std::clamp(m_selected, 0, count - 1));
    ShowInGame();
}

void CMuWarpListUI::Save()
{
    if (!m_edits.IsComplete())
    {
        return;
    }
    CommitPending();
    ShowInGame();
    m_issues.clear();
    if (!Data::Maps::SaveWarps(m_issues))
    {
        m_status = I18N::Editor::NotSavedSeeTheProblemsBelow;
        m_statusIsError = true;
        return;
    }
    m_edits.MarkSaved();
    MuEditor::SavedDataFiles::NoteSaved();
    m_status = I18N::Format(I18N::Editor::SavedTo0, {Data::Maps::GetWarpsFile().generic_string()});
    m_statusIsError = false;
}

void CMuWarpListUI::ShowInGame()
{
    ++m_version;
    g_WarpDatabase.Build(m_edits.GetList());
    SEASON3B::CMoveCommandData::GetInstance()->SetDataFileWarps(m_edits.GetList(), GetEditorLocale());
}

void CMuWarpListUI::Select(int position)
{
    if (position != m_selected)
    {
        m_takenIndex = -1;
        ++m_selectionSerial;
    }
    m_selected = position;
}

int CMuWarpListUI::GetSelectedIndex() const
{
    const std::vector<Warp>& warps = m_edits.GetList().warps;
    return m_selected >= 0 && m_selected < static_cast<int>(warps.size()) ? warps[static_cast<size_t>(m_selected)].index
                                                                          : -1;
}

void CMuWarpListUI::SelectWarpIndex(int index)
{
    const std::vector<Warp>& warps = m_edits.GetList().warps;
    const auto warp =
        std::find_if(warps.begin(), warps.end(), [index](const Warp& entry) { return entry.index == index; });
    if (warp != warps.end())
    {
        Select(static_cast<int>(warp - warps.begin()));
    }
}

bool CMuWarpListUI::SetWarpName(int index, std::string_view locale, std::string name)
{
    std::vector<Warp>& warps = m_edits.EditList().warps;
    const auto warp =
        std::find_if(warps.begin(), warps.end(), [index](const Warp& entry) { return entry.index == index; });
    if (warp == warps.end() || warp->names.GetOwn(locale) == name)
    {
        return false;
    }
    // What was typed before is a step of its own.
    CommitPending();
    warp->names.Set(locale, std::move(name));
    m_uncommitted = true;
    ShowInGame();
    CommitPending();
    return true;
}

bool CMuWarpListUI::IsOpen() const
{
    return g_MuEditorCore.GetToolWindows().warpList;
}

bool CMuWarpListUI::CanUndo() const
{
    return m_edits.CanUndo() || m_uncommitted;
}

bool CMuWarpListUI::CanRedo() const
{
    return m_edits.CanRedo();
}

std::string CMuWarpListUI::GetUndoLabel() const
{
    return m_edits.GetUndoLabel();
}

std::string CMuWarpListUI::GetRedoLabel() const
{
    return m_edits.GetRedoLabel();
}

void CMuWarpListUI::Undo()
{
    m_edits.Undo();
    AfterHistoryStep();
}

void CMuWarpListUI::Redo()
{
    m_edits.Redo();
    AfterHistoryStep();
}

void CMuWarpListUI::AfterHistoryStep()
{
    m_uncommitted = false;
    m_takenIndex = -1;
    Select(std::min(m_selected, static_cast<int>(m_edits.GetList().warps.size()) - 1));
    ShowInGame();
}

void CMuWarpListUI::HandleUndoKeys()
{
    // Only while the keyboard is the editor's; a text field being edited takes
    // the keys first.
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

#endif // _EDITOR

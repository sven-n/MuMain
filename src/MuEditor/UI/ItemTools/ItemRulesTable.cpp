#include "stdafx.h"

#ifdef _EDITOR

#include "ItemRulesTable.h"

#include "ItemEdits.h"
#include "ItemRuleFlags.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <span>

namespace MuEditor::Items
{
namespace
{
// Pixel widths before the editor UI scale.
constexpr float NumberColumnWidth = 70.0f;
constexpr float NameColumnWidth = 220.0f;
constexpr float FlagColumnWidth = 90.0f;

constexpr int NumberColumn = 0;
constexpr int NameColumn = 1;
constexpr int FirstFlagColumn = 2;
// The number and the name stay in view while the flags scroll sideways.
constexpr int FrozenColumns = FirstFlagColumn;
constexpr int FrozenRows = 1;
constexpr int NoItem = -1;
constexpr size_t NumberTextSize = 16;

// The row of the item the tools show: amber, apart from the blue of the
// rows selected for the buttons.
constexpr ImVec4 MarkedRowColor(0.9f, 0.7f, 0.0f, 0.25f);

constexpr ImGuiTableFlags TableFlags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable |
                                       ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg |
                                       ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
constexpr ImGuiMultiSelectFlags SelectionFlags =
    ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;

ImGuiID ToSelectionId(int itemType)
{
    return static_cast<ImGuiID>(itemType);
}

// ImGui hands the rows over by their index in the list; the selection keeps
// the item type of the row.
ImGuiID ItemTypeOfRow(ImGuiSelectionBasicStorage* selection, int row)
{
    const auto* itemTypes = static_cast<const std::vector<int>*>(selection->UserData);
    return ToSelectionId((*itemTypes)[row]);
}

// ImGui keeps the row a Shift click starts from as an index into the rows.
// When other rows are listed (a search, a filter, an item added), the source
// moves to the row of the same item, or is dropped while that item is not
// listed: a Shift click then starts at the clicked row. Not by RangeSrcReset:
// ImGui reads that only while the table has the keyboard focus, and typing a
// search takes the focus away.
void FollowRangeSource(ImGuiID scope, const std::vector<int>& before, const std::vector<int>& rows)
{
    ImGuiMultiSelectState* state = ImGui::GetMultiSelectState(scope);
    if (state == nullptr || state->RangeSrcItem < 0)
    {
        return;
    }
    const auto row = static_cast<size_t>(state->RangeSrcItem);
    const auto moved = row < before.size() ? std::ranges::find(rows, before[row]) : rows.end();
    state->RangeSrcItem = moved != rows.end() ? static_cast<ImGuiSelectionUserData>(moved - rows.begin())
                                              : ImGuiSelectionUserData_Invalid;
}

// A guard: a range is cut to the rows there are, so the selection never reads
// past them.
void KeepRangesInRows(ImGuiMultiSelectIO* selectionIo, int rowCount)
{
    ImVector<ImGuiSelectionRequest>& requests = selectionIo->Requests;
    for (int index = requests.Size - 1; index >= 0; --index)
    {
        ImGuiSelectionRequest& request = requests[index];
        if (request.Type != ImGuiSelectionRequestType_SetRange)
        {
            continue;
        }
        request.RangeFirstItem = std::max<ImGuiSelectionUserData>(request.RangeFirstItem, 0);
        request.RangeLastItem = std::min<ImGuiSelectionUserData>(request.RangeLastItem, rowCount - 1);
        if (request.RangeFirstItem > request.RangeLastItem)
        {
            requests.erase(requests.begin() + index);
        }
    }
}

void SetupColumns(std::span<const ItemRuleFlag> flags)
{
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::TableSetupScrollFreeze(FrozenColumns, FrozenRows);
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide, NumberColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Metadata::Name, ImGuiTableColumnFlags_NoHide, NameColumnWidth * uiScale);
    for (const ItemRuleFlag& flag : flags)
    {
        ImGui::TableSetupColumn(*flag.label, ImGuiTableColumnFlags_None, FlagColumnWidth * uiScale);
    }
}

// The header row cell by cell, so a rule name that a narrow column cuts off
// shows whole when the mouse is over it.
void RenderHeaders(int columnCount)
{
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    for (int column = 0; column < columnCount; ++column)
    {
        if (!ImGui::TableSetColumnIndex(column))
        {
            continue;
        }
        const char* name = ImGui::TableGetColumnName(column);
        ImGui::PushID(column);
        ImGui::TableHeader(name);
        ImGui::PopID();
        if (column >= FirstFlagColumn && ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", name);
        }
    }
}

// A checkbox per rule flag; a click changes the flag of this item only.
void RenderFlagCells(const Data::Items::ItemDefinition& item)
{
    const std::span<const ItemRuleFlag> flags = GetItemRuleFlags();
    for (size_t index = 0; index < flags.size(); ++index)
    {
        if (!ImGui::TableSetColumnIndex(FirstFlagColumn + static_cast<int>(index)))
        {
            continue;
        }
        const ItemRuleFlag& flag = flags[index];
        bool allowed = item.*flag.value;
        ImGui::PushID(static_cast<int>(index));
        if (ImGui::Checkbox("##Allowed", &allowed))
        {
            Data::Items::ItemDefinition changed = item;
            changed.*flag.value = allowed;
            g_ItemEdits.SetItem(*flag.label, changed);
        }
        ImGui::PopID();
    }
}
} // namespace

int CItemRulesTable::Render(const std::vector<int>& itemTypes, int markedItemType, bool reveal)
{
    const std::span<const ItemRuleFlag> flags = GetItemRuleFlags();
    const int columnCount = FirstFlagColumn + static_cast<int>(flags.size());
    if (!ImGui::BeginTable("##ItemRules", columnCount, TableFlags))
    {
        return NoItem;
    }
    SetupColumns(flags);
    RenderHeaders(columnCount);
    const int clicked = RenderRows(itemTypes, markedItemType, reveal);
    ImGui::EndTable();
    return clicked;
}

bool CItemRulesTable::IsSelected(int itemType) const
{
    return m_selection.Contains(ToSelectionId(itemType));
}

void CItemRulesTable::KeepListedRows(const std::vector<int>& itemTypes)
{
    if (m_selection.Size == 0)
    {
        return;
    }
    ImGuiSelectionBasicStorage listed;
    for (const int itemType : itemTypes)
    {
        if (IsSelected(itemType))
        {
            listed.SetItemSelected(ToSelectionId(itemType), true);
        }
    }
    m_selection.Swap(listed);
}

int CItemRulesTable::RenderRows(const std::vector<int>& itemTypes, int markedItemType, bool reveal)
{
    const int rowCount = static_cast<int>(itemTypes.size());
    // ImGui's user data is not const; ItemTypeOfRow only reads the list.
    m_selection.UserData = const_cast<std::vector<int>*>(&itemTypes);
    m_selection.AdapterIndexToStorageId = &ItemTypeOfRow;
    // BeginMultiSelect's scope is the ID on top of the window's ID stack.
    if (itemTypes != m_rows)
    {
        FollowRangeSource(ImGui::GetCurrentWindow()->IDStack.back(), m_rows, itemTypes);
        m_rows = itemTypes;
    }
    ImGuiMultiSelectIO* selectionIo = ImGui::BeginMultiSelect(SelectionFlags, m_selection.Size, rowCount);
    KeepRangesInRows(selectionIo, rowCount);
    m_selection.ApplyRequests(selectionIo);

    ImGuiListClipper clipper;
    clipper.Begin(rowCount);
    // A Shift click selects from this row on, so it has to be drawn.
    if (selectionIo->RangeSrcItem >= 0 && selectionIo->RangeSrcItem < rowCount)
    {
        clipper.IncludeItemByIndex(static_cast<int>(selectionIo->RangeSrcItem));
    }
    const auto markedRow = std::ranges::find(itemTypes, markedItemType);
    const bool scrollToMarked = reveal && markedRow != itemTypes.end();
    if (scrollToMarked)
    {
        clipper.IncludeItemByIndex(static_cast<int>(markedRow - itemTypes.begin()));
    }

    int clicked = NoItem;
    // Checkboxes as high as the text keep every row one line high.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0.0f));
    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            const bool marked = itemTypes[row] == markedItemType;
            if (RenderRow(row, itemTypes[row], marked))
            {
                clicked = itemTypes[row];
            }
            if (marked && scrollToMarked)
            {
                ImGui::SetScrollHereY();
            }
        }
    }
    ImGui::PopStyleVar();
    selectionIo = ImGui::EndMultiSelect();
    KeepRangesInRows(selectionIo, rowCount);
    m_selection.ApplyRequests(selectionIo);
    return clicked;
}

bool CItemRulesTable::RenderRow(int row, int itemType, bool marked)
{
    ImGui::TableNextRow();
    const Data::Items::ItemDefinition* item = g_ItemDatabase.Find(itemType);
    if (item == nullptr)
    {
        return false;
    }
    if (marked)
    {
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(MarkedRowColor));
    }
    char number[NumberTextSize];
    std::snprintf(number, sizeof(number), "%d,%d", Data::Items::GetItemGroup(itemType),
                  Data::Items::GetItemNumber(itemType));

    ImGui::PushID(itemType);
    ImGui::TableSetColumnIndex(NumberColumn);
    ImGui::SetNextItemSelectionUserData(row);
    const bool clicked = ImGui::Selectable(number, IsSelected(itemType),
                                           ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
    if (ImGui::TableSetColumnIndex(NameColumn))
    {
        ImGui::TextUnformatted(Core::Text::ToUtf8(item->name.c_str()).c_str());
    }
    RenderFlagCells(*item);
    ImGui::PopID();
    return clicked;
}
} // namespace MuEditor::Items

#endif // _EDITOR

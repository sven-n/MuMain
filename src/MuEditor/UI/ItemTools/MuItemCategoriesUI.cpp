#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemCategoriesUI.h"

#include "ItemEdits.h"
#include "ItemSelection.h"
#include "ItemToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"

#include <algorithm>
#include <string>

using namespace MuEditor::Items;
using Data::Items::ItemDefinition;
using Data::Items::ItemTag;

namespace
{
// Pixel widths before the editor UI scale.
constexpr float TagListWidth = 200.0f;
constexpr float NumberColumnWidth = 70.0f;

constexpr int NumberColumn = 0;
constexpr int NameColumn = 1;
constexpr int RemoveColumn = 2;
constexpr int ColumnCount = 3;
constexpr int NoItem = -1;
constexpr const char* UnknownTagName = "?";

constexpr ImGuiTableFlags TableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                       ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;

const char* TagName(ItemTag tag)
{
    const char* name = Data::Items::FindEnumName(tag);
    return name != nullptr ? name : UnknownTagName;
}

// "group,number", as the item lists show it.
std::string ItemNumberText(int itemType)
{
    return std::to_string(Data::Items::GetItemGroup(itemType)) + "," +
           std::to_string(Data::Items::GetItemNumber(itemType));
}

// A grey line that wraps at the window's edge.
void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}
} // namespace

CMuItemCategoriesUI& CMuItemCategoriesUI::GetInstance()
{
    static CMuItemCategoriesUI instance;
    return instance;
}

void CMuItemCategoriesUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::ItemCategories, MuEditor::WindowIds::ItemCategories, open))
    {
        const int selected = g_ItemSelection.GetItemType();
        ToolWindow::RenderSelectedItem(selected);
        RenderSelectedItemTags(selected);
        // The tags are fixed: code asks for each of them (docs/item-data.md).
        RenderNote(I18N::Editor::NewTagsNeedCodeThatUsesThemItemTagInItemTagSetH);
        ImGui::Separator();

        const ImVec2 tagListSize(TagListWidth * g_MuEditorCore.GetUIScale(), 0.0f);
        if (ImGui::BeginChild("##Tags", tagListSize, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
        {
            RenderTagList();
        }
        ImGui::EndChild();
        ImGui::SameLine();
        if (ImGui::BeginChild("##TagItems"))
        {
            RenderTagItems(selected);
        }
        ImGui::EndChild();
    }
    ToolWindow::End();
}

void CMuItemCategoriesUI::RenderSelectedItemTags(int selectedItemType)
{
    const ItemDefinition* item = g_ItemDatabase.Find(selectedItemType);
    if (item == nullptr)
    {
        return;
    }
    if (item->tags.IsEmpty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::TheSelectedItemHasNoTags);
        return;
    }
    ImGui::TextUnformatted(I18N::Editor::TheSelectedItemSTags);
    for (const auto& tag : Data::Items::GetEnumNames(ItemTag{}))
    {
        if (!item->tags.Has(tag.value))
        {
            continue;
        }
        ImGui::SameLine();
        // A click shows the tag's items.
        if (ImGui::SmallButton(tag.name))
        {
            m_tag = tag.value;
            m_scrollToTag = true;
        }
    }
}

void CMuItemCategoriesUI::RenderTagList()
{
    UpdateTagLabels();
    const auto tags = Data::Items::GetEnumNames(ItemTag{});
    for (size_t index = 0; index < tags.size(); ++index)
    {
        const bool picked = tags[index].value == m_tag;
        if (ImGui::Selectable(m_tagLabels[index].c_str(), picked, MuEditor::KeyboardSelection::RowFlags))
        {
            m_tag = tags[index].value;
        }
        if (picked && m_scrollToTag)
        {
            ImGui::SetScrollHereY();
        }
    }
    m_scrollToTag = false;
}

void CMuItemCategoriesUI::RenderTagItems(int selectedItemType)
{
    UpdateRows();
    const bool reveal = RevealSelection(selectedItemType);
    ImGui::SeparatorText(TagName(m_tag));
    RenderAddButton(selectedItemType);
    if (m_rows.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::NoItemHasThisTag);
        return;
    }
    const int clicked = RenderRows(selectedItemType, reveal);
    if (clicked >= 0)
    {
        g_ItemSelection.Select(clicked);
        m_shownSerial = g_ItemSelection.GetSerial();
    }
}

void CMuItemCategoriesUI::RenderAddButton(int selectedItemType)
{
    const ItemDefinition* item = g_ItemDatabase.Find(selectedItemType);
    const bool hasTag = item != nullptr && item->tags.Has(m_tag);
    ImGui::BeginDisabled(item == nullptr || hasTag);
    if (ImGui::Button(I18N::Editor::AddTheSelectedItem))
    {
        SetTag(selectedItemType, true);
    }
    ImGui::EndDisabled();
    if (hasTag)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", I18N::Editor::TheSelectedItemAlreadyHasThisTag);
    }
}

int CMuItemCategoriesUI::RenderRows(int selectedItemType, bool reveal)
{
    if (!ImGui::BeginTable("##Items", ColumnCount, TableFlags))
    {
        return NoItem;
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_None, NumberColumnWidth * g_MuEditorCore.GetUIScale());
    ImGui::TableSetupColumn(I18N::Metadata::Name, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("##Remove", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_rows.size()));
    const auto selectedRow = std::ranges::find(m_rows, selectedItemType, &TagItemRow::itemType);
    const bool scrollToSelected = reveal && selectedRow != m_rows.end();
    if (scrollToSelected)
    {
        clipper.IncludeItemByIndex(static_cast<int>(selectedRow - m_rows.begin()));
    }
    int clicked = NoItem;
    while (clipper.Step())
    {
        for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
        {
            const TagItemRow& row = m_rows[index];
            const bool selected = row.itemType == selectedItemType;
            if (RenderRow(row, selected))
            {
                clicked = row.itemType;
            }
            if (selected && scrollToSelected)
            {
                ImGui::SetScrollHereY();
            }
        }
    }
    ImGui::EndTable();
    return clicked;
}

bool CMuItemCategoriesUI::RenderRow(const TagItemRow& row, bool selected)
{
    ImGui::PushID(row.itemType);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(NumberColumn);
    const bool clicked = ImGui::Selectable(row.number.c_str(), selected,
                                           ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap |
                                               MuEditor::KeyboardSelection::RowFlags);
    ImGui::TableSetColumnIndex(NameColumn);
    ImGui::TextUnformatted(row.name.c_str());
    ImGui::TableSetColumnIndex(RemoveColumn);
    if (ImGui::SmallButton(I18N::Editor::Remove))
    {
        SetTag(row.itemType, false);
    }
    ImGui::PopID();
    return clicked;
}

void CMuItemCategoriesUI::SetTag(int itemType, bool has)
{
    const ItemDefinition* item = g_ItemDatabase.Find(itemType);
    if (item == nullptr || item->tags.Has(m_tag) == has)
    {
        return;
    }
    ItemDefinition changed = *item;
    changed.tags.Set(m_tag, has);
    const char* label = has ? I18N::Editor::AddTheTag0 : I18N::Editor::RemoveTheTag0;
    g_ItemEdits.SetItem(I18N::Format(label, {TagName(m_tag)}), changed);
}

bool CMuItemCategoriesUI::RevealSelection(int selectedItemType)
{
    if (g_ItemSelection.GetSerial() == m_shownSerial)
    {
        return false;
    }
    m_shownSerial = g_ItemSelection.GetSerial();
    // The list scrolls to the item when the picked tag has it.
    return Data::Items::IsValidItemType(selectedItemType);
}

void CMuItemCategoriesUI::UpdateTagLabels()
{
    if (m_labelsVersion == g_ItemDatabase.GetVersion())
    {
        return;
    }
    m_labelsVersion = g_ItemDatabase.GetVersion();
    m_tagLabels.clear();
    for (const auto& tag : Data::Items::GetEnumNames(ItemTag{}))
    {
        const size_t count = g_ItemDatabase.GetItemTypesWithTag(tag.value).size();
        // "###" and the name keep the entry's id while its count changes.
        m_tagLabels.push_back(std::string(tag.name) + " (" + std::to_string(count) + ")###" + tag.name);
    }
}

void CMuItemCategoriesUI::UpdateRows()
{
    if (m_rowsVersion == g_ItemDatabase.GetVersion() && m_rowsNameVersion == g_ItemDatabase.GetNameVersion() &&
        m_rowsTag == m_tag)
    {
        return;
    }
    m_rowsVersion = g_ItemDatabase.GetVersion();
    m_rowsNameVersion = g_ItemDatabase.GetNameVersion();
    m_rowsTag = m_tag;
    m_rows.clear();
    for (const int itemType : g_ItemDatabase.GetItemTypesWithTag(m_tag))
    {
        const ItemDefinition* item = g_ItemDatabase.Find(itemType);
        if (item == nullptr)
        {
            continue;
        }
        m_rows.push_back({itemType, ItemNumberText(itemType), Core::Text::ToUtf8(item->name.c_str())});
    }
}

#endif // _EDITOR

#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemRulesUI.h"

#include "ItemEdits.h"
#include "ItemGroupFilter.h"
#include "ItemRuleFlags.h"
#include "ItemSelection.h"
#include "ItemTextInput.h"
#include "ItemToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <span>
#include <string>

using namespace MuEditor::Items;
using Data::Items::ItemDefinition;

namespace
{
// Pixel widths before the editor UI scale.
constexpr float SearchWidth = 180.0f;
constexpr float FilterWidth = 160.0f;
constexpr float RuleComboWidth = 220.0f;

// A grey line that wraps at the window's edge.
void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}
} // namespace

CMuItemRulesUI& CMuItemRulesUI::GetInstance()
{
    static CMuItemRulesUI instance;
    return instance;
}

void CMuItemRulesUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::ItemRules, MuEditor::WindowIds::ItemRules, open))
    {
        const int selected = g_ItemSelection.GetItemType();
        RenderFilters();
        // The flags are not the whole rule (docs/item-data.md, Rules).
        RenderNote(I18N::Editor::ExceptionsByItemLevelDurabilityOrGMStatusAreInTheCode);

        const bool reveal = RevealSelection(selected);
        const std::vector<int>& itemTypes = UpdateRows();
        RenderBulkActions(itemTypes);
        ImGui::Separator();
        const int clicked = m_table.Render(itemTypes, selected, reveal);
        if (clicked >= 0)
        {
            g_ItemSelection.Select(clicked);
            m_shownSerial = g_ItemSelection.GetSerial();
        }
    }
    ToolWindow::End();
}

void CMuItemRulesUI::RenderFilters()
{
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::SetNextItemWidth(SearchWidth * uiScale);
    InputStringWithHint("##Search", I18N::Editor::NameOrNumber, m_search);
    ImGui::SameLine();
    RenderGroupFilter(m_filter.group, FilterWidth * uiScale);
}

void CMuItemRulesUI::RenderBulkActions(const std::vector<int>& itemTypes)
{
    const int selectedRows = m_table.GetSelectedCount();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::SelectedItems0, {std::to_string(selectedRows)}).c_str());
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::SelectSeveralRowsWithCtrlClickOrShiftClick);
    }
    ImGui::SameLine();
    RenderBulkRuleCombo();
    ImGui::SameLine();
    ImGui::BeginDisabled(selectedRows == 0);
    if (ImGui::Button(I18N::Editor::AllowForTheSelectedItems))
    {
        SetBulkRule(itemTypes, true);
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::ForbidForTheSelectedItems))
    {
        SetBulkRule(itemTypes, false);
    }
    ImGui::EndDisabled();
}

void CMuItemRulesUI::RenderBulkRuleCombo()
{
    const std::span<const ItemRuleFlag> flags = GetItemRuleFlags();
    ImGui::SetNextItemWidth(RuleComboWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::BeginCombo("##BulkRule", *flags[m_bulkRule].label))
    {
        return;
    }
    for (size_t index = 0; index < flags.size(); ++index)
    {
        if (ImGui::Selectable(*flags[index].label, index == m_bulkRule))
        {
            m_bulkRule = index;
        }
    }
    ImGui::EndCombo();
}

void CMuItemRulesUI::SetBulkRule(const std::vector<int>& itemTypes, bool allowed)
{
    const ItemRuleFlag& flag = GetItemRuleFlags()[m_bulkRule];
    std::vector<ItemDefinition> changed;
    for (const int itemType : itemTypes)
    {
        const ItemDefinition* item = g_ItemDatabase.Find(itemType);
        if (item == nullptr || !m_table.IsSelected(itemType) || (item->*flag.value) == allowed)
        {
            continue;
        }
        changed.push_back(*item);
        changed.back().*flag.value = allowed;
    }
    if (changed.empty())
    {
        return;
    }
    const char* label = allowed ? I18N::Editor::AllowFor0Items1 : I18N::Editor::ForbidFor0Items1;
    g_ItemEdits.SetItems(I18N::Format(label, {std::to_string(changed.size()), *flag.label}), changed);
}

bool CMuItemRulesUI::RevealSelection(int selectedItemType)
{
    if (g_ItemSelection.GetSerial() == m_shownSerial)
    {
        return false;
    }
    m_shownSerial = g_ItemSelection.GetSerial();
    // Only items have rules: a number without one is never listed.
    if (g_ItemDatabase.Find(selectedItemType) == nullptr)
    {
        return false;
    }
    m_filter.search = m_search;
    const std::vector<int>& listed = m_list.GetItemTypes(m_filter);
    if (std::ranges::find(listed, selectedItemType) == listed.end())
    {
        m_filter = {};
        m_search.clear();
    }
    return true;
}

const std::vector<int>& CMuItemRulesUI::UpdateRows()
{
    m_filter.search = m_search;
    const std::vector<int>& itemTypes = m_list.GetItemTypes(m_filter);
    const long long version = ToolWindow::GetDataVersion();
    if (m_filter != m_listedFilter || version != m_listedVersion)
    {
        m_table.KeepListedRows(itemTypes);
        m_listedFilter = m_filter;
        m_listedVersion = version;
    }
    return itemTypes;
}

#endif // _EDITOR

#include "stdafx.h"

#ifdef _EDITOR

#include "ItemMoveDialog.h"

#include "ItemEdits.h"
#include "ItemGroupNames.h"
#include "ItemHeldByCharacter.h"
#include "ItemSelection.h"
#include "ItemUsageView.h"

#include "../MuEditor/UI/ItemEditor/ItemListModel.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/ItemData/ItemTypeUse.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>

namespace MuEditor::Items
{
namespace
{
constexpr char PopupId[] = "###ItemMoveDialog";
constexpr float FieldWidth = 220.0f;
constexpr ImVec4 NotFreeColor{1.0f, 0.4f, 0.4f, 1.0f};
} // namespace

void CItemMoveDialog::Open(int itemType)
{
    if (g_ItemDatabase.FindEntry(itemType) == nullptr)
    {
        return;
    }
    m_itemType = itemType;
    m_group = Data::Items::GetItemGroup(itemType);
    m_number = GetNextFreeItemNumber(m_group, Data::Items::GetItemNumber(itemType));
    m_usage = FindItemUsage(itemType);
    m_numberHolder = {};
    m_open = true;
    ImGui::OpenPopup(PopupId);
}

void CItemMoveDialog::Render()
{
    char title[128];
    std::snprintf(title, sizeof(title), "%s%s", I18N::Editor::MoveItem, PopupId);
    if (!ImGui::BeginPopupModal(title, &m_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    ImGui::TextUnformatted(GetEntryLabel(m_itemType).c_str());
    ImGui::SetNextItemWidth(FieldWidth);
    if (ImGui::BeginCombo(I18N::Editor::Group, GetItemGroupLabel(m_group).c_str()))
    {
        for (int group = 0; group < MAX_ITEM_TYPE; ++group)
        {
            if (ImGui::Selectable(GetItemGroupLabel(group).c_str(), group == m_group))
            {
                m_group = group;
                m_number = GetNextFreeItemNumber(group, 0);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SetNextItemWidth(FieldWidth);
    ImGui::InputInt(I18N::Editor::Number, &m_number);
    m_number = std::clamp(m_number, 0, MAX_ITEM_INDEX - 1);
    const int target = Data::Items::MakeItemType(m_group, m_number);
    const bool free = Data::Items::IsFreeItemType(target);
    if (!free)
    {
        ImGui::TextColored(NotFreeColor, "%s", I18N::Editor::ThisNumberIsNotFree);
        m_numberHolder.Render(target);
    }
    ImGui::Separator();
    if (m_usage.GoesWithItem())
    {
        ImGui::TextUnformatted(I18N::Editor::MovesWithIt);
        UsageView::RenderGoingWithItem(m_usage);
    }
    ImGui::TextUnformatted(I18N::Editor::WhatUsesItStaysAtItsOldNumber);
    UsageView::RenderUsesAtNumber(m_usage);
    ImGui::Separator();

    const bool held = IsHeldByCharacter(m_itemType);
    if (held)
    {
        ImGui::TextDisabled("%s", I18N::Editor::TheItemIsWornOrInAnItemWindowItIsNotMovedOrRemoved);
    }
    ImGui::BeginDisabled(!free || held);
    if (ImGui::Button(I18N::Editor::Move))
    {
        if (g_ItemEdits.MoveItem(I18N::Editor::MoveItem, m_itemType, target))
        {
            g_ItemSelection.Select(target);
        }
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}
} // namespace MuEditor::Items

#endif // _EDITOR

#include "stdafx.h"

#ifdef _EDITOR

#include "ItemRemoveDialog.h"

#include "ItemEdits.h"
#include "ItemHeldByCharacter.h"
#include "ItemUsageView.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <cstdio>

namespace MuEditor::Items
{
namespace
{
constexpr char PopupId[] = "###ItemRemoveDialog";
constexpr ImVec4 RemoveColor{0.7f, 0.2f, 0.2f, 1.0f};
} // namespace

void CItemRemoveDialog::Open(int itemType)
{
    if (g_ItemDatabase.FindEntry(itemType) == nullptr)
    {
        return;
    }
    m_itemType = itemType;
    m_usage = FindItemUsage(itemType);
    m_open = true;
    ImGui::OpenPopup(PopupId);
}

void CItemRemoveDialog::Render()
{
    char title[128];
    std::snprintf(title, sizeof(title), "%s%s", I18N::Editor::RemoveItem, PopupId);
    if (!ImGui::BeginPopupModal(title, &m_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    ImGui::TextUnformatted(GetEntryLabel(m_itemType).c_str());
    ImGui::TextUnformatted(I18N::Editor::TheItemAndItsModelEntryAreRemoved);
    ImGui::Separator();
    if (m_usage.GoesWithItem())
    {
        ImGui::TextUnformatted(I18N::Editor::RemovedWithIt);
        UsageView::RenderGoingWithItem(m_usage);
    }
    ImGui::TextUnformatted(I18N::Editor::WhatUsesItStaysAtItsNumber);
    UsageView::RenderUsesAtNumber(m_usage);
    ImGui::Separator();

    const bool held = IsHeldByCharacter(m_itemType);
    if (held)
    {
        ImGui::TextDisabled("%s", I18N::Editor::TheItemIsWornOrInAnItemWindowItIsNotMovedOrRemoved);
    }
    ImGui::BeginDisabled(held);
    ImGui::PushStyleColor(ImGuiCol_Button, RemoveColor);
    if (ImGui::Button(I18N::Editor::Remove))
    {
        g_ItemEdits.RemoveItem(I18N::Editor::RemoveItem, m_itemType);
        ImGui::CloseCurrentPopup();
    }
    ImGui::PopStyleColor();
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

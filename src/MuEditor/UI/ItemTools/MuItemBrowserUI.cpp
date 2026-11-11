#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemBrowserUI.h"

#include "ItemEdits.h"
#include "ItemGroupFilter.h"
#include "ItemRuleFlags.h"
#include "ItemSelection.h"
#include "ItemTextInput.h"
#include "ItemToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "Data/GameData/ItemData/ItemTypeUse.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorActions.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorPopups.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <string>
#include <string_view>

using namespace MuEditor::Items;

namespace
{
// Pixel widths before the editor UI scale.
constexpr float SearchWidth = 180.0f;
constexpr float FilterWidth = 160.0f;
constexpr const char* NeutralLocale = "en";
} // namespace

CMuItemBrowserUI& CMuItemBrowserUI::GetInstance()
{
    static CMuItemBrowserUI instance;
    return instance;
}

void CMuItemBrowserUI::Render(bool* open)
{
    // A mark in the title while there are changes that are not saved.
    std::string title = I18N::Editor::ItemBrowser;
    if (g_ItemEdits.GetHistory().IsModifiedSinceSave())
    {
        title += " *";
    }
    if (ToolWindow::Begin(title.c_str(), MuEditor::WindowIds::ItemBrowser, open))
    {
        const int selected = g_ItemSelection.GetItemType();
        RenderFileActions();
        RenderFilters();
        RenderItemActions(selected);
        ImGui::Separator();

        const bool reveal = RevealSelection(selected);
        m_filter.search = m_search;
        const int clicked = m_rows.Render(m_list.GetItemTypes(m_filter), selected, reveal);
        if (clicked >= 0)
        {
            g_ItemSelection.Select(clicked);
            m_shownSerial = g_ItemSelection.GetSerial();
        }
        m_addDialog.Render();
        m_removeDialog.Render();
        m_moveDialog.Render();
    }
    ToolWindow::End();
}

void CMuItemBrowserUI::RenderFileActions()
{
    if (CItemEditorActions::RenderAllButtons())
    {
        m_list.Refresh();
    }
    CItemEditorPopups::RenderAll();
}

void CMuItemBrowserUI::RenderFilters()
{
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::SetNextItemWidth(SearchWidth * uiScale);
    InputStringWithHint("##Search", I18N::Editor::NameOrNumber, m_search);
    ImGui::SameLine();
    RenderGroupFilter(m_filter.group, FilterWidth * uiScale);
    ImGui::SameLine();
    RenderTagFilter();
    ImGui::SameLine();
    RenderRuleFilter();
    ImGui::SameLine();
    RenderMissingFilter();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::ClearFilters))
    {
        m_filter = {};
        m_search.clear();
    }
}

void CMuItemBrowserUI::RenderTagFilter()
{
    ImGui::SetNextItemWidth(FilterWidth * g_MuEditorCore.GetUIScale());
    const char* shown = m_filter.tag ? Data::Items::FindEnumName(*m_filter.tag) : I18N::Editor::AllTags;
    if (!ImGui::BeginCombo("##Tag", shown != nullptr ? shown : ""))
    {
        return;
    }
    if (ImGui::Selectable(I18N::Editor::AllTags, !m_filter.tag))
    {
        m_filter.tag.reset();
    }
    for (const auto& tag : Data::Items::GetEnumNames(Data::Items::ItemTag{}))
    {
        if (ImGui::Selectable(tag.name, m_filter.tag == tag.value))
        {
            m_filter.tag = tag.value;
        }
    }
    ImGui::EndCombo();
}

void CMuItemBrowserUI::RenderRuleFilter()
{
    ImGui::SetNextItemWidth(FilterWidth * g_MuEditorCore.GetUIScale());
    const auto flags = GetItemRuleFlags();
    const auto current =
        std::ranges::find_if(flags, [&](const ItemRuleFlag& flag) { return m_filter.notAllowed == flag.action; });
    const std::string shown = current != flags.end() ? I18N::Format(I18N::Editor::Not0, {*current->label})
                                                     : std::string(I18N::Editor::AllRules);
    if (!ImGui::BeginCombo("##Rule", shown.c_str()))
    {
        return;
    }
    if (ImGui::Selectable(I18N::Editor::AllRules, !m_filter.notAllowed))
    {
        m_filter.notAllowed.reset();
    }
    for (const ItemRuleFlag& flag : flags)
    {
        if (ImGui::Selectable(I18N::Format(I18N::Editor::Not0, {*flag.label}).c_str(),
                              m_filter.notAllowed == flag.action))
        {
            m_filter.notAllowed = flag.action;
        }
    }
    ImGui::EndCombo();
}

void CMuItemBrowserUI::RenderMissingFilter()
{
    const std::span<const char* const> locales = I18N::GetAvailableLocales();
    const auto missingName = [](const char* locale)
    { return I18N::Format(I18N::Editor::MissingANameIn0, {I18N::GetLanguageDisplayName(locale)}); };
    std::string shown = I18N::Editor::AllItems;
    if (m_filter.withoutModel)
    {
        shown = I18N::Editor::WithoutAModel;
    }
    else if (m_filter.modelsWithoutItem)
    {
        shown = I18N::Editor::ModelsWithoutAnItem;
    }
    else if (!m_filter.missingLocale.empty())
    {
        shown = missingName(m_filter.missingLocale.c_str());
    }
    ImGui::SetNextItemWidth(FilterWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::BeginCombo("##Missing", shown.c_str()))
    {
        return;
    }
    const auto choose = [&](bool withoutModel, bool modelsWithoutItem, const std::string& locale)
    {
        m_filter.withoutModel = withoutModel;
        m_filter.modelsWithoutItem = modelsWithoutItem;
        m_filter.missingLocale = locale;
    };
    const bool everything = !m_filter.withoutModel && !m_filter.modelsWithoutItem && m_filter.missingLocale.empty();
    if (ImGui::Selectable(I18N::Editor::AllItems, everything))
    {
        choose(false, false, "");
    }
    if (ImGui::Selectable(I18N::Editor::WithoutAModel, m_filter.withoutModel))
    {
        choose(true, false, "");
    }
    if (ImGui::Selectable(I18N::Editor::ModelsWithoutAnItem, m_filter.modelsWithoutItem))
    {
        choose(false, true, "");
    }
    for (const char* locale : locales)
    {
        if (std::string_view(locale) != NeutralLocale &&
            ImGui::Selectable(missingName(locale).c_str(), m_filter.missingLocale == locale))
        {
            choose(false, false, locale);
        }
    }
    ImGui::EndCombo();
}

void CMuItemBrowserUI::RenderItemActions(int selectedItemType)
{
    const bool hasItem = g_ItemDatabase.Find(selectedItemType) != nullptr;
    // An entry without a name moves and goes as an item does; there is nothing to duplicate.
    const bool hasEntry = g_ItemDatabase.FindEntry(selectedItemType) != nullptr;
    if (ImGui::Button(I18N::Editor::AddItem))
    {
        m_addDialog.OpenAdd(selectedItemType);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!hasItem);
    if (ImGui::Button(I18N::Editor::DuplicateItem))
    {
        m_addDialog.OpenDuplicate(selectedItemType);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!hasEntry);
    if (ImGui::Button(I18N::Editor::MoveItem))
    {
        m_moveDialog.Open(selectedItemType);
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::RemoveItem))
    {
        m_removeDialog.Open(selectedItemType);
    }
    ImGui::EndDisabled();
}

bool CMuItemBrowserUI::RevealSelection(int selectedItemType)
{
    if (g_ItemSelection.GetSerial() == m_shownSerial)
    {
        return false;
    }
    m_shownSerial = g_ItemSelection.GetSerial();
    // A number with neither an entry nor a model entry (an undone Add) is in no
    // list: the filters stay.
    if (!Data::Items::IsValidItemType(selectedItemType) || Data::Items::IsFreeItemType(selectedItemType))
    {
        return false;
    }
    m_filter.search = m_search;
    const std::vector<int>& listed = m_list.GetItemTypes(m_filter);
    if (std::ranges::find(listed, selectedItemType) == listed.end())
    {
        // All items lists it: an item, an entry without a name or a model without an entry.
        m_filter = {};
        m_search.clear();
    }
    return true;
}

#endif // _EDITOR

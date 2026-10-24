#include "stdafx.h"

#ifdef _EDITOR

#include "ItemNameRows.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "../MuEditor/UI/ItemTools/ItemEdits.h"
#include "../MuEditor/UI/ItemTools/ItemSelection.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemNames.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

#include <cstdio>

const std::vector<int>& CItemNameRows::List(std::string_view search, std::string_view missingLocale)
{
    m_filter.search.assign(search);
    m_filter.missingLocale.assign(missingLocale);
    return m_list.GetItemTypes(m_filter);
}

void CItemNameRows::Refresh()
{
    m_list.Refresh();
}

int CItemNameRows::GetVersion() const
{
    return g_ItemDatabase.GetVersion();
}

std::vector<int> CItemNameRows::CountMissing(std::span<const char* const> locales) const
{
    std::vector<int> missing;
    for (const Data::Items::ItemNameCount& count : Data::Items::CountItemNames(g_ItemDatabase.GetAllSlots(), locales))
    {
        missing.push_back(count.missing);
    }
    return missing;
}

const Data::LocalizedString* CItemNameRows::FindNames(int id) const
{
    const Data::Items::ItemDefinition* definition = g_ItemDatabase.Find(id);
    return definition != nullptr ? &definition->names : nullptr;
}

void CItemNameRows::FormatId(int id, char* buffer, std::size_t size) const
{
    std::snprintf(buffer, size, "%d,%d", Data::Items::GetItemGroup(id), Data::Items::GetItemNumber(id));
}

void CItemNameRows::CommitName(int id, const char* locale, const char* text)
{
    // A name with the separator would be stored, then refuse every save.
    if (Data::LocalizedString::ContainsSeparator(text))
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::ItemNamesCannotContainTheSeparator);
        return;
    }
    // Through the item tools' history, so the name can be undone like any edit.
    if (!g_ItemEdits.SetItemName(I18N::GetLanguageDisplayName(locale), id, locale, text))
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::TheEnglishNameCannotBeEmpty);
        return;
    }
    m_list.KeepRowsAfterOwnEdit();
}

int CItemNameRows::GetSelected() const
{
    return g_ItemSelection.GetItemType();
}

int CItemNameRows::GetSelectionSerial() const
{
    return g_ItemSelection.GetSerial();
}

void CItemNameRows::Select(int id)
{
    g_ItemSelection.Select(id);
}

void CItemNameRows::ShowBrowser()
{
    g_MuEditorCore.GetToolWindows().itemBrowser = true;
    ImGui::SetWindowFocus(MuEditor::WindowIds::ItemBrowser);
}

const char* CItemNameRows::GetSelectTooltip() const
{
    return I18N::Editor::SelectTheItemAndShowTheItemBrowser;
}

const char* CItemNameRows::GetAllRowsLabel() const
{
    return I18N::Editor::AllItems;
}

#endif // _EDITOR

#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemTranslationsUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorActions.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorPopups.h"
#include "../MuEditor/UI/ItemEditor/MuItemEditorUI.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>
#include <string_view>

namespace
{
// Pixel widths before the editor UI scale.
constexpr float IdColumnWidth = 60.0f;
constexpr float NameColumnWidth = 190.0f;
constexpr float SearchWidth = 220.0f;
constexpr float MissingComboWidth = 220.0f;
constexpr int FrozenColumns = 2;

using Data::Items::ItemDefinition;

bool IsEnglish(std::string_view locale)
{
    return locale == Data::LocalizedString::NeutralLocale;
}

template <size_t Size> void CopyToBuffer(char (&buffer)[Size], const std::string& text)
{
    std::snprintf(buffer, Size, "%s", text.c_str());
}

std::string MissingLabel(const char* locale, const Data::Items::ItemNameCount& count)
{
    return std::string(I18N::GetLanguageDisplayName(locale)) + " (" +
           I18N::Format(I18N::Editor::CountMissing, {std::to_string(count.missing)}) + ")";
}
} // namespace

CMuItemTranslationsUI& CMuItemTranslationsUI::GetInstance()
{
    static CMuItemTranslationsUI instance;
    return instance;
}

void CMuItemTranslationsUI::Render(bool* open)
{
    m_locales = I18N::GetAvailableLocales();
    m_editDrawn = false;

    char title[MuEditor::WindowIds::TitleCapacity];
    MuEditor::Layout::PrepareToolWindow();
    if (ImGui::Begin(
            MuEditor::WindowIds::Title(title, I18N::Editor::ItemTranslations, MuEditor::WindowIds::ItemTranslations),
            open, ImGuiWindowFlags_NoCollapse))
    {
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup |
                                   ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) ||
            ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId))
        {
            g_MuEditorCore.SetHoveringUI(true);
        }
        UpdateCounts();
        CItemEditorActions::RenderSaveButton();
        CItemEditorPopups::RenderAll();
        ImGui::SameLine();
        RenderToolbar();
        ImGui::Separator();
        RenderTable();
    }
    ImGui::End();
    CommitEditNotDrawn();
}

void CMuItemTranslationsUI::UpdateCounts()
{
    if (m_countsVersion == g_ItemDatabase.GetVersion() && m_counts.size() == m_locales.size())
    {
        return;
    }
    m_counts = Data::Items::CountItemNames(g_ItemDatabase.GetAllSlots(), m_locales);
    m_countsVersion = g_ItemDatabase.GetVersion();
}

void CMuItemTranslationsUI::RenderToolbar()
{
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::SetNextItemWidth(SearchWidth * uiScale);
    ImGui::InputTextWithHint("##Search", I18N::Editor::Search, m_search, sizeof(m_search));

    ImGui::SameLine();
    ImGui::TextUnformatted(I18N::Editor::MissingIn);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(MissingComboWidth * uiScale);
    const char* shown =
        m_missingLocale >= 0 ? I18N::GetLanguageDisplayName(m_locales[m_missingLocale]) : I18N::Editor::AllItems;
    if (ImGui::BeginCombo("##MissingIn", shown))
    {
        if (ImGui::Selectable(I18N::Editor::AllItems, m_missingLocale < 0))
        {
            m_missingLocale = -1;
        }
        for (int index = 0; index < static_cast<int>(m_locales.size()); ++index)
        {
            if (IsEnglish(m_locales[index]))
            {
                continue;
            }
            if (ImGui::Selectable(MissingLabel(m_locales[index], m_counts[index]).c_str(), m_missingLocale == index))
            {
                m_missingLocale = index;
            }
        }
        ImGui::EndCombo();
    }

    // With a language's missing names listed, a filled name keeps its row until
    // the list is refreshed.
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Refresh))
    {
        m_list.Refresh();
    }
}

void CMuItemTranslationsUI::RenderTable()
{
    CItemListModel::Filter filter;
    filter.search = m_search;
    if (m_missingLocale >= 0)
    {
        filter.missingLocale = m_locales[m_missingLocale];
    }
    const std::vector<int>& itemTypes = m_list.GetItemTypes(filter);

    const int columnCount = static_cast<int>(m_locales.size()) + 1;
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable |
                                      ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##ItemTranslations", columnCount, flags))
    {
        return;
    }

    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::TableSetupScrollFreeze(FrozenColumns, 1);
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide, IdColumnWidth * uiScale);
    for (size_t column = 0; column < m_locales.size(); ++column)
    {
        const bool english = IsEnglish(m_locales[column]);
        // "###" and the language's code keep the header's id the same while its
        // count changes.
        std::string label = english ? std::string(I18N::GetLanguageDisplayName(m_locales[column]))
                                    : MissingLabel(m_locales[column], m_counts[column]);
        label += std::string("###") + m_locales[column];
        ImGui::TableSetupColumn(label.c_str(), english ? ImGuiTableColumnFlags_NoHide : ImGuiTableColumnFlags_None,
                                NameColumnWidth * uiScale);
    }
    ImGui::TableHeadersRow();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(itemTypes.size()));
    // The row being edited stays drawn when it scrolls out of view, so the edit
    // goes on.
    const auto editing = std::find(itemTypes.begin(), itemTypes.end(), m_editItem);
    if (editing != itemTypes.end())
    {
        clipper.IncludeItemByIndex(static_cast<int>(editing - itemTypes.begin()));
    }
    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            const int itemType = itemTypes[row];
            ImGui::PushID(itemType);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            RenderIdCell(itemType);
            for (size_t column = 0; column < m_locales.size(); ++column)
            {
                if (ImGui::TableSetColumnIndex(static_cast<int>(column) + 1))
                {
                    RenderNameCell(itemType, static_cast<int>(column), m_locales[column]);
                }
            }
            ImGui::PopID();
        }
    }
    ImGui::EndTable();
}

void CMuItemTranslationsUI::RenderIdCell(int itemType)
{
    char id[16];
    std::snprintf(id, sizeof(id), "%d,%d", Data::Items::GetItemGroup(itemType), Data::Items::GetItemNumber(itemType));
    if (ImGui::Selectable(id))
    {
        // The item table in front, with the item listed and selected.
        g_MuEditorCore.ShowItemEditor();
        g_MuItemEditorUI.SelectItem(itemType);
        ImGui::SetWindowFocus(MuEditor::WindowIds::ItemEditor);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::ShowInTheItemTable);
    }
}

void CMuItemTranslationsUI::RenderNameCell(int itemType, int column, const char* locale)
{
    const ItemDefinition* definition = g_ItemDatabase.Find(itemType);
    if (definition == nullptr)
    {
        return;
    }

    ImGui::PushID(column);
    // While the field is active, and once more when it stops being active,
    // ImGui writes its own text into this buffer; the text from the database
    // only counts while the field is not being edited.
    char buffer[NameBufferSize];
    CopyToBuffer(buffer, Data::Items::GetOwnName(*definition, locale));
    const bool wasEditCell = m_editItem == itemType && m_editColumn == column;
    ImGui::SetNextItemWidth(-FLT_MIN);
    // A missing name shows the English one it falls back to, greyed.
    ImGui::InputTextWithHint("##Name", definition->names.GetNeutral().c_str(), buffer, NameBufferSize);
    const bool active = ImGui::IsItemActive();
    if (active)
    {
        m_editItem = itemType;
        m_editColumn = column;
        m_editLocale = locale;
        m_editText = buffer;
        m_editDrawn = true;
    }
    else if (ImGui::IsItemDeactivated())
    {
        // Another cell may have taken over earlier in this frame: this cell's
        // edit is stored all the same, and the other cell stays the edited one.
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            CommitName(itemType, locale, buffer);
        }
        if (wasEditCell)
        {
            m_editItem = -1;
            m_editColumn = -1;
        }
    }

    // Not while the field is active: opening the menu would take the focus from
    // it before its text is stored.
    if (!IsEnglish(locale) && !active && ImGui::BeginPopupContextItem("##NameMenu"))
    {
        if (ImGui::MenuItem(I18N::Editor::SameAsEnglish))
        {
            CommitName(itemType, locale, definition->names.GetNeutral().c_str());
        }
        if (ImGui::MenuItem(I18N::Editor::RemoveTranslation, nullptr, false,
                            Data::Items::HasNameIn(*definition, locale)))
        {
            CommitName(itemType, locale, "");
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void CMuItemTranslationsUI::CommitName(int itemType, const char* locale, const char* text)
{
    const ItemDefinition* definition = g_ItemDatabase.Find(itemType);
    if (definition == nullptr || Data::Items::GetOwnName(*definition, locale) == text)
    {
        return;
    }
    if (!g_ItemDataHandler.SetItemName(itemType, locale, text))
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::TheEnglishNameCannotBeEmpty);
        return;
    }
    m_list.KeepRowsAfterOwnEdit();
}

void CMuItemTranslationsUI::CommitEditNotDrawn()
{
    if (m_editItem < 0 || m_editDrawn)
    {
        return;
    }
    CommitName(m_editItem, m_editLocale.c_str(), m_editText.c_str());
    m_editItem = -1;
    m_editColumn = -1;
}

#endif // _EDITOR

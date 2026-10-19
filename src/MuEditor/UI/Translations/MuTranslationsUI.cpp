#include "stdafx.h"

#ifdef _EDITOR

#include "MuTranslationsUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorActions.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorPopups.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorTable.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cctype>
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

// The item's own name in the language; empty when it shows the English one.
std::string OwnName(const ItemDefinition& definition, std::string_view locale)
{
    if (IsEnglish(locale))
    {
        return definition.names.GetNeutral();
    }
    const auto& translations = definition.names.GetTranslations();
    const auto translation = translations.find(locale);
    return translation != translations.end() ? translation->second : std::string();
}

template <size_t Size> void CopyToBuffer(char (&buffer)[Size], const std::string& text)
{
    std::snprintf(buffer, Size, "%s", text.c_str());
}

std::string ToLower(const char* text)
{
    std::string lower = text;
    for (char& character : lower)
    {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return lower;
}
} // namespace

CMuTranslationsUI& CMuTranslationsUI::GetInstance()
{
    static CMuTranslationsUI instance;
    return instance;
}

void CMuTranslationsUI::Render(bool* open)
{
    // English first, then the other languages of the UI.
    m_locales.clear();
    m_locales.push_back(Data::LocalizedString::NeutralLocale.data());
    for (const char* locale : I18N::GetAvailableLocales())
    {
        if (!IsEnglish(locale))
        {
            m_locales.push_back(locale);
        }
    }

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
}

void CMuTranslationsUI::UpdateCounts()
{
    if (m_countsVersion == g_ItemDatabase.GetVersion() && m_counts.size() == m_locales.size())
    {
        return;
    }
    m_counts = Data::Items::CountItemNames(g_ItemDatabase.GetAllSlots(), m_locales);
    m_countsVersion = g_ItemDatabase.GetVersion();
}

void CMuTranslationsUI::RenderToolbar()
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
        for (int index = 1; index < static_cast<int>(m_locales.size()); ++index)
        {
            const std::string label =
                std::string(I18N::GetLanguageDisplayName(m_locales[index])) + " (" +
                I18N::Format(I18N::Editor::CountMissing, {std::to_string(m_counts[index].missing)}) + ")";
            if (ImGui::Selectable(label.c_str(), m_missingLocale == index))
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

void CMuTranslationsUI::RenderTable()
{
    CItemListModel::Filter filter;
    filter.search = ToLower(m_search);
    if (m_missingLocale >= 0)
    {
        filter.missingLocale = m_locales[m_missingLocale];
    }
    const std::vector<int>& itemTypes = m_list.GetItemTypes(filter);

    const int columnCount = static_cast<int>(m_locales.size()) + 1;
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable |
                                      ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Translations", columnCount, flags))
    {
        return;
    }

    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::TableSetupScrollFreeze(FrozenColumns, 1);
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide, IdColumnWidth * uiScale);
    for (size_t column = 0; column < m_locales.size(); ++column)
    {
        // The language's code after "###" keeps the column's place and
        // visibility while its count changes.
        std::string label = I18N::GetLanguageDisplayName(m_locales[column]);
        if (column > 0)
        {
            label += " (" + I18N::Format(I18N::Editor::CountMissing, {std::to_string(m_counts[column].missing)}) + ")";
        }
        label += std::string("###") + m_locales[column];
        ImGui::TableSetupColumn(label.c_str(), column == 0 ? ImGuiTableColumnFlags_NoHide : ImGuiTableColumnFlags_None,
                                NameColumnWidth * uiScale);
    }
    ImGui::TableHeadersRow();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(itemTypes.size()));
    // The row being edited stays, also when it scrolls out of view, so the edit
    // is not lost.
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

void CMuTranslationsUI::RenderIdCell(int itemType)
{
    char id[16];
    std::snprintf(id, sizeof(id), "%d,%d", Data::Items::GetItemGroup(itemType), Data::Items::GetItemNumber(itemType));
    if (ImGui::Selectable(id))
    {
        g_MuEditorCore.ShowItemEditor();
        CItemEditorTable::RequestScrollToIndex(itemType);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::ShowInTheItemTable);
    }
}

void CMuTranslationsUI::RenderNameCell(int itemType, int column, const char* locale)
{
    const ItemDefinition* definition = g_ItemDatabase.Find(itemType);
    if (definition == nullptr)
    {
        return;
    }

    ImGui::PushID(column);
    const bool editing = m_editItem == itemType && m_editColumn == column;
    char shown[NameBufferSize];
    if (!editing)
    {
        CopyToBuffer(shown, OwnName(*definition, locale));
    }
    char* buffer = editing ? m_editBuffer : shown;
    ImGui::SetNextItemWidth(-FLT_MIN);
    // A missing name shows the English one it falls back to, greyed.
    ImGui::InputTextWithHint("##Name", definition->names.GetNeutral().c_str(), buffer, NameBufferSize);
    if (ImGui::IsItemActivated())
    {
        m_editItem = itemType;
        m_editColumn = column;
        CopyToBuffer(m_editBuffer, OwnName(*definition, locale));
    }
    if (ImGui::IsItemDeactivated())
    {
        if (ImGui::IsItemDeactivatedAfterEdit() && m_editItem == itemType && m_editColumn == column)
        {
            CommitName(itemType, locale);
        }
        m_editItem = -1;
        m_editColumn = -1;
    }

    if (!IsEnglish(locale) && ImGui::BeginPopupContextItem("##NameMenu"))
    {
        if (ImGui::MenuItem(I18N::Editor::SameAsEnglish))
        {
            g_ItemDataHandler.SetItemName(itemType, locale, definition->names.GetNeutral());
            m_list.KeepRowsAfterOwnEdit();
        }
        if (ImGui::MenuItem(I18N::Editor::RemoveTranslation, nullptr, false,
                            Data::Items::HasNameIn(*definition, locale)))
        {
            g_ItemDataHandler.SetItemName(itemType, locale, "");
            m_list.KeepRowsAfterOwnEdit();
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void CMuTranslationsUI::CommitName(int itemType, const char* locale)
{
    const ItemDefinition* definition = g_ItemDatabase.Find(itemType);
    if (definition == nullptr || OwnName(*definition, locale) == m_editBuffer)
    {
        return;
    }
    if (!g_ItemDataHandler.SetItemName(itemType, locale, m_editBuffer))
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::TheEnglishNameCannotBeEmpty);
        return;
    }
    m_list.KeepRowsAfterOwnEdit();
}

#endif // _EDITOR

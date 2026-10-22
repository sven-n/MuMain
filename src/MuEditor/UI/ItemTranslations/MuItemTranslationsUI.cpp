#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemTranslationsUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorActions.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorPopups.h"
#include "../MuEditor/UI/ItemTools/ItemEdits.h"
#include "../MuEditor/UI/ItemTools/ItemSelection.h"
#include "../MuEditor/UI/ItemTools/ItemToolWindow.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"

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
// The row of the item the tools show, as in the other item tools.
constexpr ImVec4 MarkedRowColor(0.9f, 0.7f, 0.0f, 0.25f);

using Data::Items::ItemDefinition;

bool IsEnglish(std::string_view locale)
{
    return locale == Data::LocalizedString::NeutralLocale;
}

// Lets ImGui size the std::string it edits to the text, as
// misc/cpp/imgui_stdlib does, so a name of any length is edited whole.
int ResizeCellText(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        auto* text = static_cast<std::string*>(data->UserData);
        text->resize(data->BufTextLen);
        data->Buf = text->data();
    }
    return 0;
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

    // One of the item tools: their docking, undo keys and selection.
    if (MuEditor::Items::ToolWindow::Begin(I18N::Editor::ItemTranslations, MuEditor::WindowIds::ItemTranslations, open))
    {
        UpdateCountsAndLabels();
        CItemEditorActions::RenderSaveButton();
        CItemEditorPopups::RenderAll();
        ImGui::SameLine();
        RenderToolbar();
        ImGui::Separator();
        RenderTable();
    }
    MuEditor::Items::ToolWindow::End();
    CommitEditNotDrawn();
}

void CMuItemTranslationsUI::UpdateCountsAndLabels()
{
    const char* uiLocale = I18N::GetCurrentLocale();
    if (m_countsVersion == g_ItemDatabase.GetVersion() && m_counts.size() == m_locales.size() &&
        m_labelsLocale == uiLocale)
    {
        return;
    }
    m_counts = Data::Items::CountItemNames(g_ItemDatabase.GetAllSlots(), m_locales);
    m_countsVersion = g_ItemDatabase.GetVersion();
    m_labelsLocale = uiLocale;

    m_missingLabels.clear();
    m_headerLabels.clear();
    for (size_t index = 0; index < m_locales.size(); ++index)
    {
        const char* locale = m_locales[index];
        m_missingLabels.push_back(IsEnglish(locale) ? std::string(I18N::GetLanguageDisplayName(locale))
                                                    : MissingLabel(locale, m_counts[index]));
        // "###" and the language's code keep the header's id the same while its
        // count changes.
        m_headerLabels.push_back(m_missingLabels.back() + "###" + locale);
    }
}

void CMuItemTranslationsUI::RenderToolbar()
{
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::SetNextItemWidth(SearchWidth * uiScale);
    ImGui::InputTextWithHint("##Search", I18N::Editor::NameOrNumber, m_search, sizeof(m_search));

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
            if (ImGui::Selectable(m_missingLabels[index].c_str(), m_missingLocale == index))
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

bool CMuItemTranslationsUI::RevealSelection(int selectedItemType)
{
    if (g_ItemSelection.GetSerial() == m_shownSerial)
    {
        return false;
    }
    m_shownSerial = g_ItemSelection.GetSerial();
    if (g_ItemDatabase.Find(selectedItemType) == nullptr)
    {
        return false;
    }
    m_filter.search.assign(m_search);
    m_filter.missingLocale.assign(m_missingLocale >= 0 ? m_locales[m_missingLocale] : "");
    const std::vector<int>& listed = m_list.GetItemTypes(m_filter);
    if (std::find(listed.begin(), listed.end(), selectedItemType) == listed.end())
    {
        m_search[0] = '\0';
        m_missingLocale = -1;
    }
    return true;
}

void CMuItemTranslationsUI::RenderTable()
{
    const int selected = g_ItemSelection.GetItemType();
    const bool reveal = RevealSelection(selected);
    // Assigned in place, so the strings keep their memory from frame to frame.
    m_filter.search.assign(m_search);
    m_filter.missingLocale.assign(m_missingLocale >= 0 ? m_locales[m_missingLocale] : "");
    const std::vector<int>& itemTypes = m_list.GetItemTypes(m_filter);

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
        ImGui::TableSetupColumn(m_headerLabels[column].c_str(),
                                english ? ImGuiTableColumnFlags_NoHide : ImGuiTableColumnFlags_None,
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
    const auto selectedRow = std::find(itemTypes.begin(), itemTypes.end(), selected);
    const bool scrollToSelected = reveal && selectedRow != itemTypes.end();
    if (scrollToSelected)
    {
        clipper.IncludeItemByIndex(static_cast<int>(selectedRow - itemTypes.begin()));
    }
    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            const int itemType = itemTypes[row];
            ImGui::PushID(itemType);
            ImGui::TableNextRow();
            if (itemType == selected)
            {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(MarkedRowColor));
            }
            ImGui::TableSetColumnIndex(0);
            if (itemType == selected && scrollToSelected)
            {
                ImGui::SetScrollHereY();
            }
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
    if (ImGui::Selectable(id, false, MuEditor::KeyboardSelection::RowFlags))
    {
        // The item selected for all item tools; a click also brings the item
        // browser to the front, an arrow key keeps the keyboard here.
        g_ItemSelection.Select(itemType);
        m_shownSerial = g_ItemSelection.GetSerial();
        if (!MuEditor::KeyboardSelection::MovedOnto())
        {
            g_MuEditorCore.GetToolWindows().itemBrowser = true;
            ImGui::SetWindowFocus(MuEditor::WindowIds::ItemBrowser);
        }
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::SelectTheItemAndShowTheItemBrowser);
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
    // ImGui writes its own text into m_cellText; the text from the database
    // only counts while the field is not being edited.
    m_cellText = Data::Items::GetOwnName(*definition, locale);
    const bool wasEditCell = m_editItem == itemType && m_editColumn == column;
    ImGui::SetNextItemWidth(-FLT_MIN);
    // A missing name shows the English one it falls back to, greyed.
    ImGui::InputTextWithHint("##Name", definition->names.GetNeutral().c_str(), m_cellText.data(),
                             m_cellText.capacity() + 1, ImGuiInputTextFlags_CallbackResize, ResizeCellText,
                             &m_cellText);
    const bool active = ImGui::IsItemActive();
    if (active)
    {
        m_editItem = itemType;
        m_editColumn = column;
        m_editText = m_cellText;
        m_editDrawn = true;
    }
    else if (ImGui::IsItemDeactivated())
    {
        // Another cell may have taken over earlier in this frame: this cell's
        // edit is stored all the same, and the other cell stays the edited one.
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            CommitName(itemType, locale, m_cellText.c_str());
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
    // A name with the separator would be stored, then refuse every save.
    if (std::string_view(text).find(Data::LocalizedString::Separator) != std::string_view::npos)
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::ItemNamesCannotContainTheSeparator);
        return;
    }
    // Through the item tools' history, so the name can be undone like any edit.
    if (!g_ItemEdits.SetItemName(I18N::GetLanguageDisplayName(locale), itemType, locale, text))
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
    CommitName(m_editItem, m_locales[m_editColumn], m_editText.c_str());
    m_editItem = -1;
    m_editColumn = -1;
}

#endif // _EDITOR

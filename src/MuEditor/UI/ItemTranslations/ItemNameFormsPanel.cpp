#include "stdafx.h"

#ifdef _EDITOR

#include "ItemNameFormsPanel.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/ItemTools/ItemEdits.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemTooltipLines.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/Translation/ItemReference.h"
#include "Data/Translation/ItemReferenceChecks.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace
{
using Data::Items::ItemDefinition;

// The name and case columns of the table, at UI scale 1.
constexpr int FixedColumns = 2;
constexpr float NameColumnWidth = 180.0f;
constexpr float FormColumnWidth = 140.0f;
// The rows the table shows at once; more scroll, so the names grid below keeps its room.
constexpr int MaxVisibleRows = 8;

// The text whose forms a text that names the item shows in `locale`: the item's names (index -1), or a name line's
// text (written in place or shared, "@x"), without conditions and without placeholders in that language (a line
// written as "{name}" takes the item's forms). None for other lines, whose forms never show (ValidateNameForms warns
// about them).
const Data::LocalizedString* NamesOf(const ItemDefinition& definition, int index, std::string_view locale)
{
    if (index < 0)
    {
        return &definition.names;
    }
    const Data::Items::ItemNameLine& line = definition.nameLines[static_cast<size_t>(index)];
    const Data::LocalizedString* text = Data::Items::FindLineText(g_ItemDatabase.GetTooltipFile(), line);
    if (!line.hasText || text == nullptr || !line.when.empty() || text->Get(locale).find('{') != std::string::npos)
    {
        return nullptr;
    }
    return text;
}

const Data::Items::ItemNameForms& FormsOf(const ItemDefinition& definition, int index)
{
    return index < 0 ? definition.nameForms : definition.nameLines[static_cast<size_t>(index)].nameForms;
}

// English names have no forms, also when NameCases.json names English (loading warns about it).
bool DeclinesNames(std::string_view locale)
{
    return locale != Data::LocalizedString::NeutralLocale;
}

// Whether the table has a row for the name `index` (-1: the item's name, else the name line) in the language.
bool HasRow(const ItemDefinition& definition, int index, std::string_view locale)
{
    const Data::LocalizedString* names = NamesOf(definition, index, locale);
    return names != nullptr && names->HasOwn(locale);
}

int CountRows(const ItemDefinition& definition)
{
    int rows = 0;
    for (const auto& [locale, cases] : g_ItemDatabase.GetNameCases().GetLanguages())
    {
        if (!DeclinesNames(locale))
        {
            continue;
        }
        for (int index = -1; index < static_cast<int>(definition.nameLines.size()); ++index)
        {
            rows += HasRow(definition, index, locale) ? 1 : 0;
        }
    }
    return rows;
}

// The table's height for `rows` rows of fields, the header row and the horizontal scroll bar; at most MaxVisibleRows
// rows, the rest scroll. A scrolling table without a height would take the whole rest of the window.
float TableHeight(int rows)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    const float rowHeight = ImGui::GetFrameHeight() + 2.0f * style.CellPadding.y;
    return static_cast<float>(std::min(rows, MaxVisibleRows) + 1) * rowHeight + style.ScrollbarSize;
}

// The languages of the Cases table: the UI languages but English, then the name cases file's other languages (a typo
// such as "pl-PL"; Data Checks warns about them), so they can be taken out.
std::vector<std::string> CasesTableLanguages()
{
    std::vector<std::string> locales;
    for (const char* locale : I18N::GetAvailableLocales())
    {
        if (std::string_view(locale) != Data::LocalizedString::NeutralLocale)
        {
            locales.emplace_back(locale);
        }
    }
    for (const auto& [locale, cases] : g_ItemDatabase.GetNameCases().GetLanguages())
    {
        if (std::find(locales.begin(), locales.end(), locale) == locales.end())
        {
            locales.push_back(locale);
        }
    }
    return locales;
}

bool IsUiLanguage(const std::string& locale)
{
    const auto locales = I18N::GetAvailableLocales();
    return std::any_of(locales.begin(), locales.end(), [&](const char* uiLocale) { return locale == uiLocale; });
}

void RenderCasesRow(const std::string& locale)
{
    ImGui::PushID(locale.c_str());
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    if (IsUiLanguage(locale))
    {
        ImGui::TextUnformatted(I18N::GetLanguageDisplayName(locale.c_str()));
    }
    else
    {
        ImGui::TextDisabled("%s", locale.c_str());
    }
    for (const std::string_view caseKey : Data::Translation::ItemCaseKeys)
    {
        ImGui::TableNextColumn();
        ImGui::PushID(std::string(caseKey).c_str());
        const Data::Items::ItemNameCases& current = g_ItemDatabase.GetNameCases();
        bool uses = current.Uses(locale, caseKey);
        if (ImGui::Checkbox("##Uses", &uses))
        {
            std::vector<std::string> cases = current.GetCases(locale);
            if (uses)
            {
                cases.emplace_back(caseKey);
            }
            else
            {
                std::erase(cases, std::string(caseKey));
            }
            Data::Items::ItemNameCases changed = current;
            changed.SetCases(locale, std::move(cases));
            // Through the item tools' history, so it can be undone like any edit.
            g_ItemEdits.SetNameCases(I18N::Editor::NameCases, changed);
        }
        ImGui::PopID();
    }
    ImGui::PopID();
}

void RenderCasesTable()
{
    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
    const int columns = 1 + static_cast<int>(Data::Translation::ItemCaseKeys.size());
    if (!ImGui::BeginTable("##NameCases", columns, flags))
    {
        return;
    }
    ImGui::TableSetupColumn(I18N::Editor::Language);
    for (const std::string_view caseKey : Data::Translation::ItemCaseKeys)
    {
        ImGui::TableSetupColumn(std::string(caseKey).c_str());
    }
    ImGui::TableHeadersRow();
    for (const std::string& locale : CasesTableLanguages())
    {
        RenderCasesRow(locale);
    }
    ImGui::EndTable();
}

// "+3" for a name line shown at one level, "+3 - +5" for more.
std::string LevelsOf(const Data::Items::ItemNameLine& line)
{
    std::string levels = "+" + std::to_string(line.minLevel);
    if (line.maxLevel != line.minLevel)
    {
        levels += " - +" + std::to_string(line.maxLevel);
    }
    return levels;
}
} // namespace

void CItemNameFormsPanel::Render(int itemType)
{
    const ItemDefinition* definition = g_ItemDatabase.Find(itemType);
    if (definition == nullptr)
    {
        return;
    }
    if (!ImGui::CollapsingHeader(I18N::Editor::NameForms))
    {
        return;
    }
    RenderUses(itemType);
    ImGui::SameLine();
    RenderCasesButton();
    if (g_ItemDatabase.GetNameCases().GetLanguages().empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::NoLanguageDeclinesItemNames);
        return;
    }
    RenderTable(*definition);
}

void CItemNameFormsPanel::RenderUses(int itemType)
{
    if (m_usesItemType != itemType || m_usesVersion != g_ItemDatabase.GetVersion())
    {
        m_uses = Data::Translation::FindTextsNamingItem(g_ItemDatabase, Data::Items::GetItemGroup(itemType),
                                                        Data::Items::GetItemNumber(itemType));
        m_usesItemType = itemType;
        m_usesVersion = g_ItemDatabase.GetVersion();
    }
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::TextsNamingTheItem0, {std::to_string(m_uses.size())}).c_str());
    if (!m_uses.empty() && ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        for (const std::string& use : m_uses)
        {
            ImGui::TextUnformatted(use.c_str());
        }
        ImGui::EndTooltip();
    }
}

void CItemNameFormsPanel::RenderCasesButton() const
{
    if (ImGui::SmallButton(I18N::Editor::NameCasesButton))
    {
        ImGui::OpenPopup("##NameCasesPopup");
    }
    if (ImGui::BeginPopup("##NameCasesPopup"))
    {
        ImGui::TextDisabled("%s", I18N::Editor::NameCasesNote);
        RenderCasesTable();
        ImGui::EndPopup();
    }
}

void CItemNameFormsPanel::RenderTable(const ItemDefinition& definition)
{
    const int rows = CountRows(definition);
    if (rows == 0)
    {
        return;
    }
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX |
                                      ImGuiTableFlags_ScrollY;
    const int columns = FixedColumns + static_cast<int>(Data::Translation::ItemCaseKeys.size());
    if (!ImGui::BeginTable("##NameForms", columns, flags, ImVec2(0.0f, TableHeight(rows))))
    {
        return;
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::TableSetupColumn(I18N::Editor::Name, ImGuiTableColumnFlags_None, NameColumnWidth * uiScale);
    ImGui::TableSetupColumn(I18N::Editor::Language);
    for (const std::string_view caseKey : Data::Translation::ItemCaseKeys)
    {
        ImGui::TableSetupColumn(std::string(caseKey).c_str(), ImGuiTableColumnFlags_None, FormColumnWidth * uiScale);
    }
    ImGui::TableHeadersRow();
    // The fields' IDs name the item: what was typed in a field left in the frame another item becomes selected never
    // lands on the new item's field.
    ImGui::PushID(Data::Items::MakeItemType(definition.group, definition.number));
    for (const auto& [locale, cases] : g_ItemDatabase.GetNameCases().GetLanguages())
    {
        if (!DeclinesNames(locale))
        {
            continue;
        }
        for (int index = -1; index < static_cast<int>(definition.nameLines.size()); ++index)
        {
            RenderRow(definition, index, locale);
        }
    }
    ImGui::PopID();
    ImGui::EndTable();
}

void CItemNameFormsPanel::RenderRow(const ItemDefinition& definition, int index, const std::string& locale)
{
    if (!HasRow(definition, index, locale))
    {
        return;
    }
    const Data::LocalizedString* names = NamesOf(definition, index, locale);
    ImGui::PushID(index);
    ImGui::PushID(locale.c_str());
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    const std::string label =
        index < 0 ? names->GetOwn(locale)
                  : LevelsOf(definition.nameLines[static_cast<size_t>(index)]) + " " + names->GetOwn(locale);
    ImGui::TextUnformatted(label.c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(I18N::GetLanguageDisplayName(locale.c_str()));
    for (const std::string_view caseKey : Data::Translation::ItemCaseKeys)
    {
        ImGui::TableNextColumn();
        RenderFormCell(definition, index, locale, std::string(caseKey));
    }
    ImGui::PopID();
    ImGui::PopID();
}

void CItemNameFormsPanel::RenderFormCell(const ItemDefinition& definition, int index, const std::string& locale,
                                         const std::string& caseKey)
{
    if (!g_ItemDatabase.GetNameCases().Uses(locale, caseKey))
    {
        ImGui::TextDisabled("-");
        return;
    }
    ImGui::PushID(caseKey.c_str());
    ImGui::SetNextItemWidth(-FLT_MIN);
    // An empty form shows the name it falls back to, greyed.
    std::string typed;
    if (MuEditor::Items::InputStringOnLeave("##Form", NamesOf(definition, index, locale)->GetOwn(locale).c_str(),
                                            FormsOf(definition, index).Find(locale, caseKey), typed))
    {
        CommitForm(definition, index, locale, caseKey, std::move(typed));
    }
    ImGui::PopID();
}

void CItemNameFormsPanel::CommitForm(const ItemDefinition& definition, int index, const std::string& locale,
                                     const std::string& caseKey, std::string text) const
{
    if (FormsOf(definition, index).Find(locale, caseKey) == text)
    {
        return;
    }
    // Through the item tools' history, so the form can be undone like any edit.
    ItemDefinition changed = definition;
    Data::Items::ItemNameForms& forms =
        index < 0 ? changed.nameForms : changed.nameLines[static_cast<size_t>(index)].nameForms;
    forms.Set(locale, caseKey, std::move(text));
    g_ItemEdits.SetItem(I18N::Editor::NameForm, changed);
}

#endif // _EDITOR

#include "stdafx.h"

#ifdef _EDITOR

#include "ItemNameFormsPanel.h"

#include "../MuEditor/UI/ItemTools/ItemEdits.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/Translation/ItemReference.h"
#include "Data/Translation/ItemReferenceChecks.h"
#include "I18N/All.h"
#include "imgui.h"

namespace
{
using Data::Items::ItemDefinition;

// The name and case columns of the table.
constexpr int FixedColumns = 2;
constexpr float NameColumnWidth = 180.0f;
constexpr float FormColumnWidth = 140.0f;

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

// The text that has forms: the item's names (index -1) or a name line's own text written in place without
// placeholders (a line written as "{name}" takes the item's forms); none for other lines.
const Data::LocalizedString* NamesOf(const ItemDefinition& definition, int index)
{
    if (index < 0)
    {
        return &definition.names;
    }
    const Data::Items::ItemNameLine& line = definition.nameLines[static_cast<size_t>(index)];
    if (!line.hasText || !line.textName.empty() || line.text.GetNeutral().find('{') != std::string::npos)
    {
        return nullptr;
    }
    return &line.text;
}

const Data::Items::ItemNameForms& FormsOf(const ItemDefinition& definition, int index)
{
    return index < 0 ? definition.nameForms : definition.nameLines[static_cast<size_t>(index)].nameForms;
}

// The languages that may decline names: the UI languages but English.
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
    for (const char* locale : I18N::GetAvailableLocales())
    {
        if (std::string_view(locale) == Data::LocalizedString::NeutralLocale)
        {
            continue;
        }
        ImGui::PushID(locale);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(I18N::GetLanguageDisplayName(locale));
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
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX;
    const int columns = FixedColumns + static_cast<int>(Data::Translation::ItemCaseKeys.size());
    if (!ImGui::BeginTable("##NameForms", columns, flags))
    {
        return;
    }
    ImGui::TableSetupColumn(I18N::Editor::Name, ImGuiTableColumnFlags_None, NameColumnWidth);
    ImGui::TableSetupColumn(I18N::Editor::Language);
    for (const std::string_view caseKey : Data::Translation::ItemCaseKeys)
    {
        ImGui::TableSetupColumn(std::string(caseKey).c_str(), ImGuiTableColumnFlags_None, FormColumnWidth);
    }
    ImGui::TableHeadersRow();
    for (const auto& [locale, cases] : g_ItemDatabase.GetNameCases().GetLanguages())
    {
        RenderRow(definition, -1, locale);
        for (int index = 0; index < static_cast<int>(definition.nameLines.size()); ++index)
        {
            RenderRow(definition, index, locale);
        }
    }
    ImGui::EndTable();
}

void CItemNameFormsPanel::RenderRow(const ItemDefinition& definition, int index, const std::string& locale)
{
    const Data::LocalizedString* names = NamesOf(definition, index);
    if (names == nullptr || !names->HasOwn(locale))
    {
        return;
    }
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
    // While the field is active, and once more when it stops being active, ImGui writes its own text into
    // m_cellText; the form from the data only counts while the field is not being edited.
    m_cellText = FormsOf(definition, index).Find(locale, caseKey);
    ImGui::SetNextItemWidth(-FLT_MIN);
    // An empty form shows the name it falls back to, greyed.
    ImGui::InputTextWithHint("##Form", NamesOf(definition, index)->GetOwn(locale).c_str(), m_cellText.data(),
                             m_cellText.capacity() + 1, ImGuiInputTextFlags_CallbackResize, ResizeCellText,
                             &m_cellText);
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        CommitForm(definition, index, locale, caseKey, m_cellText);
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

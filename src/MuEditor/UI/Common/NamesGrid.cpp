#include "stdafx.h"

#ifdef _EDITOR

#include "NamesGrid.h"

#include "KeyboardSelection.h"
#include "NamesGridRows.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <string_view>

namespace MuEditor
{
namespace
{
// Pixel widths before the editor UI scale.
constexpr float IdColumnWidth = 60.0f;
constexpr float NameColumnWidth = 190.0f;
constexpr float SearchWidth = 220.0f;
constexpr float MissingComboWidth = 220.0f;
constexpr int FrozenColumns = 2;
constexpr int IdBufferSize = 16;
// The row of the selection the tools show, as in the other tools.
constexpr ImVec4 MarkedRowColor(0.9f, 0.7f, 0.0f, 0.25f);

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

std::string MissingLabel(const char* locale, int missing)
{
    return std::string(I18N::GetLanguageDisplayName(locale)) + " (" +
           I18N::Format(I18N::Editor::CountMissing, {std::to_string(missing)}) + ")";
}
} // namespace

void NamesGrid::BeginFrame()
{
    m_locales = I18N::GetAvailableLocales();
    m_editDrawn = false;
}

void NamesGrid::UpdateCountsAndLabels(const NamesGridRows& rows)
{
    const char* uiLocale = I18N::GetCurrentLocale();
    if (m_countsVersion == rows.GetVersion() && m_missingCounts.size() == m_locales.size() &&
        m_labelsLocale == uiLocale)
    {
        return;
    }
    m_missingCounts = rows.CountMissing(m_locales);
    m_countsVersion = rows.GetVersion();
    m_labelsLocale = uiLocale;

    m_missingLabels.clear();
    m_headerLabels.clear();
    for (size_t index = 0; index < m_locales.size(); ++index)
    {
        const char* locale = m_locales[index];
        m_missingLabels.push_back(IsEnglish(locale) ? std::string(I18N::GetLanguageDisplayName(locale))
                                                    : MissingLabel(locale, m_missingCounts[index]));
        // "###" and the language's code keep the header's id the same while its
        // count changes.
        m_headerLabels.push_back(m_missingLabels.back() + "###" + locale);
    }
}

void NamesGrid::RenderToolbar(NamesGridRows& rows)
{
    const float uiScale = g_MuEditorCore.GetUIScale();
    ImGui::SetNextItemWidth(SearchWidth * uiScale);
    ImGui::InputTextWithHint("##Search", I18N::Editor::NameOrNumber, m_search, sizeof(m_search));

    ImGui::SameLine();
    ImGui::TextUnformatted(I18N::Editor::MissingIn);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(MissingComboWidth * uiScale);
    const char* shown =
        m_missingLocale >= 0 ? I18N::GetLanguageDisplayName(m_locales[m_missingLocale]) : rows.GetAllRowsLabel();
    if (ImGui::BeginCombo("##MissingIn", shown))
    {
        if (ImGui::Selectable(rows.GetAllRowsLabel(), m_missingLocale < 0))
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
        rows.Refresh();
    }
}

const std::vector<int>& NamesGrid::ListRows(NamesGridRows& rows) const
{
    return rows.List(m_search, m_missingLocale >= 0 ? std::string_view(m_locales[m_missingLocale]) : "");
}

bool NamesGrid::RevealSelection(NamesGridRows& rows, int selected)
{
    if (rows.GetSelectionSerial() == m_shownSerial)
    {
        return false;
    }
    m_shownSerial = rows.GetSelectionSerial();
    if (rows.FindNames(selected) == nullptr)
    {
        return false;
    }
    const std::vector<int>& listed = ListRows(rows);
    if (std::find(listed.begin(), listed.end(), selected) == listed.end())
    {
        m_search[0] = '\0';
        m_missingLocale = -1;
    }
    return true;
}

void NamesGrid::RenderTable(NamesGridRows& rows, const char* tableId)
{
    const int selected = rows.GetSelected();
    const bool reveal = RevealSelection(rows, selected);
    const std::vector<int>& ids = ListRows(rows);

    const int columnCount = static_cast<int>(m_locales.size()) + 1;
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable |
                                      ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable(tableId, columnCount, flags))
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
    clipper.Begin(static_cast<int>(ids.size()));
    // The row being edited stays drawn when it scrolls out of view, so the edit
    // goes on.
    const auto editing = std::find(ids.begin(), ids.end(), m_editId);
    if (editing != ids.end())
    {
        clipper.IncludeItemByIndex(static_cast<int>(editing - ids.begin()));
    }
    const auto selectedRow = std::find(ids.begin(), ids.end(), selected);
    const bool scrollToSelected = reveal && selectedRow != ids.end();
    if (scrollToSelected)
    {
        clipper.IncludeItemByIndex(static_cast<int>(selectedRow - ids.begin()));
    }
    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            const int id = ids[row];
            ImGui::PushID(id);
            ImGui::TableNextRow();
            if (id == selected)
            {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(MarkedRowColor));
            }
            ImGui::TableSetColumnIndex(0);
            if (id == selected && scrollToSelected)
            {
                ImGui::SetScrollHereY();
            }
            RenderIdCell(rows, id);
            for (size_t column = 0; column < m_locales.size(); ++column)
            {
                if (ImGui::TableSetColumnIndex(static_cast<int>(column) + 1))
                {
                    RenderNameCell(rows, id, static_cast<int>(column), m_locales[column]);
                }
            }
            ImGui::PopID();
        }
    }
    ImGui::EndTable();
}

void NamesGrid::RenderIdCell(NamesGridRows& rows, int id)
{
    char text[IdBufferSize];
    rows.FormatId(id, text, sizeof(text));
    if (ImGui::Selectable(text, false, KeyboardSelection::RowFlags))
    {
        // The row selected for all the data's tools; a click also brings the
        // browser to the front, an arrow key keeps the keyboard here.
        rows.Select(id);
        m_shownSerial = rows.GetSelectionSerial();
        if (!KeyboardSelection::MovedOnto())
        {
            rows.ShowBrowser();
        }
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", rows.GetSelectTooltip());
    }
}

void NamesGrid::RenderNameCell(NamesGridRows& rows, int id, int column, const char* locale)
{
    const Data::LocalizedString* names = rows.FindNames(id);
    if (names == nullptr)
    {
        return;
    }

    ImGui::PushID(column);
    // While the field is active, and once more when it stops being active,
    // ImGui writes its own text into m_cellText; the text from the data only
    // counts while the field is not being edited.
    m_cellText = names->GetOwn(locale);
    const bool wasEditCell = m_editId == id && m_editColumn == column;
    ImGui::SetNextItemWidth(-FLT_MIN);
    // A missing name shows the English one it falls back to, greyed.
    ImGui::InputTextWithHint("##Name", names->GetNeutral().c_str(), m_cellText.data(), m_cellText.capacity() + 1,
                             ImGuiInputTextFlags_CallbackResize, ResizeCellText, &m_cellText);
    const bool active = ImGui::IsItemActive();
    if (active)
    {
        m_editId = id;
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
            CommitName(rows, id, locale, m_cellText.c_str());
        }
        if (wasEditCell)
        {
            m_editId = -1;
            m_editColumn = -1;
        }
    }

    // Not while the field is active: opening the menu would take the focus from
    // it before its text is stored.
    if (!IsEnglish(locale) && !active && ImGui::BeginPopupContextItem("##NameMenu"))
    {
        if (ImGui::MenuItem(I18N::Editor::SameAsEnglish))
        {
            CommitName(rows, id, locale, names->GetNeutral().c_str());
        }
        if (ImGui::MenuItem(I18N::Editor::RemoveTranslation, nullptr, false, names->HasOwn(locale)))
        {
            CommitName(rows, id, locale, "");
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void NamesGrid::CommitName(NamesGridRows& rows, int id, const char* locale, const char* text) const
{
    const Data::LocalizedString* names = rows.FindNames(id);
    if (names == nullptr || names->GetOwn(locale) == text)
    {
        return;
    }
    rows.CommitName(id, locale, text);
}

void NamesGrid::CommitEditNotDrawn(NamesGridRows& rows)
{
    if (m_editId < 0 || m_editDrawn)
    {
        return;
    }
    CommitName(rows, m_editId, m_locales[m_editColumn], m_editText.c_str());
    m_editId = -1;
    m_editColumn = -1;
}
} // namespace MuEditor

#endif // _EDITOR

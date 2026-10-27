#pragma once

#ifdef _EDITOR

#include <span>
#include <string>
#include <vector>

namespace MuEditor
{
class NamesGridRows;

// The names of a kind of data as a grid of its rows and the languages of the
// UI (English first), with how many names each language misses and a filter
// for them. Edits go to the rows (NamesGridRows::CommitName) right away.
//
// A window draws it in this order each frame: BeginFrame, then inside the
// window UpdateCountsAndLabels, RenderToolbar and RenderTable, and after the
// window's End CommitEditNotDrawn.
class NamesGrid
{
public:
    void BeginFrame();
    // The counts and the labels built from them, again when the rows' data or
    // the UI language changed.
    void UpdateCountsAndLabels(const NamesGridRows& rows);
    // The search, the Missing in filter and Refresh.
    void RenderToolbar(NamesGridRows& rows);
    // `tableId` keeps the table's column widths and hidden columns.
    void RenderTable(NamesGridRows& rows, const char* tableId);
    // Stores the text of a cell that stopped being drawn while it was being
    // edited (its column or the window hidden, its row gone from the list):
    // ImGui then drops the edit without reporting its end.
    void CommitEditNotDrawn(NamesGridRows& rows);

private:
    static constexpr int SearchBufferSize = 256;

    // When a row was selected elsewhere: clears the search and the missing
    // filter if they hide it. Returns whether to scroll to it.
    bool RevealSelection(NamesGridRows& rows, int selected);
    const std::vector<int>& ListRows(NamesGridRows& rows) const;
    void RenderIdCell(NamesGridRows& rows, int id);
    void RenderNameCell(NamesGridRows& rows, int id, int column, const char* locale);
    void CommitName(NamesGridRows& rows, int id, const char* locale, const char* text) const;

    // The UI languages, English first (I18N::GetAvailableLocales).
    std::span<const char* const> m_locales;
    std::vector<int> m_missingCounts;
    int m_countsVersion = -1;
    // Per language, built with the counts so drawing does not format them each
    // frame: the Missing in entry ("Deutsch (3 missing)") and the column header
    // (the entry with "###" and the code). In the UI language m_labelsLocale.
    std::vector<std::string> m_missingLabels;
    std::vector<std::string> m_headerLabels;
    std::string m_labelsLocale;

    char m_search[SearchBufferSize] = {};
    // Index into m_locales of the language whose missing names are listed, or
    // -1 for all rows.
    int m_missingLocale = -1;
    // The selection serial last shown (NamesGridRows::GetSelectionSerial).
    int m_shownSerial = -1;

    // The text of the cell being drawn: one string for all cells, so drawing
    // allocates only when a longer name comes.
    std::string m_cellText;
    // The cell being edited (its column indexes m_locales), its text so far,
    // and whether it was drawn this frame.
    int m_editId = -1;
    int m_editColumn = -1;
    std::string m_editText;
    bool m_editDrawn = false;
};
} // namespace MuEditor

#endif // _EDITOR

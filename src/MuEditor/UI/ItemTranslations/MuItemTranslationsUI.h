#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/ItemEditor/ItemListModel.h"

#include "Data/GameData/ItemData/ItemNames.h"

#include <span>
#include <string>
#include <vector>

// The Item Translations window: the item names as a grid of items and the
// languages of the UI (English first), with how many names each language
// misses and a filter for them. Edits go into the item database right away;
// Save Items writes them.
class CMuItemTranslationsUI
{
public:
    static CMuItemTranslationsUI& GetInstance();

    void Render(bool* open);

private:
    static constexpr int SearchBufferSize = 256;

    void RenderToolbar();
    void RenderTable();
    // When an item was selected elsewhere: clears the search and the missing
    // filter if they hide it. Returns whether to scroll to it.
    bool RevealSelection(int selectedItemType);
    void RenderIdCell(int itemType);
    void RenderNameCell(int itemType, int column, const char* locale);
    void CommitName(int itemType, const char* locale, const char* text);
    // Stores the text of a cell that stopped being drawn while it was being
    // edited (its column or the window hidden, its row gone from the list):
    // ImGui then drops the edit without reporting its end.
    void CommitEditNotDrawn();
    // The counts and the labels built from them, again when the item database
    // or the UI language changed.
    void UpdateCountsAndLabels();

    // The UI languages, English first (I18N::GetAvailableLocales).
    std::span<const char* const> m_locales;
    std::vector<Data::Items::ItemNameCount> m_counts;
    int m_countsVersion = -1;
    // Per language, built with the counts so drawing does not format them each
    // frame: the Missing in entry ("Deutsch (3 missing)") and the column header
    // (the entry with "###" and the code). In the UI language m_labelsLocale.
    std::vector<std::string> m_missingLabels;
    std::vector<std::string> m_headerLabels;
    std::string m_labelsLocale;

    char m_search[SearchBufferSize] = {};
    // Index into m_locales of the language whose missing names are listed, or
    // -1 for all items.
    int m_missingLocale = -1;
    // The selection serial last shown (ItemSelection::GetSerial).
    int m_shownSerial = -1;
    CItemListModel::Filter m_filter;
    CItemListModel m_list;

    // The text of the cell being drawn: one string for all cells, so drawing
    // allocates only when a longer name comes.
    std::string m_cellText;
    // The cell being edited (its column indexes m_locales), its text so far,
    // and whether it was drawn this frame.
    int m_editItem = -1;
    int m_editColumn = -1;
    std::string m_editText;
    bool m_editDrawn = false;
};

#define g_MuItemTranslationsUI CMuItemTranslationsUI::GetInstance()

#endif // _EDITOR

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
    static constexpr int NameBufferSize = 256;

    void RenderToolbar();
    void RenderTable();
    void RenderIdCell(int itemType);
    void RenderNameCell(int itemType, int column, const char* locale);
    void CommitName(int itemType, const char* locale, const char* text);
    // Stores the text of a cell that stopped being drawn while it was being
    // edited (its column or the window hidden, its row gone from the list):
    // ImGui then drops the edit without reporting its end.
    void CommitEditNotDrawn();
    void UpdateCounts();

    // The UI languages, English first (I18N::GetAvailableLocales).
    std::span<const char* const> m_locales;
    std::vector<Data::Items::ItemNameCount> m_counts;
    int m_countsVersion = -1;

    char m_search[NameBufferSize] = {};
    // Index into m_locales of the language whose missing names are listed, or
    // -1 for all items.
    int m_missingLocale = -1;
    CItemListModel m_list;

    // The cell being edited, its text so far, and whether it was drawn this
    // frame.
    int m_editItem = -1;
    int m_editColumn = -1;
    std::string m_editLocale;
    std::string m_editText;
    bool m_editDrawn = false;
};

#define g_MuItemTranslationsUI CMuItemTranslationsUI::GetInstance()

#endif // _EDITOR

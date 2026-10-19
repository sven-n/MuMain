#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/ItemEditor/ItemListModel.h"

#include "Data/GameData/ItemData/ItemNames.h"

#include <string>
#include <vector>

// The translations tool: the item names as a grid of items and languages (the
// languages of the UI, English first), with how many names each language
// misses and a filter for them. Edits go into the item database right away;
// Save Items writes them.
class CMuTranslationsUI
{
public:
    static CMuTranslationsUI& GetInstance();

    void Render(bool* open);

private:
    static constexpr int NameBufferSize = 256;

    void RenderToolbar();
    void RenderTable();
    void RenderIdCell(int itemType);
    void RenderNameCell(int itemType, int column, const char* locale);
    void CommitName(int itemType, const char* locale);
    void UpdateCounts();

    std::vector<const char*> m_locales;
    std::vector<Data::Items::ItemNameCount> m_counts;
    int m_countsVersion = -1;

    char m_search[NameBufferSize] = {};
    // Index into m_locales of the language whose missing names are listed, or
    // -1 for all items.
    int m_missingLocale = -1;
    CItemListModel m_list;

    // The cell being edited, and its text while it is.
    int m_editItem = -1;
    int m_editColumn = -1;
    char m_editBuffer[NameBufferSize] = {};
};

#define g_MuTranslationsUI CMuTranslationsUI::GetInstance()

#endif // _EDITOR

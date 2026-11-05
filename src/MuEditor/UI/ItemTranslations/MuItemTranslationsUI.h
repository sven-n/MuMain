#pragma once

#ifdef _EDITOR

#include "ItemNameRows.h"
#include "../MuEditor/UI/Common/NamesGrid.h"

// The Item Translations window: the item names as a grid of items and the
// languages of the UI (English first), with how many names each language
// misses and a filter for them (the names grid). Edits go into the item
// database right away; Save Items writes them.
class CMuItemTranslationsUI
{
public:
    static CMuItemTranslationsUI& GetInstance();

    void Render(bool* open);

private:
    MuEditor::NamesGrid m_grid;
    CItemNameRows m_rows;
};

#define g_MuItemTranslationsUI CMuItemTranslationsUI::GetInstance()

#endif // _EDITOR

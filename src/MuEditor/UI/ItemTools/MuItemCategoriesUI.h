#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemTagSet.h"

#include <string>
#include <vector>

// The item categories tool (section 9): the tags with how many items have
// each, and the items of the picked tag, where the selected item is added
// and items are removed. A click selects the item for all item tools.
class CMuItemCategoriesUI
{
public:
    static CMuItemCategoriesUI& GetInstance();

    void Render(bool* open);

private:
    // An item of the picked tag, with the texts its row shows.
    struct TagItemRow
    {
        int itemType;
        std::string number;
        std::string name;
    };

    CMuItemCategoriesUI() = default;

    void RenderSelectedItemTags(int selectedItemType);
    void RenderTagList();
    void RenderTagItems(int selectedItemType);
    void RenderAddButton(int selectedItemType);
    // Returns the item type whose row was clicked, or -1.
    int RenderRows(int selectedItemType, bool reveal);
    bool RenderRow(const TagItemRow& row, bool selected);
    // Gives the item the picked tag or takes it away, as one undo step.
    void SetTag(int itemType, bool has);
    // When the selection changed elsewhere: whether the list scrolls to it.
    bool RevealSelection(int selectedItemType);
    // The labels of the tag list ("jewel (12)"), again when the items changed.
    void UpdateTagLabels();
    // The rows of the picked tag, again when the tag, the items or the UI
    // language changed.
    void UpdateRows();

    // The first tag (cape) until another is picked.
    Data::Items::ItemTag m_tag{};
    // The tag list picked a tag elsewhere and scrolls to it.
    bool m_scrollToTag = false;
    // In the order of GetEnumNames(ItemTag{}).
    std::vector<std::string> m_tagLabels;
    int m_labelsVersion = -1;
    std::vector<TagItemRow> m_rows;
    Data::Items::ItemTag m_rowsTag{};
    int m_rowsVersion = -1;
    int m_rowsNameVersion = -1;
    int m_shownSerial = -1;
};

#define g_MuItemCategoriesUI CMuItemCategoriesUI::GetInstance()

#endif // _EDITOR

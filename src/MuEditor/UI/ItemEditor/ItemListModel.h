#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemTagSet.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The items the item tools list: every item that exists, in item type order,
// narrowed by a search and the filters (group, tag, rule flag, missing name,
// missing model), or the numbers that have a model but no item. Kept apart
// from ImGui so it can be tested; the item browser and Item Translations use
// it.
class CItemListModel
{
public:
    struct Filter
    {
        // Matches "group,number", the item type or a name in any language;
        // letters A to Z ignore case.
        std::string search;
        // Only items without a name of their own in this language; empty for
        // all items.
        std::string missingLocale;
        // Only the items of this group.
        std::optional<int> group;
        // Only the items with this tag.
        std::optional<Data::Items::ItemTag> tag;
        // Only the items that do not allow this (a rule flag that is off).
        std::optional<Data::Items::ItemAction> notAllowed;
        // Only the items without a model entry.
        bool withoutModel = false;
        // Instead of the items, the numbers that have a model entry but no
        // item (the search and the group still apply).
        bool modelsWithoutItem = false;

        bool operator==(const Filter& other) const = default;
    };

    // The item types; built again when the filter, the item database or the
    // item model database changed, or after Refresh.
    const std::vector<int>& GetItemTypes(const Filter& filter);
    // After an edit made through the list: the rows stay as they are until the
    // filter changes or Refresh, so a filled name does not take its row away
    // while the list is being worked through.
    void KeepRowsAfterOwnEdit();
    void Refresh();

private:
    std::vector<int> m_itemTypes;
    bool Lists(int itemType, const Filter& filter) const;

    Filter m_filter;
    int m_version = -1;
    int m_modelVersion = -1;
    bool m_built = false;
};

// Whether the item matches the search text (see CItemListModel::Filter).
bool MatchesItemSearch(const Data::Items::ItemDefinition& definition, std::string_view search);

// The numbers of the group a new item can take (Data::Items::IsFreeItemType).
std::vector<int> GetFreeItemNumbers(int group);
// The first of them from `from` on, else the first of the group; 0 when the
// group has none.
int GetNextFreeItemNumber(int group, int from);

#endif // _EDITOR

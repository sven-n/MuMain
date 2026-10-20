#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <string>
#include <string_view>
#include <vector>

// The items the item tools list: every item that exists, in item type order,
// narrowed by a search and, if set, to the items without a name of their own
// in one language. Kept apart from ImGui so it can be tested; the item browser
// of the item tools builds on it.
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

        bool operator==(const Filter& other) const = default;
    };

    // The item types; built again when the filter or the item database
    // changed, or after Refresh.
    const std::vector<int>& GetItemTypes(const Filter& filter);
    // After an edit made through the list: the rows stay as they are until the
    // filter changes or Refresh, so a filled name does not take its row away
    // while the list is being worked through.
    void KeepRowsAfterOwnEdit();
    void Refresh();

private:
    std::vector<int> m_itemTypes;
    Filter m_filter;
    int m_version = -1;
    bool m_built = false;
};

// Whether the item matches the search text (see CItemListModel::Filter).
bool MatchesItemSearch(const Data::Items::ItemDefinition& definition, std::string_view search);

// Whether the UTF-8 text contains the search; only the letters A to Z ignore
// case. The item searches of the editor share it.
bool ContainsIgnoringCase(std::string_view text, std::string_view search);

#endif // _EDITOR

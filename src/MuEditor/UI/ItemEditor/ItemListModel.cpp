#include "stdafx.h"

#ifdef _EDITOR

#include "ItemListModel.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemNames.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <algorithm>
#include <charconv>

namespace
{
// Only A to Z: std::tolower follows the process locale, which the client takes
// from the system; with a Windows code page it changes bytes of UTF-8 letters.
char ToLowerAscii(char character)
{
    return character >= 'A' && character <= 'Z' ? static_cast<char>(character - 'A' + 'a') : character;
}

bool ReadWholeNumber(std::string_view text, int& value)
{
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    return error == std::errc{} && end == text.data() + text.size();
}

// "group,number" or the item type.
bool MatchesItemNumber(const Data::Items::ItemDefinition& definition, std::string_view search)
{
    const size_t comma = search.find(',');
    if (comma != std::string_view::npos)
    {
        int group = 0;
        int number = 0;
        return ReadWholeNumber(search.substr(0, comma), group) && ReadWholeNumber(search.substr(comma + 1), number) &&
               group == definition.group && number == definition.number;
    }
    int itemType = 0;
    return ReadWholeNumber(search, itemType) &&
           itemType == Data::Items::MakeItemType(definition.group, definition.number);
}
} // namespace

bool ContainsIgnoringCase(std::string_view text, std::string_view search)
{
    const auto found = std::search(text.begin(), text.end(), search.begin(), search.end(),
                                   [](char a, char b) { return ToLowerAscii(a) == ToLowerAscii(b); });
    return found != text.end();
}

bool MatchesItemSearch(const Data::Items::ItemDefinition& definition, std::string_view search)
{
    if (search.empty() || MatchesItemNumber(definition, search))
    {
        return true;
    }
    if (ContainsIgnoringCase(definition.names.GetNeutral(), search))
    {
        return true;
    }
    const auto& translations = definition.names.GetTranslations();
    return std::any_of(translations.begin(), translations.end(),
                       [&](const auto& translation) { return ContainsIgnoringCase(translation.second, search); });
}

const std::vector<int>& CItemListModel::GetItemTypes(const Filter& filter)
{
    if (m_built && filter == m_filter && m_version == g_ItemDatabase.GetVersion())
    {
        return m_itemTypes;
    }

    m_itemTypes.clear();
    for (const Data::Items::ItemDefinition& definition : g_ItemDatabase.GetAllSlots())
    {
        if (!definition.Exists() || !MatchesItemSearch(definition, filter.search))
        {
            continue;
        }
        if (!filter.missingLocale.empty() && Data::Items::HasNameIn(definition, filter.missingLocale))
        {
            continue;
        }
        m_itemTypes.push_back(Data::Items::MakeItemType(definition.group, definition.number));
    }
    m_filter = filter;
    m_version = g_ItemDatabase.GetVersion();
    m_built = true;
    return m_itemTypes;
}

void CItemListModel::KeepRowsAfterOwnEdit()
{
    m_version = g_ItemDatabase.GetVersion();
}

void CItemListModel::Refresh()
{
    m_built = false;
}

#endif // _EDITOR

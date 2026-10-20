#include "stdafx.h"

#ifdef _EDITOR

#include "ItemListModel.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemNames.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/ItemData/ItemTypeUse.h"

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

std::vector<int> GetFreeItemNumbers(int group)
{
    std::vector<int> numbers;
    for (int number = 0; number < MAX_ITEM_INDEX; ++number)
    {
        if (Data::Items::IsValidItemId(group, number) &&
            Data::Items::IsFreeItemType(Data::Items::MakeItemType(group, number)))
        {
            numbers.push_back(number);
        }
    }
    return numbers;
}

int GetNextFreeItemNumber(int group, int from)
{
    const std::vector<int> free = GetFreeItemNumbers(group);
    const auto next = std::ranges::find_if(free, [from](int number) { return number >= from; });
    return next != free.end() ? *next : (free.empty() ? 0 : free.front());
}

bool CItemListModel::Lists(int itemType, const Filter& filter) const
{
    const Data::Items::ItemDefinition& slot = g_ItemDatabase.GetAllSlots()[itemType];
    if (filter.group && Data::Items::GetItemGroup(itemType) != *filter.group)
    {
        return false;
    }
    if (filter.modelsWithoutItem)
    {
        // Empty slots may not have their ids; the search goes by the number.
        Data::Items::ItemDefinition number;
        number.group = Data::Items::GetItemGroup(itemType);
        number.number = Data::Items::GetItemNumber(itemType);
        return !slot.Exists() && g_ItemModelDatabase.Find(itemType) != nullptr &&
               MatchesItemSearch(number, filter.search);
    }
    if (!slot.Exists() || !MatchesItemSearch(slot, filter.search))
    {
        return false;
    }
    if (!filter.missingLocale.empty() && Data::Items::HasNameIn(slot, filter.missingLocale))
    {
        return false;
    }
    if (filter.tag && !slot.tags.Has(*filter.tag))
    {
        return false;
    }
    if (filter.notAllowed && slot.IsAllowed(*filter.notAllowed))
    {
        return false;
    }
    return !filter.withoutModel || g_ItemModelDatabase.Find(itemType) == nullptr;
}

const std::vector<int>& CItemListModel::GetItemTypes(const Filter& filter)
{
    if (m_built && filter == m_filter && m_version == g_ItemDatabase.GetVersion() &&
        m_modelVersion == g_ItemModelDatabase.GetVersion())
    {
        return m_itemTypes;
    }

    m_itemTypes.clear();
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        if (Lists(itemType, filter))
        {
            m_itemTypes.push_back(itemType);
        }
    }
    m_filter = filter;
    m_version = g_ItemDatabase.GetVersion();
    m_modelVersion = g_ItemModelDatabase.GetVersion();
    m_built = true;
    return m_itemTypes;
}

void CItemListModel::KeepRowsAfterOwnEdit()
{
    m_version = g_ItemDatabase.GetVersion();
    m_modelVersion = g_ItemModelDatabase.GetVersion();
}

void CItemListModel::Refresh()
{
    m_built = false;
}

#endif // _EDITOR

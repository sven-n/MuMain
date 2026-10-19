#include "stdafx.h"

#include "ItemNames.h"

namespace Data::Items
{
bool HasNameIn(const ItemDefinition& definition, std::string_view locale)
{
    if (locale == LocalizedString::NeutralLocale)
    {
        return !definition.names.GetNeutral().empty();
    }
    return definition.names.GetTranslations().contains(locale);
}

std::vector<ItemNameCount> CountItemNames(std::span<const ItemDefinition> items, std::span<const char* const> locales)
{
    std::vector<ItemNameCount> counts;
    counts.reserve(locales.size());
    for (const char* locale : locales)
    {
        ItemNameCount count{locale};
        for (const ItemDefinition& definition : items)
        {
            if (!definition.Exists())
            {
                continue;
            }
            ++(HasNameIn(definition, locale) ? count.named : count.missing);
        }
        counts.push_back(std::move(count));
    }
    return counts;
}
} // namespace Data::Items

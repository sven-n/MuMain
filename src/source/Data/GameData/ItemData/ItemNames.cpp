#include "stdafx.h"

#include "ItemNames.h"

namespace Data::Items
{
const std::string& GetOwnName(const ItemDefinition& definition, std::string_view locale)
{
    return definition.names.GetOwn(locale);
}

bool HasNameIn(const ItemDefinition& definition, std::string_view locale)
{
    return definition.names.HasOwn(locale);
}

std::string_view GetItemNameLocale(std::string_view uiLocale, bool translateItemNames)
{
    return translateItemNames ? uiLocale : LocalizedString::NeutralLocale;
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

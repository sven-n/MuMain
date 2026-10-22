#include "stdafx.h"

#include "ItemNames.h"

namespace Data::Items
{
const std::string& GetOwnName(const ItemDefinition& definition, std::string_view locale)
{
    static const std::string none;
    if (locale == LocalizedString::NeutralLocale)
    {
        return definition.names.GetNeutral();
    }
    const auto& translations = definition.names.GetTranslations();
    const auto translation = translations.find(locale);
    return translation != translations.end() ? translation->second : none;
}

bool HasNameIn(const ItemDefinition& definition, std::string_view locale)
{
    // A translation is never stored empty (LocalizedString::Set removes it).
    return !GetOwnName(definition, locale).empty();
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

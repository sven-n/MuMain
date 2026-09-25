#include "stdafx.h"

#include "ItemDataValidation.h"
#include "ItemType.h"
#include "Core/Text/Utf8.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <string>

namespace Data::Items
{
namespace
{
constexpr const char* NameField = "name";

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const ItemDefinition& definition,
              const std::string& message)
{
    issues.push_back({severity, "", definition.group, definition.number, NameField, message});
}

bool ContainsSeparator(const std::string& text)
{
    return text.find(LocalizedString::Separator) != std::string::npos;
}

// Locale codes like "pt" or "zh-TW". Anything else (empty, "=", "||") would
// break the LocalizedString format used for the OpenMU exchange.
bool IsValidLocaleCode(const std::string& locale)
{
    return !locale.empty() && std::all_of(locale.begin(), locale.end(), [](char character) {
        return std::isalnum(static_cast<unsigned char>(character)) || character == '-';
    });
}

// The game shows names through the MAX_ITEM_NAME-sized ITEM_ATTRIBUTE name.
bool IsTooLongForGame(const std::string& text)
{
    return Core::Text::FromUtf8(text).size() > static_cast<size_t>(MAX_ITEM_NAME - 1);
}

void ValidateName(const ItemDefinition& definition, std::vector<ItemDataIssue>& issues)
{
    const LocalizedString& names = definition.names;
    if (names.GetNeutral().empty())
    {
        AddIssue(issues, ItemDataIssueSeverity::Error, definition, "the English name is missing");
    }

    const auto check = [&](const std::string& locale, const std::string& text) {
        if (ContainsSeparator(text))
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, definition,
                     "the " + locale + " name must not contain \"" + std::string(LocalizedString::Separator) + "\"");
        }
        if (IsTooLongForGame(text))
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                     "the " + locale + " name is longer than " + std::to_string(MAX_ITEM_NAME - 1) +
                         " characters and is cut in the game");
        }
    };

    check(std::string(LocalizedString::NeutralLocale), names.GetNeutral());
    for (const auto& [locale, text] : names.GetTranslations())
    {
        if (!IsValidLocaleCode(locale))
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, definition,
                     "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)");
            continue;
        }
        check(locale, text);
    }
}
} // namespace

void ValidateItems(std::span<const ItemDefinition> items, std::vector<ItemDataIssue>& issues)
{
    std::set<int> seenItemTypes;
    for (const ItemDefinition& definition : items)
    {
        if (!IsValidItemId(definition.group, definition.number))
        {
            issues.push_back({ItemDataIssueSeverity::Error, "", definition.group, definition.number, "number",
                              "the item id is out of range"});
            continue;
        }

        if (!seenItemTypes.insert(MakeItemType(definition.group, definition.number)).second)
        {
            issues.push_back({ItemDataIssueSeverity::Error, "", definition.group, definition.number, "number",
                              "the item is defined more than once"});
        }

        ValidateName(definition, issues);
    }
}
} // namespace Data::Items

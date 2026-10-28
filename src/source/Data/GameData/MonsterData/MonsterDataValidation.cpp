#include "stdafx.h"

#include "MonsterDataValidation.h"

#include "Core/Text/Utf8.h"

#include <algorithm>
#include <string>

namespace Data::Monsters
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const MonsterDefinition& monster,
              const std::string& message)
{
    issues.push_back({severity, "", ItemDataIssue::NoItem, monster.number,
                      "monster " + std::to_string(monster.number) + " name", message});
}

bool IsTooLongForGame(const std::string& text)
{
    std::string cut = text;
    return Core::Text::CutToUtf16Length(cut, MaxMonsterNameLength);
}

void ValidateName(const MonsterDefinition& monster, std::vector<ItemDataIssue>& issues)
{
    const LocalizedString& names = monster.names;
    if (names.GetNeutral().empty())
    {
        // A translation still shows in its own language (LocalizedString::Get).
        AddIssue(issues, ItemDataIssueSeverity::Warning, monster,
                 names.GetTranslations().empty()
                     ? "the English name is empty: the game shows no name"
                     : "the English name is empty: the game shows no name in a language without a translation");
    }

    const auto check = [&](const std::string& locale, const std::string& text)
    {
        if (LocalizedString::ContainsSeparator(text))
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, monster,
                     "the " + locale + " name must not contain \"" + std::string(LocalizedString::Separator) + "\"");
        }
        if (IsTooLongForGame(text))
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, monster,
                     "the " + locale + " name is longer than " + std::to_string(MaxMonsterNameLength) +
                         " characters and is cut in the game");
        }
    };

    check(std::string(LocalizedString::NeutralLocale), names.GetNeutral());
    for (const auto& [locale, text] : names.GetTranslations())
    {
        if (!LocalizedString::IsValidLocaleCode(locale))
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, monster,
                     "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)");
            continue;
        }
        check(locale, text);
    }
}
} // namespace

void ValidateMonsterNames(std::span<const MonsterDefinition> monsters, std::vector<ItemDataIssue>& issues)
{
    for (const MonsterDefinition& monster : monsters)
    {
        ValidateName(monster, issues);
    }
}

void ValidateMonsterNameLocales(std::span<const MonsterDefinition> monsters, std::span<const char* const> uiLocales,
                                std::vector<ItemDataIssue>& issues)
{
    const auto isShown = [&](const std::string& locale)
    {
        return std::any_of(uiLocales.begin(), uiLocales.end(),
                           [&](const char* uiLocale) { return locale == uiLocale; });
    };
    for (const MonsterDefinition& monster : monsters)
    {
        for (const auto& [locale, text] : monster.names.GetTranslations())
        {
            if (!isShown(locale))
            {
                AddIssue(issues, ItemDataIssueSeverity::Warning, monster,
                         "the " + locale + " name is never shown: the game has no texts in that language");
            }
        }
    }
}
} // namespace Data::Monsters

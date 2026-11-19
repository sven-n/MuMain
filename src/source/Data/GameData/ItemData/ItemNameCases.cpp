#include "stdafx.h"

#include "Data/GameData/ItemData/ItemNameCases.h"

#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/Translation/ItemReference.h"

#include <algorithm>

namespace Data::Items
{
namespace
{
using Json::OrderedJson;

constexpr int NameCasesFormatVersion = 1;
constexpr const char* LanguagesKey = "languages";

// The cases in the order of the case keys, each once.
std::vector<std::string> InCaseKeyOrder(const std::vector<std::string>& cases)
{
    std::vector<std::string> ordered;
    for (const std::string_view caseKey : Translation::ItemCaseKeys)
    {
        if (std::find(cases.begin(), cases.end(), caseKey) != cases.end())
        {
            ordered.emplace_back(caseKey);
        }
    }
    return ordered;
}

void ReadLanguage(const std::string& locale, const OrderedJson& json, const std::string& source, ItemNameCases& cases,
                  std::vector<ItemDataIssue>& issues)
{
    const std::string field = std::string(LanguagesKey) + "." + locale;
    if (!LocalizedString::IsValidLocaleCode(locale))
    {
        Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, field,
                           "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)");
        return;
    }
    if (locale == LocalizedString::NeutralLocale)
    {
        // The editor's Cases... lists no English to take it out again.
        issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field,
                          "English names have no forms: a text in English shows the name itself; leave English out"});
        return;
    }
    if (!json.is_array())
    {
        Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, field,
                           R"(must be a list of cases: ["gen", "dat", "acc", "ins", "loc"])");
        return;
    }
    std::vector<std::string> read;
    for (const OrderedJson& entry : json)
    {
        if (!entry.is_string() || !Translation::IsItemCaseKey(entry.get<std::string>()))
        {
            Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, field,
                               "has " + entry.dump() + ": a case is gen, dat, acc, ins or loc");
            continue;
        }
        read.push_back(entry.get<std::string>());
    }
    cases.SetCases(locale, std::move(read));
}
} // namespace

const std::vector<std::string>& ItemNameCases::GetCases(std::string_view locale) const
{
    static const std::vector<std::string> None;
    const auto found = m_languages.find(locale);
    return found != m_languages.end() ? found->second : None;
}

bool ItemNameCases::Uses(std::string_view locale, std::string_view caseKey) const
{
    const std::vector<std::string>& cases = GetCases(locale);
    return std::find(cases.begin(), cases.end(), caseKey) != cases.end();
}

void ItemNameCases::SetCases(std::string_view locale, std::vector<std::string> cases)
{
    std::vector<std::string> ordered = InCaseKeyOrder(cases);
    if (ordered.empty())
    {
        if (const auto found = m_languages.find(locale); found != m_languages.end())
        {
            m_languages.erase(found);
        }
        return;
    }
    m_languages[std::string(locale)] = std::move(ordered);
}

void ReadItemNameCasesJson(std::string_view text, const std::string& source, ItemNameCases& cases,
                           std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Json::ReadFileVersion(text, source, NameCasesFormatVersion, root, issues))
    {
        return;
    }
    const auto languages = root.find(LanguagesKey);
    if (languages == root.end() || !languages->is_object())
    {
        Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, LanguagesKey,
                           R"(must be an object of languages and their cases: {"pl": ["gen", "acc"]})");
        return;
    }
    for (const auto& [locale, list] : languages->items())
    {
        ReadLanguage(locale, list, source, cases, issues);
    }
}

std::string WriteItemNameCasesJson(const ItemNameCases& cases)
{
    OrderedJson languages = OrderedJson::object();
    for (const auto& [locale, list] : cases.GetLanguages())
    {
        languages[locale] = list;
    }
    OrderedJson root;
    root[Json::Keys::FormatVersion] = NameCasesFormatVersion;
    root[LanguagesKey] = std::move(languages);
    std::string text = root.dump(Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    for (const auto& [locale, list] : cases.GetLanguages())
    {
        text = Json::PutListsOnOneLine(text, locale);
    }
    return text + "\n";
}
} // namespace Data::Items

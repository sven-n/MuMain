#include "stdafx.h"

#include "OptionTextsJson.h"
#include "ItemDescriptionPlaceholders.h"
#include "ItemJsonCommon.h"

#include <algorithm>

namespace Data::Items
{
namespace
{
using Json::OrderedJson;

constexpr const char* TextsKey = "texts";

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& source,
              const std::string& field, const std::string& message)
{
    issues.push_back({severity, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
}

// The problems of a text, each a field and its message: no English, a language code that is none, a placeholder
// problem, or a placeholder other than {value}.
std::vector<std::pair<std::string, std::string>> CheckText(const LocalizedString& text, const std::string& field)
{
    if (text.GetNeutral().empty())
    {
        return {{field, "the English text is missing; the lines that name it are left out"}};
    }
    std::vector<std::pair<std::string, std::string>> problems;
    for (const auto& [locale, translation] : text.GetTranslations())
    {
        if (!LocalizedString::IsValidLocaleCode(locale))
        {
            problems.emplace_back(field, "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)");
        }
    }
    const std::vector<std::pair<std::string, std::string>> placeholders = CheckTextPlaceholders(text, field);
    problems.insert(problems.end(), placeholders.begin(), placeholders.end());
    const PlaceholderScan english = ScanPlaceholders(text.GetNeutral());
    for (const std::string& name : english.names)
    {
        if (name != OptionValuePlaceholder)
        {
            problems.emplace_back(field, "{" + name + "} is no placeholder of an option text; the one there is {" +
                                             std::string(OptionValuePlaceholder) + "}, the option's value");
            break;
        }
    }
    return problems;
}
} // namespace

void ReadOptionTextsJson(std::string_view text, const std::string& source, ItemOptionTexts& texts,
                         std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Json::ReadFileVersion(text, source, OptionTextsFormatVersion, root, issues))
    {
        return;
    }
    const auto list = root.find(TextsKey);
    if (list == root.end() || !list->is_object())
    {
        Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, TextsKey,
                           "missing or not an object of texts by name");
        return;
    }
    for (const std::string& name : Json::FindRepeatedKeys(text, TextsKey))
    {
        Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, std::string(TextsKey) + "." + name,
                           "is in the list more than once");
    }
    for (const auto& [name, json] : list->items())
    {
        const std::string field = std::string(TextsKey) + "." + name;
        if (!Json::IsName(name))
        {
            Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, field,
                               "a text's name has only letters and digits");
            continue;
        }
        LocalizedString entry;
        if (!Json::ReadLocalizedString(
                json, field, entry, [&](const std::string& errorField, const std::string& message)
                { Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, errorField, message); }))
        {
            continue;
        }
        const std::vector<std::pair<std::string, std::string>> problems = CheckText(entry, field);
        for (const auto& [problemField, message] : problems)
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, source, problemField, message);
        }
        if (problems.empty())
        {
            texts.texts.emplace(name, std::move(entry));
        }
    }
    Json::WarnAboutUnknownKeys(root, Json::KeySet{Json::Keys::FormatVersion, TextsKey}, "",
                               [&](const std::string& unknown, const std::string& message)
                               { AddIssue(issues, ItemDataIssueSeverity::Warning, source, unknown, message); });
}

std::string WriteOptionTextsJson(const ItemOptionTexts& texts)
{
    OrderedJson root;
    root[Json::Keys::FormatVersion] = OptionTextsFormatVersion;
    root[TextsKey] = OrderedJson::object();
    for (const auto& [name, text] : texts.texts)
    {
        root[TextsKey][name] = Json::WriteLocalizedString(text);
    }
    return root.dump(Json::Indent, ' ', false, OrderedJson::error_handler_t::replace) + "\n";
}

void ValidateOptionTextLocales(const ItemOptionTexts& texts, std::span<const char* const> uiLocales,
                               const std::string& source, std::vector<ItemDataIssue>& issues)
{
    for (const auto& [name, text] : texts.texts)
    {
        for (const auto& [locale, translation] : text.GetTranslations())
        {
            const bool shown = std::any_of(uiLocales.begin(), uiLocales.end(),
                                           [&](const char* uiLocale) { return locale == uiLocale; });
            if (!shown)
            {
                AddIssue(issues, ItemDataIssueSeverity::Warning, source, std::string(TextsKey) + "." + name,
                         "the " + locale + " text is never shown: the game has no texts in that language");
            }
        }
    }
}
} // namespace Data::Items

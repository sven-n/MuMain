#include "stdafx.h"

#include "Data/Translation/ItemReferenceChecks.h"

#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemReferenceNames.h"
#include "Data/Translation/ItemReference.h"
#include "I18N/All.h"

#include <set>
#include <tuple>

namespace Data::Translation
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::ItemReferenceProblem;

// An item and level a text names, whatever the case.
using NamedItem = std::tuple<int, int, int>;

std::set<NamedItem> NamedItems(std::string_view text)
{
    std::set<NamedItem> named;
    for (const ItemReferenceMatch& match : FindItemReferences(text))
    {
        if (match.reference)
        {
            named.emplace(match.reference->group, match.reference->number, match.reference->level.value_or(-1));
        }
    }
    return named;
}

std::string ItemNumber(const ItemReference& reference)
{
    return "(" + std::to_string(reference.group) + "," + std::to_string(reference.number) + ")";
}

// The message of a resolved reference's problem, or empty when the name shows as the text wants it.
std::string DescribeProblem(const Items::ItemDatabase& database, const ItemReference& reference,
                            std::string_view locale, ItemDataIssueSeverity& severity)
{
    const Items::ReferencedName name = Items::ResolveReferencedName(database, reference, locale);
    const std::string written = FormatItemReference(reference);
    switch (name.problem)
    {
    case ItemReferenceProblem::UnknownItem:
        severity = ItemDataIssueSeverity::Error;
        return written + " names no item: there is no item " + ItemNumber(reference) + "; the text shows it as written";
    case ItemReferenceProblem::UnfillableLine:
        severity = ItemDataIssueSeverity::Error;
        return written + ": the name of " + ItemNumber(reference) + " at +" + std::to_string(*reference.level) +
               " needs values only the game has (a skill's or monster's name, a bundle count); the text shows the "
               "reference as written";
    case ItemReferenceProblem::StandardName:
        severity = ItemDataIssueSeverity::Warning;
        return written + ": " + ItemNumber(reference) + " has no name line of its own at +" +
               std::to_string(*reference.level) + "; the text shows \"" + name.text + "\"";
    case ItemReferenceProblem::MissingForm:
        severity = ItemDataIssueSeverity::Warning;
        if (!database.GetNameCases().Uses(locale, reference.form))
        {
            return written + ": \"" + std::string(locale) + "\" does not decline names in the case " + reference.form +
                   " (NameCases.json); the text shows \"" + name.text + "\"";
        }
        return written + ": \"" + name.text + "\" " + ItemNumber(reference) + " has no " + std::string(locale) + " " +
               reference.form + " form; the text shows the name itself";
    case ItemReferenceProblem::None:
        break;
    }
    return {};
}

void AddIssue(const CheckedText& text, ItemDataIssueSeverity severity, int group, int number, std::string message,
              std::vector<ItemDataIssue>& issues)
{
    issues.push_back({severity, text.source, group, number, text.field, std::move(message)});
}

void CheckNamedItems(const CheckedText& text, std::string_view english, std::vector<ItemDataIssue>& issues)
{
    if (english.empty() || !HasItemReference(text.text))
    {
        return;
    }
    const std::set<NamedItem> named = NamedItems(text.text);
    if (named == NamedItems(english))
    {
        return;
    }
    // A UI text has no item of its own: the problem names the first item the translation names, so a click selects it.
    int group = text.group;
    int number = text.number;
    if (group == ItemDataIssue::NoItem && !named.empty())
    {
        group = std::get<0>(*named.begin());
        number = std::get<1>(*named.begin());
    }
    AddIssue(text, ItemDataIssueSeverity::Warning, group, number,
             "names other items than the English text: \"" + std::string(english) + "\"", issues);
}

std::string TextField(const std::string& lineField, std::string_view locale)
{
    std::string field = lineField + ".text";
    if (locale != LocalizedString::NeutralLocale)
    {
        field += "." + std::string(locale);
    }
    return field;
}

// Every language's text of an item's line, written in place (a shared text is checked with the tooltip file).
void CheckLine(const Items::ItemDatabase& database, const Items::ItemDefinition& definition,
               const Items::ItemDescriptionLine& line, const std::string& lineField, std::vector<ItemDataIssue>& issues)
{
    if (!line.textName.empty())
    {
        return;
    }
    const std::string& english = line.text.GetNeutral();
    CheckItemReferences(database,
                        {english, LocalizedString::NeutralLocale, "", definition.group, definition.number,
                         TextField(lineField, LocalizedString::NeutralLocale)},
                        {}, issues);
    for (const auto& [locale, translation] : line.text.GetTranslations())
    {
        CheckItemReferences(
            database, {translation, locale, "", definition.group, definition.number, TextField(lineField, locale)},
            english, issues);
    }
}

void CheckItemLines(const Items::ItemDatabase& database, const Items::ItemDefinition& definition,
                    std::vector<ItemDataIssue>& issues)
{
    for (size_t index = 0; index < definition.description.size(); ++index)
    {
        CheckLine(database, definition, definition.description[index], "description[" + std::to_string(index) + "]",
                  issues);
    }
    for (size_t index = 0; index < definition.nameLines.size(); ++index)
    {
        CheckLine(database, definition, definition.nameLines[index], "nameLines[" + std::to_string(index) + "]",
                  issues);
    }
    const auto& additions = definition.tooltipLayoutChanges.add;
    for (size_t addition = 0; addition < additions.size(); ++addition)
    {
        for (size_t index = 0; index < additions[addition].lines.size(); ++index)
        {
            CheckLine(database, definition, additions[addition].lines[index],
                      "tooltipLayoutChanges.add[" + std::to_string(addition) + "].lines[" + std::to_string(index) + "]",
                      issues);
        }
    }
}
} // namespace

void CheckItemReferences(const Items::ItemDatabase& database, const CheckedText& text, std::string_view english,
                         std::vector<ItemDataIssue>& issues)
{
    for (const ItemReferenceMatch& match : FindItemReferences(text.text))
    {
        if (!match.reference)
        {
            AddIssue(text, ItemDataIssueSeverity::Error, text.group, text.number,
                     "\"" + std::string(text.text.substr(match.position, match.length)) +
                         "\" is no item reference: write {item:group,number}, with \"+level\" for a level and "
                         "\":case\" for a form of the name",
                     issues);
            continue;
        }
        ItemDataIssueSeverity severity = ItemDataIssueSeverity::Warning;
        std::string message = DescribeProblem(database, *match.reference, text.locale, severity);
        if (message.empty())
        {
            continue;
        }
        // A UI text has no item of its own: its problems name the item it names, so a click selects that one.
        const bool ownItem = text.group != ItemDataIssue::NoItem;
        AddIssue(text, severity, ownItem ? text.group : match.reference->group,
                 ownItem ? text.number : match.reference->number, std::move(message), issues);
    }
    CheckNamedItems(text, english, issues);
}

void CheckUiItemTexts(const Items::ItemDatabase& database, std::vector<ItemDataIssue>& issues)
{
    const auto locales = I18N::GetAvailableLocales();
    for (const I18N::ItemText& itemText : I18N::GetItemTexts())
    {
        const std::string english = Core::Text::ToUtf8(itemText.values[0]);
        for (size_t index = 0; index < locales.size(); ++index)
        {
            if (((itemText.ownLocales >> index) & 1u) == 0)
            {
                continue;
            }
            const std::string_view locale = locales[index];
            const std::string text = index == 0 ? english : Core::Text::ToUtf8(itemText.values[index]);
            CheckedText checked{text,
                                locale,
                                std::string(itemText.group) + "." + std::string(locale) + ".resx",
                                ItemDataIssue::NoItem,
                                ItemDataIssue::NoItem,
                                itemText.key};
            CheckItemReferences(database, checked, index == 0 ? std::string_view() : std::string_view(english), issues);
        }
    }
}

std::vector<std::string> FindTextsNamingItem(const Items::ItemDatabase& database, int group, int number)
{
    const auto names = [&](std::string_view text)
    {
        for (const ItemReferenceMatch& match : FindItemReferences(text))
        {
            if (match.reference && match.reference->group == group && match.reference->number == number)
            {
                return true;
            }
        }
        return false;
    };
    const auto namesInSome = [&](const LocalizedString& text)
    {
        if (names(text.GetNeutral()))
        {
            return true;
        }
        for (const auto& [locale, translation] : text.GetTranslations())
        {
            if (names(translation))
            {
                return true;
            }
        }
        return false;
    };

    std::vector<std::string> found;
    const size_t localeCount = I18N::GetAvailableLocales().size();
    for (const I18N::ItemText& itemText : I18N::GetItemTexts())
    {
        for (size_t index = 0; index < localeCount; ++index)
        {
            if (((itemText.ownLocales >> index) & 1u) != 0 && names(Core::Text::ToUtf8(itemText.values[index])))
            {
                found.push_back(std::string(itemText.group) + ": " + itemText.key);
                break;
            }
        }
    }
    for (const Items::ItemDefinition& definition : database.GetAllSlots())
    {
        if (!definition.Exists())
        {
            continue;
        }
        const std::string owner = database.GetLogName(Items::MakeItemType(definition.group, definition.number));
        for (size_t index = 0; index < definition.description.size(); ++index)
        {
            if (namesInSome(definition.description[index].text))
            {
                found.push_back(owner + ": description[" + std::to_string(index) + "]");
            }
        }
        for (size_t index = 0; index < definition.nameLines.size(); ++index)
        {
            if (namesInSome(definition.nameLines[index].text))
            {
                found.push_back(owner + ": nameLines[" + std::to_string(index) + "]");
            }
        }
        const auto& additions = definition.tooltipLayoutChanges.add;
        for (size_t addition = 0; addition < additions.size(); ++addition)
        {
            for (size_t index = 0; index < additions[addition].lines.size(); ++index)
            {
                if (namesInSome(additions[addition].lines[index].text))
                {
                    found.push_back(owner + ": tooltipLayoutChanges.add[" + std::to_string(addition) + "].lines[" +
                                    std::to_string(index) + "]");
                }
            }
        }
    }
    for (const auto& [name, text] : database.GetTooltipFile().texts)
    {
        if (namesInSome(text))
        {
            found.push_back("Tooltip.json: texts." + name);
        }
    }
    return found;
}

void CheckItemDataTexts(const Items::ItemDatabase& database, const std::string& tooltipFile,
                        std::vector<ItemDataIssue>& issues)
{
    for (const Items::ItemDefinition& definition : database.GetAllSlots())
    {
        if (definition.Exists())
        {
            CheckItemLines(database, definition, issues);
        }
    }
    for (const auto& [name, text] : database.GetTooltipFile().texts)
    {
        const std::string field = "texts." + name;
        const std::string& english = text.GetNeutral();
        CheckItemReferences(
            database,
            {english, LocalizedString::NeutralLocale, tooltipFile, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field},
            {}, issues);
        for (const auto& [locale, translation] : text.GetTranslations())
        {
            CheckItemReferences(
                database,
                {translation, locale, tooltipFile, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field + "." + locale},
                english, issues);
        }
    }
}
} // namespace Data::Translation

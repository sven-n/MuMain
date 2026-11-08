#include "stdafx.h"

#include "Data/Translation/ItemReferenceChecks.h"

#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemReferenceNames.h"
#include "Data/GameData/ItemData/ItemTooltipLines.h"
#include "Data/GameData/ItemData/ItemType.h"
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

// Why the reference's case has no form: the language doesn't decline names in it, or the item has no form for it.
std::string MissingFormReason(const Items::ItemDatabase& database, const ItemReference& reference,
                              std::string_view locale)
{
    if (!database.GetNameCases().Uses(locale, reference.form))
    {
        return "\"" + std::string(locale) + "\" does not decline names in the case " + reference.form +
               " (NameCases.json)";
    }
    return ItemNumber(reference) + " has no " + std::string(locale) + " " + reference.form + " form";
}

// A level without a name of its own shows the standard name, with the item's form when the reference asks for a case:
// both can be missing.
std::string DescribeStandardName(const Items::ItemDatabase& database, const ItemReference& reference,
                                 std::string_view locale, const std::string& shown)
{
    std::string message = FormatItemReference(reference) + ": " + ItemNumber(reference) +
                          " has no name of its own at +" + std::to_string(*reference.level) +
                          " (no name line, or one with an empty text); the text shows \"" + shown + "\"";
    const ItemReference itemName{reference.group, reference.number, std::nullopt, reference.form};
    if (!reference.form.empty() &&
        Items::ResolveReferencedName(database, itemName, locale).problem == ItemReferenceProblem::MissingForm)
    {
        message += ", and " + MissingFormReason(database, reference, locale) + ", so the name itself stands in";
    }
    return message;
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
    case ItemReferenceProblem::NoSuchLevel:
        severity = ItemDataIssueSeverity::Error;
        return written + ": no item has level +" + std::to_string(*reference.level) + " (the levels are +0 to +" +
               std::to_string(Items::ItemLevelCount - 1) + "); the text shows the reference as written";
    case ItemReferenceProblem::UnfillableLine:
        severity = ItemDataIssueSeverity::Error;
        return written + ": the name of " + ItemNumber(reference) + " at +" + std::to_string(*reference.level) +
               " needs values only the game has (a skill's or monster's name, a bundle count); the text shows the "
               "reference as written";
    case ItemReferenceProblem::StandardName:
        severity = ItemDataIssueSeverity::Warning;
        return DescribeStandardName(database, reference, locale, name.text);
    case ItemReferenceProblem::MissingForm:
        severity = ItemDataIssueSeverity::Warning;
        return written + ": " + MissingFormReason(database, reference, locale) + "; the text shows \"" + name.text +
               "\", the name itself";
    case ItemReferenceProblem::UnshowableName:
        severity = ItemDataIssueSeverity::Error;
        return written + ": the name of " + ItemNumber(reference) +
               " has a '%', '{' or '}', which the text would read as its own (a printf format, a placeholder); the "
               "text shows the reference as written";
    case ItemReferenceProblem::UndeclinedLine:
        severity = ItemDataIssueSeverity::Warning;
        return written + ": the name of " + ItemNumber(reference) + " at +" + std::to_string(*reference.level) +
               " is its name line \"" + name.text + "\", whose placeholders leave out {name}, so it has no " +
               reference.form + " form; the text shows the line as it is";
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
                         Items::LineTextField(lineField, LocalizedString::NeutralLocale)},
                        {}, issues);
    for (const auto& [locale, translation] : line.text.GetTranslations())
    {
        CheckItemReferences(
            database,
            {translation, locale, "", definition.group, definition.number, Items::LineTextField(lineField, locale)},
            english, issues);
    }
}

// Every language's text of a text of the tooltip file at `field` ("texts.talisman", "layouts[0].lines[3].text"), whose
// translations are at "<field>.<locale>".
void CheckTooltipFileText(const Items::ItemDatabase& database, const LocalizedString& text,
                          const std::string& tooltipFile, const std::string& field, std::vector<ItemDataIssue>& issues)
{
    const std::string& english = text.GetNeutral();
    CheckItemReferences(
        database,
        {english, LocalizedString::NeutralLocale, tooltipFile, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field}, {},
        issues);
    for (const auto& [locale, translation] : text.GetTranslations())
    {
        CheckItemReferences(
            database,
            {translation, locale, tooltipFile, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field + "." + locale},
            english, issues);
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
        // The names and fields only for an item whose lines name it: most name none.
        bool namesIt = false;
        Items::ForEachItemTooltipLine(definition, [&](const Items::ItemDescriptionLine& line)
                                      { namesIt = namesIt || namesInSome(line.text); });
        if (!namesIt)
        {
            continue;
        }
        const std::string owner = database.GetLogName(Items::MakeItemType(definition.group, definition.number));
        for (const auto& [line, field] : Items::TooltipLinesOf(definition))
        {
            if (namesInSome(line->text))
            {
                found.push_back(owner + ": " + field);
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
    for (const auto& [line, field] : Items::TooltipFileLinesOf(database.GetTooltipFile()))
    {
        if (line->textName.empty() && namesInSome(line->text))
        {
            found.push_back("Tooltip.json: " + field);
        }
    }
    return found;
}

void CheckItemDataTexts(const Items::ItemDatabase& database, const std::string& tooltipFile,
                        std::vector<ItemDataIssue>& issues)
{
    for (const Items::ItemDefinition& definition : database.GetAllSlots())
    {
        if (!definition.Exists())
        {
            continue;
        }
        for (const auto& [line, field] : Items::TooltipLinesOf(definition))
        {
            CheckLine(database, definition, *line, field, issues);
        }
    }
    for (const auto& [name, text] : database.GetTooltipFile().texts)
    {
        CheckTooltipFileText(database, text, tooltipFile, "texts." + name, issues);
    }
    for (const auto& [line, field] : Items::TooltipFileLinesOf(database.GetTooltipFile()))
    {
        // A shared text is checked above.
        if (line->textName.empty())
        {
            CheckTooltipFileText(database, line->text, tooltipFile, field + ".text", issues);
        }
    }
}
} // namespace Data::Translation

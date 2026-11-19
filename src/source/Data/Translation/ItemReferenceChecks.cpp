#include "stdafx.h"

#include "Data/Translation/ItemReferenceChecks.h"

#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemReferenceNames.h"
#include "Data/GameData/ItemData/ItemTooltipLines.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/Translation/ItemReference.h"
#include "I18N/All.h"

#include <algorithm>
#include <iterator>
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
                            std::string_view locale, bool uiText, ItemDataIssueSeverity& severity)
{
    const Items::ReferencedName name = uiText ? Items::ResolveUiTextName(database, reference, locale)
                                              : Items::ResolveReferencedName(database, reference, locale);
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
    case ItemReferenceProblem::LineNamesItself:
        severity = ItemDataIssueSeverity::Error;
        return written + ": the name line of " + ItemNumber(reference) + " at +" + std::to_string(*reference.level) +
               " names its own level through name lines; the text shows the reference as written";
    case ItemReferenceProblem::MissingLineText:
        severity = ItemDataIssueSeverity::Error;
        return written + ": the name line of " + ItemNumber(reference) + " at +" + std::to_string(*reference.level) +
               " has a shared text that Tooltip.json doesn't have; the text shows the reference as written";
    case ItemReferenceProblem::LineNamesNoItem:
        severity = ItemDataIssueSeverity::Error;
        return written + ": the name line of " + ItemNumber(reference) + " at +" + std::to_string(*reference.level) +
               " names an item or a level no item has; the text shows the reference as written";
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
               " has a '%', which this text, a printf format, would read as a conversion; the text shows the "
               "reference as written";
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

// Below this a byte, and a character, is ASCII.
constexpr char32_t FirstNonAscii = 0x80;

// UTF-8: the bytes that continue a character are 10xxxxxx and carry 6 bits each; an ASCII byte is a character of its
// own, a lead byte below 0xE0 (from 0xC0) starts 2 bytes, below 0xF0 3, else 4.
constexpr unsigned char ContinuationMask = 0xC0;
constexpr unsigned char ContinuationBits = 0x80;
constexpr unsigned char ContinuationPayload = 0x3F;
constexpr int ContinuationPayloadBits = 6;
constexpr unsigned char FirstThreeByteLead = 0xE0;
constexpr unsigned char FirstFourByteLead = 0xF0;
constexpr unsigned char LeadPayload = 0x7F;

// The letters a word goes on through outside ASCII: Latin (Latin-1 Supplement to Latin Extended-B, without the signs
// "×" and "÷"; Latin Extended Additional), Greek and Cyrillic (to Cyrillic Supplement).
struct CharacterRange
{
    char32_t first;
    char32_t last;
};
constexpr CharacterRange WordLetterRanges[] = {{0xC0, 0x24F}, {0x370, 0x52F}, {0x1E00, 0x1EFF}};
constexpr char32_t MultiplicationSign = 0xD7;
constexpr char32_t DivisionSign = 0xF7;

bool IsContinuationByte(char byte)
{
    return (static_cast<unsigned char>(byte) & ContinuationMask) == ContinuationBits;
}

// The character of UTF-8 `text` that starts at `at` (a lead byte), or that ends right before it when `before`; 0 at the
// text's ends.
char32_t CharacterAt(std::string_view text, size_t at, bool before)
{
    if (before)
    {
        if (at == 0)
        {
            return 0;
        }
        // Back over the continuation bytes to the character's lead byte.
        do
        {
            --at;
        } while (at > 0 && IsContinuationByte(text[at]));
    }
    if (at >= text.size())
    {
        return 0;
    }
    const auto lead = static_cast<unsigned char>(text[at]);
    const int length = lead < FirstNonAscii ? 1 : lead < FirstThreeByteLead ? 2 : lead < FirstFourByteLead ? 3 : 4;
    char32_t character = length == 1 ? lead : lead & (LeadPayload >> length);
    for (int i = 1; i < length && at + i < text.size(); ++i)
    {
        character =
            (character << ContinuationPayloadBits) | (static_cast<unsigned char>(text[at + i]) & ContinuationPayload);
    }
    return character;
}

// Whether a word goes on through the character: an ASCII letter or digit, or a letter of the Latin, Greek or Cyrillic
// scripts. Other characters (spaces, punctuation, quotes such as "„" or "«", Chinese and Japanese, which write no
// spaces) end a word.
bool ContinuesWord(char32_t character)
{
    if (character < FirstNonAscii)
    {
        return (character >= U'0' && character <= U'9') || (character >= U'A' && character <= U'Z') ||
               (character >= U'a' && character <= U'z');
    }
    if (character == MultiplicationSign || character == DivisionSign)
    {
        return false;
    }
    return std::any_of(std::begin(WordLetterRanges), std::end(WordLetterRanges),
                       [character](const CharacterRange& range)
                       { return character >= range.first && character <= range.last; });
}

// Whether `text` has `name` as words of its own, not inside a longer word ("Rena" in "Arena").
bool HasWords(std::string_view text, std::string_view name)
{
    if (name.empty())
    {
        return false;
    }
    for (size_t at = text.find(name); at != std::string_view::npos; at = text.find(name, at + 1))
    {
        if (!ContinuesWord(CharacterAt(text, at, true)) && !ContinuesWord(CharacterAt(text, at + name.size(), false)))
        {
            return true;
        }
    }
    return false;
}

// The name of the reference that `text` spells: the name it shows in the text's language, or its form in a case the
// language declines names in (the item's form, or at a level the name line's); empty when the text has none of them.
std::string SpelledName(const Items::ItemDatabase& database, const ItemReference& reference, const CheckedText& text)
{
    const std::string name = Items::ResolveReferencedName(database, reference, text.locale).text;
    if (HasWords(text.text, name))
    {
        return name;
    }
    for (const std::string& caseKey : database.GetNameCases().GetCases(text.locale))
    {
        ItemReference inCase = reference;
        inCase.form = caseKey;
        const std::string form = Items::ResolveReferencedName(database, inCase, text.locale).text;
        if (HasWords(text.text, form))
        {
            return form;
        }
    }
    return {};
}

// A translation that spells the names its English text writes by number: a warning when the translation has the
// names the references would show (the items' names, or a level's name line, in its language), so references would
// show the same and follow the item data. Languages without item names yet (Russian, Japanese, ...) spell them on
// purpose, and a translation worded without the names has nothing to name.
void CheckSpelledNames(const Items::ItemDatabase& database, const CheckedText& text, std::string_view english,
                       std::vector<ItemDataIssue>& issues)
{
    const std::set<NamedItem> named = NamedItems(english);
    if (named.empty())
    {
        return;
    }
    std::string spelled;
    for (const auto& [group, number, level] : named)
    {
        const Items::ItemDefinition* definition = database.Find(group, number);
        if (definition == nullptr || !definition->names.HasOwn(text.locale))
        {
            return;
        }
        const ItemReference reference{group, number, level < 0 ? std::nullopt : std::optional<int>(level), {}};
        const std::string name = SpelledName(database, reference, text);
        if (name.empty())
        {
            return;
        }
        spelled += (spelled.empty() ? "\"" : ", \"") + name + "\"";
    }
    // A UI text has no item of its own: the problem names the first item, so a click selects it.
    const bool ownItem = text.group != ItemDataIssue::NoItem;
    const NamedItem& first = *named.begin();
    AddIssue(text, ItemDataIssueSeverity::Warning, ownItem ? text.group : std::get<0>(first),
             ownItem ? text.number : std::get<1>(first),
             "spells " + spelled + ", which the English text names by number (\"" + std::string(english) +
                 "\"): write the same references, so the names follow the item data and Translate item names",
             issues);
}

void CheckNamedItems(const Items::ItemDatabase& database, const CheckedText& text, std::string_view english,
                     std::vector<ItemDataIssue>& issues)
{
    if (english.empty())
    {
        return;
    }
    if (!HasItemReference(text.text))
    {
        CheckSpelledNames(database, text, english, issues);
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
        std::string message = DescribeProblem(database, *match.reference, text.locale, text.uiText, severity);
        if (message.empty())
        {
            continue;
        }
        // A UI text has no item of its own: its problems name the item it names, so a click selects that one.
        const bool ownItem = text.group != ItemDataIssue::NoItem;
        AddIssue(text, severity, ownItem ? text.group : match.reference->group,
                 ownItem ? text.number : match.reference->number, std::move(message), issues);
    }
    CheckNamedItems(database, text, english, issues);
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
                                itemText.key,
                                true};
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

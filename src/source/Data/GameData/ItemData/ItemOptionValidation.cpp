#include "stdafx.h"

#include "ItemOptionValidation.h"
#include "ItemDescriptionPlaceholders.h"
#include "ItemEnumNames.h"
#include "ItemOptionEffects.h"
#include "OptionTextsJson.h"

#include <algorithm>
#include <map>
#include <set>

namespace Data::Items
{
namespace
{
constexpr const char* OptionsField = "options";

// A definition's problems name its file in Data/Items/Options and its place there.
std::string DefinitionField(std::span<const ItemOptionDefinition> definitions, size_t index)
{
    const std::string& file = definitions[index].file;
    size_t place = 0;
    for (size_t before = 0; before < index; ++before)
    {
        place += definitions[before].file == file ? 1 : 0;
    }
    return "definitions[" + std::to_string(place) + "]";
}

std::string DefinitionSource(const ItemOptionDefinition& definition)
{
    return "Options/" + definition.file;
}

void ValidateNames(std::span<const ItemOptionDefinition> definitions, std::vector<ItemDataIssue>& issues)
{
    std::map<std::string, const ItemOptionDefinition*, std::less<>> first;
    for (size_t index = 0; index < definitions.size(); ++index)
    {
        const ItemOptionDefinition& definition = definitions[index];
        const auto [found, added] = first.emplace(definition.name, &definition);
        if (!added)
        {
            issues.push_back({ItemDataIssueSeverity::Fatal, DefinitionSource(definition), ItemDataIssue::NoItem,
                              ItemDataIssue::NoItem, DefinitionField(definitions, index) + ".name",
                              "\"" + definition.name + "\" is the name of a definition before it (" +
                                  DefinitionSource(*found->second) + "): items link a definition by its name"});
        }
    }
}

void ValidateOptions(std::span<const ItemOptionDefinition> definitions, const ItemOptionTexts& texts,
                     std::set<std::string, std::less<>>& namedTexts, std::vector<ItemDataIssue>& issues)
{
    for (size_t index = 0; index < definitions.size(); ++index)
    {
        const ItemOptionDefinition& definition = definitions[index];
        for (size_t optionIndex = 0; optionIndex < definition.options.size(); ++optionIndex)
        {
            const ItemOption& option = definition.options[optionIndex];
            const std::string field =
                DefinitionField(definitions, index) + ".options[" + std::to_string(optionIndex) + "]";
            if (!FindItemOptionEffect(definition.type, option.attribute, option.add))
            {
                issues.push_back({ItemDataIssueSeverity::Warning, DefinitionSource(definition), ItemDataIssue::NoItem,
                                  ItemDataIssue::NoItem, field + ".attribute",
                                  "the client computes no " + option.attribute + " for a definition of type " +
                                      FindEnumName(definition.type) + " this way: its line is shown, it adds nothing"});
            }
            for (size_t term = 0; term < option.value.per.size(); ++term)
            {
                const std::string& attribute = option.value.per[term].attribute;
                if (FindItemOptionTermAttribute(attribute) == ItemOptionTermAttribute::Unread)
                {
                    issues.push_back({ItemDataIssueSeverity::Warning, DefinitionSource(definition),
                                      ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                                      field + ".per[" + std::to_string(term) + "].attribute",
                                      "the client reads no hero attribute " + attribute + ": the term adds nothing"});
                }
            }
            const auto divisorTerms = std::count_if(option.value.per.begin(), option.value.per.end(),
                                                    [](const ItemOptionTerm& term) { return term.divisor > 0; });
            for (const std::string& text : option.texts)
            {
                namedTexts.insert(text);
                const LocalizedString* found = texts.Find(text);
                if (found == nullptr)
                {
                    issues.push_back({ItemDataIssueSeverity::Error, DefinitionSource(definition), ItemDataIssue::NoItem,
                                      ItemDataIssue::NoItem, field + ".text",
                                      "\"" + text + "\" is no text of OptionTexts.json: its line is left out"});
                    continue;
                }
                const std::vector<std::string> names = ScanPlaceholders(found->GetNeutral()).names;
                if (divisorTerms != 1 && std::find(names.begin(), names.end(), OptionDivisorPlaceholder) != names.end())
                {
                    const std::string problem =
                        divisorTerms == 0 ? std::string("no term with a divisor: the line shows 0 there")
                                          : std::to_string(divisorTerms) + " terms with a divisor: the line shows the "
                                                                           "first one's";
                    issues.push_back({ItemDataIssueSeverity::Warning, DefinitionSource(definition),
                                      ItemDataIssue::NoItem, ItemDataIssue::NoItem, field + ".text",
                                      "\"" + text + "\" shows {" + std::string(OptionDivisorPlaceholder) +
                                          "}, but the option has " + problem});
                }
            }
        }
    }
}
} // namespace

void ValidateItemOptions(std::span<const ItemOptionDefinition> definitions, const ItemOptionTexts& texts,
                         const std::string& textsSource, std::vector<ItemDataIssue>& issues)
{
    ValidateNames(definitions, issues);
    std::set<std::string, std::less<>> namedTexts;
    ValidateOptions(definitions, texts, namedTexts, issues);
    for (const auto& [name, text] : texts.texts)
    {
        if (!namedTexts.contains(name))
        {
            issues.push_back({ItemDataIssueSeverity::Warning, textsSource, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                              "texts." + name, "no option names this text"});
        }
    }
}

void ValidateOptionRules(std::span<const OptionRule> rules, std::span<const ItemOptionDefinition> definitions,
                         const std::string& source, std::vector<ItemDataIssue>& issues)
{
    for (size_t index = 0; index < rules.size(); ++index)
    {
        for (const std::string& name : rules[index].options)
        {
            const bool known =
                std::any_of(definitions.begin(), definitions.end(),
                            [&](const ItemOptionDefinition& definition) { return definition.name == name; });
            if (!known)
            {
                issues.push_back({ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                                  "rules[" + std::to_string(index) + "].options",
                                  "\"" + name + "\" is no option definition of Data/Items/Options"});
            }
        }
    }
}

void ValidateItemOptionLinks(std::span<const ItemDefinition> items, const ItemOptionDatabase& options,
                             std::vector<ItemDataIssue>& issues)
{
    for (const ItemDefinition& item : items)
    {
        // A deleted item keeps its fields in the item database, but is not saved.
        if (!item.Exists())
        {
            continue;
        }
        size_t known = 0;
        for (const std::string& name : item.options)
        {
            if (options.Find(name) == ItemOptionDatabase::NoDefinition)
            {
                issues.push_back(
                    {ItemDataIssueSeverity::Error, "", item.group, item.number, OptionsField,
                     "\"" + name + "\" is no option definition of Data/Items/Options: the link is left out"});
                continue;
            }
            ++known;
        }
        const std::vector<ItemOptionIndex> links = options.ResolveLinks(item.options);
        if (links.size() < known)
        {
            issues.push_back({ItemDataIssueSeverity::Error, "", item.group, item.number, OptionsField,
                              "links two definitions whose options the same part of the server's item data turns on "
                              "(of one type): the later one is left out"});
        }
        // The excellent byte of wings holds their wing options, the Horn of Fenrir's its kind; no horn is excellent.
        const bool byteMeansOther = item.slot == ItemSlot::Wings || item.tags.Has(ItemTag::HornMount);
        const bool linksExcellent = std::any_of(links.begin(), links.end(), [&options](ItemOptionIndex link)
                                                { return options.Get(link).type == ItemOptionType::Excellent; });
        if (byteMeansOther && linksExcellent)
        {
            issues.push_back(
                {ItemDataIssueSeverity::Error, "", item.group, item.number, OptionsField,
                 "links excellent options, but its excellent byte holds its wing options or a horn's kind, "
                 "or nothing: no horn is excellent"});
        }
    }
}

void ValidateOptionDefinitionUse(std::span<const ItemDefinition> items, const ItemOptionDatabase& options,
                                 std::vector<ItemDataIssue>& issues)
{
    std::set<std::string, std::less<>> linked;
    for (const OptionRule& rule : options.GetRules())
    {
        linked.insert(rule.options.begin(), rule.options.end());
    }
    for (const ItemDefinition& item : items)
    {
        if (item.Exists())
        {
            linked.insert(item.options.begin(), item.options.end());
        }
    }
    const std::span<const ItemOptionDefinition> definitions = options.GetDefinitions();
    for (size_t index = 0; index < definitions.size(); ++index)
    {
        if (!linked.contains(definitions[index].name))
        {
            issues.push_back({ItemDataIssueSeverity::Warning, DefinitionSource(definitions[index]),
                              ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                              DefinitionField(definitions, index) + ".name",
                              "no item and no option rule links \"" + definitions[index].name + "\""});
        }
    }
}
} // namespace Data::Items

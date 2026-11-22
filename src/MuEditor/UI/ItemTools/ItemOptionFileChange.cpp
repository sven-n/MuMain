#include "stdafx.h"

#ifdef _EDITOR

#include "ItemOptionFileChange.h"

#include "../MuEditor/UI/Common/TextSearch.h"

#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/ItemData/OptionRule.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <functional>
#include <map>

using Data::Items::ItemDefinition;
using Data::Items::ItemOptionDefinition;
using Data::Items::ItemOptionFiles;
using Data::Items::ItemOptionValue;
using Data::Items::OptionRule;

namespace MuEditor::Items
{
namespace
{
// The item links the definition, or picks it by its attackType (of FindAncientBonusPicks).
bool Links(const ItemDefinition& item, const AncientBonusPicks& picks, std::string_view name)
{
    return item.Exists() &&
           (std::ranges::find(item.options, name) != item.options.end() || FindAncientBonusPick(item, picks) == name);
}

bool NamesChangedText(const ItemOptionDefinition& definition, const ItemOptionFiles& before,
                      const ItemOptionFiles& after)
{
    for (const Data::Items::ItemOption& option : definition.options)
    {
        for (const std::string& text : option.texts)
        {
            const Data::LocalizedString* was = before.texts.Find(text);
            const Data::LocalizedString* is = after.texts.Find(text);
            if ((was == nullptr) != (is == nullptr) || (was != nullptr && *was != *is))
            {
                return true;
            }
        }
    }
    return false;
}
} // namespace

std::string FindFileOfType(const ItemOptionFiles& files, Data::Items::ItemOptionType type)
{
    const auto isAncient = [](Data::Items::ItemOptionType each)
    { return each == Data::Items::ItemOptionType::AncientOption || each == Data::Items::ItemOptionType::AncientBonus; };
    std::map<std::string, int, std::less<>> counts;
    for (const ItemOptionDefinition& definition : files.definitions)
    {
        if (definition.type == type || (isAncient(definition.type) && isAncient(type)))
        {
            ++counts[definition.file];
        }
    }
    // The shipped files' names: the ancient types share Ancient.json, the wing options are in Wings.json.
    const char* name = isAncient(type)                             ? "ancient"
                       : type == Data::Items::ItemOptionType::Wing ? "wings"
                                                                   : Data::Items::FindEnumName(type);
    std::string typeFile = name != nullptr ? name : "options";
    typeFile.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(typeFile.front())));
    typeFile += ".json";
    if (counts.contains(typeFile))
    {
        return typeFile;
    }
    const auto most = std::ranges::max_element(counts, {}, [](const auto& entry) { return entry.second; });
    return most != counts.end() ? most->first : typeFile;
}

const ItemOptionDefinition* FindDefinitionNamed(const ItemOptionFiles& files, std::string_view name)
{
    const auto found = std::ranges::find(files.definitions, name, &ItemOptionDefinition::name);
    return found != files.definitions.end() ? &*found : nullptr;
}

AncientBonusPicks FindAncientBonusPicks(const ItemOptionFiles& files)
{
    AncientBonusPicks picks{};
    for (const ItemOptionDefinition& definition : files.definitions)
    {
        const auto name = std::ranges::find(Data::Items::AncientBonusNames, definition.name);
        if (name != Data::Items::AncientBonusNames.end() && IsPickedByName(definition))
        {
            picks[static_cast<size_t>(name - Data::Items::AncientBonusNames.begin())] = *name;
        }
    }
    return picks;
}

bool IsOnlyInCopy(const ItemOptionFiles& stored, std::string_view name)
{
    return FindDefinitionNamed(stored, name) == nullptr;
}

AncientBonusPicks MergeCopyPicks(const ItemOptionFiles& stored, const ItemOptionFiles& edit)
{
    AncientBonusPicks picks = FindAncientBonusPicks(stored);
    const AncientBonusPicks copyPicks = FindAncientBonusPicks(edit);
    for (size_t index = 0; index < picks.size(); ++index)
    {
        if (picks[index].empty() && !copyPicks[index].empty() && IsOnlyInCopy(stored, copyPicks[index]))
        {
            picks[index] = copyPicks[index];
        }
    }
    return picks;
}

std::map<std::string, int, std::less<>> CountOptionUsers(std::span<const ItemDefinition> slots,
                                                         const AncientBonusPicks& picks)
{
    std::map<std::string, int, std::less<>> counts;
    for (const ItemDefinition& item : slots)
    {
        if (!item.Exists())
        {
            continue;
        }
        for (const std::string& name : item.options)
        {
            ++counts[name];
        }
        if (const std::string_view picked = FindAncientBonusPick(item, picks); !picked.empty())
        {
            ++counts[std::string(picked)];
        }
    }
    return counts;
}

std::string_view FindAncientBonusPick(const ItemDefinition& item, const AncientBonusPicks& picks)
{
    const size_t attackType = item.attackType;
    return attackType >= 1 && attackType <= picks.size() ? picks[attackType - 1] : std::string_view();
}

std::vector<const ItemOptionDefinition*> ListDefinitions(const ItemOptionFiles& stored, const ItemOptionFiles& edit,
                                                         int typeFilter, std::string_view search)
{
    const auto shown = [&](const ItemOptionDefinition& definition)
    {
        return (typeFilter < 0 || static_cast<int>(definition.type) == typeFilter) &&
               (search.empty() || MuEditor::TextSearch::ContainsIgnoringCase(definition.name, search) ||
                MuEditor::TextSearch::ContainsIgnoringCase(definition.displayName.Get("en"), search));
    };
    std::vector<const ItemOptionDefinition*> listed;
    for (const ItemOptionDefinition& definition : stored.definitions)
    {
        if (shown(definition))
        {
            listed.push_back(&definition);
        }
    }
    for (const ItemOptionDefinition& definition : edit.definitions)
    {
        if (shown(definition) && IsOnlyInCopy(stored, definition.name))
        {
            listed.push_back(&definition);
        }
    }
    return listed;
}

void DropMissingMarks(std::set<std::string, std::less<>>& marked, const ItemOptionFiles& stored,
                      const ItemOptionFiles& edit)
{
    std::erase_if(
        marked, [&](const std::string& name)
        { return FindDefinitionNamed(stored, name) == nullptr && FindDefinitionNamed(edit, name) == nullptr; });
}

bool IsUsefulOptionLink(const ItemDefinition& item, const ItemOptionDefinition& definition,
                        const Data::Items::ItemOptionDatabase& options)
{
    using Data::Items::ItemOptionType;
    if (definition.type == ItemOptionType::AncientOption || definition.type == ItemOptionType::AncientBonus)
    {
        return false;
    }
    // The excellent byte holds wing options on wings, excellent options on every other item but a horn (its kind, or
    // nothing): the other type would not count (ItemOptionDatabase::ResolveLinks), or the loader reports it.
    if (definition.type == ItemOptionType::Excellent && Data::Items::ExcellentByteMeansOther(item))
    {
        return false;
    }
    if (definition.type == ItemOptionType::Wing &&
        (item.slot != Data::Items::ItemSlot::Wings || item.tags.Has(Data::Items::ItemTag::HornMount)))
    {
        return false;
    }
    // A second definition of a type the item links: the first counts.
    return std::ranges::none_of(item.options,
                                [&](const std::string& name)
                                {
                                    const int index = options.Find(name);
                                    return name == definition.name ||
                                           (index != Data::Items::ItemOptionDatabase::NoDefinition &&
                                            options.Get(static_cast<Data::Items::ItemOptionIndex>(index)).type ==
                                                definition.type);
                                });
}

bool IsPickedByName(const ItemOptionDefinition& definition)
{
    return definition.type == Data::Items::ItemOptionType::AncientBonus &&
           std::ranges::find(Data::Items::AncientBonusNames, definition.name) != Data::Items::AncientBonusNames.end();
}

EditedCopyState UpdateEditedCopy(ItemOptionFiles& edit, ItemOptionFiles& loaded, const ItemOptionFiles& stored)
{
    if (edit == loaded || edit == stored)
    {
        edit = stored;
        loaded = stored;
        return EditedCopyState::Follows;
    }
    return stored != loaded ? EditedCopyState::Stale : EditedCopyState::Edited;
}

std::vector<int> FindOptionUsers(std::span<const ItemDefinition> slots, const ItemOptionFiles& files,
                                 std::string_view name)
{
    const AncientBonusPicks picks = FindAncientBonusPicks(files);
    std::vector<int> users;
    for (const ItemDefinition& item : slots)
    {
        if (Links(item, picks, name))
        {
            users.push_back(Data::Items::MakeItemType(item.group, item.number));
        }
    }
    std::ranges::sort(users);
    return users;
}

int CountOptionRuleUses(std::span<const OptionRule> rules, std::string_view name)
{
    return static_cast<int>(std::ranges::count_if(
        rules, [name](const OptionRule& rule) { return std::ranges::find(rule.options, name) != rule.options.end(); }));
}

std::vector<std::string> FindChangedDefinitions(const ItemOptionFiles& before, const ItemOptionFiles& after)
{
    std::vector<std::string> changed;
    for (const ItemOptionDefinition& definition : after.definitions)
    {
        const ItemOptionDefinition* was = FindDefinitionNamed(before, definition.name);
        if (was == nullptr || was->type != definition.type || was->options != definition.options ||
            NamesChangedText(definition, before, after))
        {
            changed.push_back(definition.name);
        }
    }
    for (const ItemOptionDefinition& definition : before.definitions)
    {
        if (FindDefinitionNamed(after, definition.name) == nullptr)
        {
            changed.push_back(definition.name);
        }
    }
    return changed;
}

std::vector<int> FindItemsReached(std::span<const ItemDefinition> slots, const ItemOptionFiles& before,
                                  const ItemOptionFiles& after, std::span<const std::string> changed,
                                  std::string_view renamedFrom)
{
    const AncientBonusPicks picksBefore = FindAncientBonusPicks(before);
    const AncientBonusPicks picksAfter = FindAncientBonusPicks(after);
    const auto uses = [&](const ItemDefinition& item, std::string_view name)
    { return Links(item, picksBefore, name) || Links(item, picksAfter, name); };
    std::vector<int> reached;
    for (const ItemDefinition& item : slots)
    {
        const bool reaches = std::ranges::any_of(changed, [&](const std::string& name) { return uses(item, name); }) ||
                             (!renamedFrom.empty() && uses(item, renamedFrom));
        if (reaches)
        {
            reached.push_back(Data::Items::MakeItemType(item.group, item.number));
        }
    }
    std::ranges::sort(reached);
    return reached;
}

void RenameOptionDefinition(ItemOptionFiles& files, std::string_view from, const std::string& to)
{
    for (ItemOptionDefinition& definition : files.definitions)
    {
        if (definition.name == from)
        {
            definition.name = to;
        }
    }
    for (OptionRule& rule : files.rules)
    {
        std::ranges::replace(rule.options, from, to);
    }
}

std::vector<ItemDefinition> FindItemListChanges(std::span<const ItemDefinition> slots, const ItemOptionFiles& before,
                                                const ItemOptionFiles& after, std::string_view renamedFrom,
                                                std::string_view renamedTo)
{
    const Data::Items::ItemOptionDatabase database(after);
    std::vector<ItemDefinition> changed;
    for (const ItemDefinition& item : slots)
    {
        if (!item.Exists())
        {
            continue;
        }
        std::vector<std::string> list;
        if (IsSameOptionList(item.options, Data::Items::DeriveItemOptions(before.rules, item)))
        {
            list = Data::Items::DeriveItemOptions(after.rules, item);
        }
        else
        {
            list = item.options;
            if (!renamedFrom.empty())
            {
                std::ranges::replace(list, renamedFrom, std::string(renamedTo));
            }
        }
        database.OrderLinks(list);
        if (list != item.options)
        {
            ItemDefinition updated = item;
            updated.options = std::move(list);
            changed.push_back(std::move(updated));
        }
    }
    return changed;
}

bool IsSameOptionList(std::span<const std::string> list, std::span<const std::string> other)
{
    std::vector<std::string> sorted(list.begin(), list.end());
    std::vector<std::string> sortedOther(other.begin(), other.end());
    std::ranges::sort(sorted);
    std::ranges::sort(sortedOther);
    return sorted == sortedOther;
}

int HighestOptionLevel(const ItemOptionDefinition& definition)
{
    int highest = 0;
    for (const Data::Items::ItemOption& option : definition.options)
    {
        // A list by the item's level follows the item, not the option level.
        if (option.value.isList && option.value.levelType == Data::Items::ItemOptionLevelType::OptionLevel)
        {
            highest = std::max(highest, static_cast<int>(option.value.values.size()));
        }
    }
    return highest;
}

bool FillOptionLevels(ItemOptionDefinition& definition, int level)
{
    bool changed = false;
    const size_t size = static_cast<size_t>(std::max(level, 1));
    for (Data::Items::ItemOption& option : definition.options)
    {
        std::vector<int>& values = option.value.values;
        if (!option.value.isList || option.value.levelType != Data::Items::ItemOptionLevelType::OptionLevel ||
            values.empty() || values.size() == size)
        {
            continue;
        }
        if (values.size() > size)
        {
            values.resize(size);
        }
        else
        {
            const int step = values.size() >= 2 ? values.back() - values[values.size() - 2] : values.back();
            while (values.size() < size)
            {
                values.push_back(values.back() + step);
            }
        }
        changed = true;
    }
    return changed;
}

void FillOptionValue(ItemOptionValue& value, int fromLevel, int fromValue, int step, int toLevel)
{
    const int firstLevel = value.FirstLevel();
    std::vector<int> values = value.isList ? value.values : std::vector<int>();
    values.resize(static_cast<size_t>(toLevel - firstLevel + 1), 0);
    for (int level = std::max(fromLevel, firstLevel); level <= toLevel; ++level)
    {
        values[static_cast<size_t>(level - firstLevel)] = fromValue + step * (level - fromLevel);
    }
    value.values = std::move(values);
    value.isList = true;
}
} // namespace MuEditor::Items

#endif // _EDITOR

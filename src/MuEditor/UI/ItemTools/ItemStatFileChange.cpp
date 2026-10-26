#include "stdafx.h"

#ifdef _EDITOR

#include "ItemStatFileChange.h"

#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <algorithm>
#include <map>

namespace MuEditor::Items
{
using Data::Items::ItemDefinition;
using Data::Items::ItemStatProfile;
using Data::Items::ItemStatProfiles;

namespace
{
// Every level whose bonus a table can make differ: 0 to 15 by its values, and
// one above for what it adds per level.
constexpr int LevelsToCompare = Data::Items::ItemLevelCount + 1;

bool SameStatTerms(const ItemStatProfiles& before, const Data::Items::StatTerms& old, const ItemStatProfiles& after,
                   const Data::Items::StatTerms& changed)
{
    if (old.excellent != changed.excellent || old.ancient != changed.ancient ||
        old.perLevelAfterHalving != changed.perLevelAfterHalving)
    {
        return false;
    }
    for (int level = 0; level < LevelsToCompare; ++level)
    {
        if (before.GetLevelBonus(old, level) != after.GetLevelBonus(changed, level))
        {
            return false;
        }
    }
    return true;
}

bool SameRequirementTerms(const Data::Items::RequirementTerms& old, const Data::Items::RequirementTerms& changed)
{
    return old.levelPerItemLevel == changed.levelPerItemLevel && old.excellentLevelAdd == changed.excellentLevelAdd &&
           old.stats == changed.stats && old.strengthPerOptionLevel == changed.strengthPerOptionLevel;
}

bool SameTerms(const ItemStatProfiles& before, const ItemStatProfile& old, const ItemStatProfiles& after,
               const ItemStatProfile& changed)
{
    for (size_t stat = 0; stat < Data::Items::ItemStatCount; ++stat)
    {
        if (!SameStatTerms(before, old.stats[stat], after, changed.stats[stat]))
        {
            return false;
        }
    }
    return SameRequirementTerms(old.requirements, changed.requirements);
}

// A line names a table in its text or in its "values".
bool LineNamesTable(const Data::Items::ItemDescriptionLine& line, std::string_view table)
{
    const std::string placeholder = Data::Items::MakeTablePlaceholder(table);
    if (Data::Items::NamesTable(line.text.GetNeutral(), table) ||
        std::ranges::any_of(line.values, [&](const auto& value) { return value.second == placeholder; }))
    {
        return true;
    }
    return std::ranges::any_of(line.text.GetTranslations(), [&](const auto& translation)
                               { return Data::Items::NamesTable(translation.second, table); });
}

// A description line or a name line of the item names the table.
bool DescriptionNamesTable(const ItemDefinition& item, std::string_view table)
{
    const auto names = [&](const Data::Items::ItemDescriptionLine& line) { return LineNamesTable(line, table); };
    return std::ranges::any_of(item.description, names) || std::ranges::any_of(item.nameLines, names);
}

// The line naming the table `to` where it names `from`, in every language and in its "values".
void RenameTableInLine(Data::Items::ItemDescriptionLine& line, std::string_view from, std::string_view to)
{
    Data::LocalizedString text;
    text.Set(Data::LocalizedString::NeutralLocale,
             Data::Items::RenameTablePlaceholders(line.text.GetNeutral(), from, to));
    for (const auto& [locale, translation] : line.text.GetTranslations())
    {
        text.Set(locale, Data::Items::RenameTablePlaceholders(translation, from, to));
    }
    line.text = std::move(text);
    for (auto& [placeholder, value] : line.values)
    {
        if (value == Data::Items::MakeTablePlaceholder(from))
        {
            value = Data::Items::MakeTablePlaceholder(to);
        }
    }
}

// "" for the default, also when an item names "default".
std::string ProfileOf(const ItemDefinition& item)
{
    return item.statProfile == Data::Items::StatProfileNames::Default ? std::string() : item.statProfile;
}
} // namespace

std::vector<StatProfileChange> FindStatProfileChanges(std::span<const ItemDefinition> slots,
                                                      const ItemStatProfiles& before, const ItemStatProfiles& after,
                                                      std::string_view renamedFrom, std::string_view renamedTo)
{
    std::vector<StatProfileChange> changes;
    for (const ItemDefinition& item : slots)
    {
        if (!item.Exists())
        {
            continue;
        }
        const int itemType = Data::Items::MakeItemType(item.group, item.number);
        if (!renamedFrom.empty() && item.statProfile == renamedFrom)
        {
            changes.push_back({itemType, item.statProfile, std::string(renamedTo)});
            continue;
        }
        const std::string given = Data::Items::DeriveStatProfile(before, item);
        if (!item.statProfile.empty() && item.statProfile != given)
        {
            // Picked for the item.
            continue;
        }
        std::string next = Data::Items::DeriveStatProfile(after, item);
        if (next != given)
        {
            changes.push_back({itemType, item.statProfile, std::move(next)});
        }
    }
    return changes;
}

std::vector<ItemDefinition> ApplyStatProfileChanges(std::span<const ItemDefinition> slots,
                                                    std::span<const StatProfileChange> changes)
{
    std::map<int, const StatProfileChange*> byItemType;
    for (const StatProfileChange& change : changes)
    {
        byItemType[change.itemType] = &change;
    }
    std::vector<ItemDefinition> changed;
    for (const ItemDefinition& item : slots)
    {
        const auto found = byItemType.find(Data::Items::MakeItemType(item.group, item.number));
        if (item.Exists() && found != byItemType.end())
        {
            ItemDefinition copy = item;
            copy.statProfile = found->second->to;
            changed.push_back(std::move(copy));
        }
    }
    return changed;
}

void RenameStatProfile(Data::Items::ItemStatFile& file, std::string_view from, const std::string& to)
{
    for (Data::Items::WrittenStatProfile& profile : file.profiles)
    {
        if (profile.name == from)
        {
            profile.name = to;
        }
    }
    for (Data::Items::StatProfileRule& rule : file.profileRules)
    {
        if (rule.profile == from)
        {
            rule.profile = to;
        }
    }
}

std::vector<int> FindTableDescriptionUsers(std::span<const ItemDefinition> slots, std::string_view table)
{
    std::vector<int> items;
    for (const ItemDefinition& item : slots)
    {
        if (item.Exists() && DescriptionNamesTable(item, table))
        {
            items.push_back(Data::Items::MakeItemType(item.group, item.number));
        }
    }
    std::ranges::sort(items);
    return items;
}

void RenameTableInDescriptions(std::span<const ItemDefinition> slots, std::vector<ItemDefinition>& changed,
                               std::string_view from, std::string_view to)
{
    for (const ItemDefinition& item : slots)
    {
        if (!item.Exists() || !DescriptionNamesTable(item, from))
        {
            continue;
        }
        const auto found = std::ranges::find_if(changed, [&](const ItemDefinition& entry)
                                                { return entry.group == item.group && entry.number == item.number; });
        ItemDefinition& renamed = found != changed.end() ? *found : changed.emplace_back(item);
        for (Data::Items::ItemDescriptionLine& line : renamed.description)
        {
            RenameTableInLine(line, from, to);
        }
        for (Data::Items::ItemDescriptionLine& line : renamed.nameLines)
        {
            RenameTableInLine(line, from, to);
        }
    }
}

std::vector<std::string> FindProfilesWithChangedTerms(const ItemStatProfiles& before, const ItemStatProfiles& after)
{
    const bool bonusesChanged = before.GetExcellentDropLevelBonus() != after.GetExcellentDropLevelBonus() ||
                                before.GetAncientDropLevelBonus() != after.GetAncientDropLevelBonus();
    std::vector<std::string> names = {""};
    const std::vector<std::string> profileNames = after.GetProfileNames();
    names.insert(names.end(), profileNames.begin(), profileNames.end());

    std::vector<std::string> changed;
    for (const std::string& name : names)
    {
        const int old = before.Find(name);
        if (bonusesChanged || old == ItemStatProfiles::UnknownProfile ||
            !SameTerms(before, before.Get(old), after, after.Get(after.Find(name))))
        {
            changed.push_back(name);
        }
    }
    return changed;
}

std::vector<int> FindItemsReached(std::span<const ItemDefinition> slots, std::span<const StatProfileChange> changes,
                                  std::span<const std::string> changedProfiles)
{
    std::vector<int> items;
    for (const StatProfileChange& change : changes)
    {
        items.push_back(change.itemType);
    }
    for (const ItemDefinition& item : slots)
    {
        if (item.Exists() && std::ranges::find(changedProfiles, ProfileOf(item)) != changedProfiles.end())
        {
            items.push_back(Data::Items::MakeItemType(item.group, item.number));
        }
    }
    std::ranges::sort(items);
    items.erase(std::unique(items.begin(), items.end()), items.end());
    return items;
}
} // namespace MuEditor::Items

#endif // _EDITOR

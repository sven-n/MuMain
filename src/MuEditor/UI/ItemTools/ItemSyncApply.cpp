#include "stdafx.h"

#ifdef _EDITOR

#include "ItemSyncApply.h"

#include "ItemEdits.h"
#include "ItemStatFileChange.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <optional>
#include <utility>

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemDataIssue;
using Data::Items::ItemDefinition;
using Data::Items::ItemExchangeFile;
using Data::Items::ItemTag;

// The tags of the exchange file; the client's other tags stay.
constexpr ItemTag SharedTags[] = {ItemTag::Ammunition, ItemTag::SecondClassQuestItem, ItemTag::ThirdClassQuestItem};

// The rule all shipped items follow: groups 0 to 5, a hand slot, two cells wide or more.
bool IsTwoHanded(const ItemDefinition& item)
{
    const bool handSlot = item.slot == Data::Items::ItemSlot::MainHand || item.slot == Data::Items::ItemSlot::OffHand;
    return item.group <= 5 && handSlot && item.width >= 2;
}

// "" for the default, as the profile rules give it.
std::string RuleName(const std::string& profile)
{
    return profile == Data::Items::StatProfileNames::Default ? std::string() : profile;
}

const ItemDefinition* FindItem(const ItemExchangeFile& file, int group, int number)
{
    const auto item = std::ranges::find_if(file.items, [&](const ItemDefinition& entry)
                                           { return entry.group == group && entry.number == number; });
    return item != file.items.end() ? &*item : nullptr;
}
} // namespace

ItemExchangeFile BuildClientExchange()
{
    return Data::Items::BuildItemExchange(g_ItemDatabase.GetAllSlots(), g_ItemDataHandler.GetStatFile());
}

ItemDefinition ToClientItem(const ItemDefinition* current, const ItemDefinition& exchange,
                            const Data::Items::ItemStatProfiles& profiles)
{
    ItemDefinition item = current != nullptr ? *current : ItemDefinition{};
    item.group = exchange.group;
    item.number = exchange.number;
    item.names = exchange.names;
    for (const ItemTag tag : SharedTags)
    {
        item.tags.Set(tag, exchange.tags.Has(tag));
    }
    item.width = exchange.width;
    item.height = exchange.height;
    item.slot = exchange.slot;
    item.skill = exchange.skill;
    item.level = exchange.level;
    item.durability = exchange.durability;
    item.magicDurability = exchange.magicDurability;
    item.damageMin = exchange.damageMin;
    item.damageMax = exchange.damageMax;
    item.blockRate = exchange.blockRate;
    item.defense = exchange.defense;
    item.attackSpeed = exchange.attackSpeed;
    item.walkSpeed = exchange.walkSpeed;
    item.magicPower = exchange.magicPower;
    item.excellentBonus = exchange.excellentBonus;
    item.excellentRaisesLevel = exchange.excellentRaisesLevel;
    item.requirements = exchange.requirements;
    item.requirementsByLevel = exchange.requirementsByLevel;
    item.classRequirements = exchange.classRequirements;
    item.resistances = exchange.resistances;
    item.sellValue = exchange.sellValue;
    item.buyPrice = exchange.buyPrice;
    item.tradable = exchange.tradable;
    item.droppable = exchange.droppable;
    item.storable = exchange.storable;
    item.sellable = exchange.sellable;
    item.personalShopSellable = exchange.personalShopSellable;
    item.repairable = exchange.repairable;
    if (current == nullptr)
    {
        item.twoHanded = IsTwoHanded(item);
        item.weaponKind = Data::Items::GetGroupWeaponKind(item);
    }
    // "" stores the item by its rules (CItemDataHandler::SetItems).
    const std::string ruled = Data::Items::DeriveStatProfile(profiles, item);
    item.statProfile = RuleName(exchange.statProfile) == ruled ? std::string() : exchange.statProfile;
    return item;
}

std::vector<std::string> DescribeNewItem(const ItemDefinition& exchange)
{
    std::vector<std::string> notes;
    const int itemType = Data::Items::MakeItemType(exchange.group, exchange.number);
    if (g_ItemModelDatabase.Find(itemType) == nullptr)
    {
        notes.push_back(I18N::Editor::ItHasNoModelYet);
    }
    const ItemDefinition item = ToClientItem(nullptr, exchange, g_ItemDatabase.GetStatProfiles());
    if (!item.statProfile.empty())
    {
        const std::string ruled = Data::Items::DeriveStatProfile(g_ItemDatabase.GetStatProfiles(), item);
        notes.push_back(I18N::Format(
            I18N::Editor::ItsProfileRulesGive0TheFile1,
            {ruled.empty() ? std::string(Data::Items::StatProfileNames::Default) : ruled, exchange.statProfile}));
    }
    return notes;
}

bool ApplyItemSyncImport(const ItemSyncDiff& diff, const ItemExchangeFile& file, std::vector<ItemDataIssue>& issues)
{
    const ItemExchangeFile client = BuildClientExchange();
    std::vector<ItemDataIssue> applyIssues;
    const ItemExchangeFile applied = ApplyItemSyncDiff(client, file, diff, applyIssues);
    issues.insert(issues.end(), applyIssues.begin(), applyIssues.end());
    if (Data::Items::HasErrors(applyIssues))
    {
        return false;
    }

    // The formulas; the client's profile rules stay.
    std::optional<Data::Items::ItemStatFile> statFile;
    if (applied.formulas && client.formulas && !(*applied.formulas == *client.formulas))
    {
        statFile = *applied.formulas;
        statFile->profileRules = g_ItemDataHandler.GetStatFile().profileRules;
    }
    const Data::Items::ItemStatProfiles profiles =
        statFile ? Data::Items::ItemStatProfiles(*statFile) : g_ItemDatabase.GetStatProfiles();

    std::vector<ItemDefinition> items;
    std::vector<int> removed;
    for (const SyncItem& row : diff.items)
    {
        const int itemType = Data::Items::MakeItemType(row.group, row.number);
        if (row.change == SyncChange::Removed)
        {
            if (row.checked && row.locked.empty())
            {
                removed.push_back(itemType);
            }
            continue;
        }
        const ItemDefinition* exchange = FindItem(applied, row.group, row.number);
        const ItemDefinition* current = g_ItemDatabase.Find(itemType);
        if (exchange == nullptr || (current != nullptr && Data::Items::ToExchangeItem(*current) == *exchange))
        {
            continue;
        }
        items.push_back(ToClientItem(current, *exchange, profiles));
    }

    // The checks of a save, on the data as it would be.
    std::map<int, ItemDefinition> candidates;
    for (const ItemDefinition& item : g_ItemDatabase.GetAllSlots())
    {
        if (item.Exists())
        {
            candidates[Data::Items::MakeItemType(item.group, item.number)] = item;
        }
    }
    for (const ItemDefinition& item : items)
    {
        ItemDefinition candidate = item;
        if (candidate.statProfile.empty())
        {
            candidate.statProfile = Data::Items::DeriveStatProfile(profiles, candidate);
        }
        candidates[Data::Items::MakeItemType(item.group, item.number)] = std::move(candidate);
    }
    for (const int itemType : removed)
    {
        candidates.erase(itemType);
    }
    std::vector<ItemDefinition> checked;
    for (auto& [itemType, item] : candidates)
    {
        checked.push_back(std::move(item));
    }
    std::vector<ItemDataIssue> checkIssues;
    Data::Items::ValidateItems(checked, checkIssues);
    Data::Items::ValidateStatProfiles(checked, profiles, Data::Items::GetItemStatsFile().generic_string(), checkIssues);
    if (statFile)
    {
        Data::Items::ValidateItemStatsAsWritten(*statFile, checkIssues);
    }
    if (Data::Items::HasErrors(checkIssues))
    {
        std::ranges::copy_if(checkIssues, std::back_inserter(issues), [](const ItemDataIssue& issue)
                             { return issue.severity == Data::Items::ItemDataIssueSeverity::Error; });
        return false;
    }

    // One step of the undo history.
    const std::string label = I18N::Editor::ImportItems;
    g_ItemEdits.BeginGroup(label);
    if (statFile)
    {
        const std::vector<StatProfileChange> changes =
            FindStatProfileChanges(g_ItemDatabase.GetAllSlots(), g_ItemDatabase.GetStatProfiles(), profiles);
        g_ItemEdits.SetStatFile(label, *statFile, ApplyStatProfileChanges(g_ItemDatabase.GetAllSlots(), changes));
    }
    if (!items.empty())
    {
        g_ItemEdits.SetItems(label, items);
    }
    for (const int itemType : removed)
    {
        g_ItemEdits.RemoveItem(label, itemType);
    }
    g_ItemEdits.EndGroup();
    return true;
}

bool ApplyItemSyncExport(const ItemSyncDiff& diff, const ItemExchangeFile& file, const std::filesystem::path& path,
                         std::vector<ItemDataIssue>& issues)
{
    ItemExchangeFile written = ApplyItemSyncDiff(BuildClientExchange(), file, diff, issues);
    if (Data::Items::HasErrors(issues))
    {
        return false;
    }
    written.source = Data::Items::ItemExchangeSourceMuMain;
    written.server.clear();
    return Data::Items::WriteItemTextFile(path, Data::Items::WriteItemExchangeJson(written), issues) ==
           Data::Items::ItemDataSaveResult::Saved;
}
} // namespace MuEditor::Items

#endif // _EDITOR

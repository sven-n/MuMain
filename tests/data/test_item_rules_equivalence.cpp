// One-time check for phase 3 of the data-driven items plan: the item rules and
// categories moved from hardcoded lists into the item data. This file keeps a
// copy of the old code (item_rules_old.inl) and
//   - "generate": writes tags, wing tiers and rule flags into src/bin/Data/Items
//     from the old code (run once with --no-skip);
//   - "compare": checks that the new code gives the same answers as the old
//     code for every item and every item state.
// It is not meant to stay: once the data is the source of truth, a data change
// would make it fail.

#include "stdafx.h"

#include "doctest.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/CComGem.h"
#include "GameLogic/Items/ChangeRingManager.h"
#include "GameLogic/Items/ItemCategories.h"
#include "GameLogic/Items/ShopRestrictions.h"
#include "GameLogic/Items/TradeRestrictions.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <vector>

extern CHARACTER* Hero;

#include "item_rules_old.inl"

using namespace Data::Items;

namespace
{
const std::filesystem::path ItemDirectory = std::filesystem::path(MU_TEST_DATA_DIR) / "Items";

struct ItemState
{
    int level = 0;
    bool period = false;
    bool expired = false;
    int durability = 0;
    int ancient = 0;
    int excellent = 0;
};

std::string Describe(const ItemState& state)
{
    return "+" + std::to_string(state.level) + (state.period ? (state.expired ? " rented(expired)" : " rented") : "") +
           " dur " + std::to_string(state.durability) + (state.ancient ? " ancient" : "") +
           (state.excellent ? " excellent" : "");
}

ITEM MakeItem(int itemType, const ItemState& state = {})
{
    ITEM item{};
    item.Type = static_cast<short>(itemType);
    item.Level = state.level;
    item.bPeriodItem = state.period;
    item.bExpiredPeriod = state.expired;
    item.Durability = static_cast<BYTE>(state.durability);
    item.AncientDiscriminator = static_cast<BYTE>(state.ancient);
    item.ExcellentFlags = static_cast<BYTE>(state.excellent);
    return item;
}

std::vector<ItemState> AllStates()
{
    std::vector<ItemState> states;
    for (int level = 0; level <= 15; ++level)
        for (int period = 0; period <= 1; ++period)
            for (int expired = 0; expired <= 1; ++expired)
                for (int durability = 0; durability <= 2; ++durability)
                    for (int ancient = 0; ancient <= 1; ++ancient)
                        for (int excellent = 0; excellent <= 1; ++excellent)
                            states.push_back({level, period == 1, expired == 1, durability, ancient, excellent});
    return states;
}

// The test's own character, so the GM Gift rule has a Hero to look at.
class HeroScope
{
public:
    HeroScope() : m_previous(Hero)
    {
        Hero = &m_character;
        Hero->CtlCode = 0;
    }
    ~HeroScope() { Hero = m_previous; }

    void SetGameMaster(bool isGameMaster) { Hero->CtlCode = isGameMaster ? CTLCODE_20OPERATOR : 0; }

private:
    CHARACTER m_character{};
    CHARACTER* m_previous;
};

std::vector<ItemDefinition> LoadShippedItems()
{
    mu::log::Init();
    ItemDataLoadResult result = LoadItemDataDirectory(ItemDirectory);
    for (const ItemDataIssue& issue : result.issues)
    {
        if (issue.severity == ItemDataIssueSeverity::Error)
        {
            FAIL(issue.message);
        }
    }
    g_ItemDatabase.Build(result.items);
    return result.items;
}

std::vector<int> ExistingItemTypes()
{
    std::vector<int> types;
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        if (g_ItemDatabase.Find(itemType) != nullptr)
        {
            types.push_back(itemType);
        }
    }
    return types;
}

class DifferenceReport
{
public:
    void Add(const std::string& function, int itemType, const std::string& detail, bool oldValue, bool newValue)
    {
        auto& lines = m_differences[function];
        lines.push_back(g_ItemDatabase.GetLogName(itemType) + (detail.empty() ? "" : " " + detail) +
                        ": old " + (oldValue ? "true" : "false") + ", new " + (newValue ? "true" : "false"));
    }

    size_t Count() const
    {
        size_t count = 0;
        for (const auto& [function, lines] : m_differences)
            count += lines.size();
        return count;
    }

    void Write(const std::string& fileName) const
    {
        std::ofstream file(fileName);
        for (const auto& [function, lines] : m_differences)
        {
            file << "== " << function << " (" << lines.size() << ")\n";
            for (const std::string& line : lines)
                file << line << "\n";
        }
        MESSAGE(fileName << ": " << Count() << " differences");
    }

private:
    std::map<std::string, std::vector<std::string>> m_differences;
};

using ItemPredicate = bool (*)(const ITEM*);
using NumberPredicate = bool (*)(int);

void CompareItem(DifferenceReport& report, const char* name, ItemPredicate oldFunction, ItemPredicate newFunction,
                 const std::vector<int>& types)
{
    for (int itemType : types)
    {
        const ITEM item = MakeItem(itemType);
        const bool oldValue = oldFunction(&item);
        const bool newValue = newFunction(&item);
        if (oldValue != newValue)
            report.Add(name, itemType, "", oldValue, newValue);
    }
}

void CompareType(DifferenceReport& report, const char* name, NumberPredicate oldFunction, NumberPredicate newFunction,
                 const std::vector<int>& types)
{
    for (int itemType : types)
    {
        const bool oldValue = oldFunction(itemType);
        const bool newValue = newFunction(itemType);
        if (oldValue != newValue)
            report.Add(name, itemType, "", oldValue, newValue);
    }
    // Values outside the item range (a model id of another kind, -1 for "none").
    for (int itemType : {-MAX_ITEM, -1, MAX_ITEM, MAX_ITEM + 1000})
    {
        if (oldFunction(itemType) != newFunction(itemType))
            report.Add(name, itemType, "(out of range)", oldFunction(itemType), newFunction(itemType));
    }
}

void CompareModel(DifferenceReport& report, const char* name, NumberPredicate oldFunction,
                  NumberPredicate newFunction, const std::vector<int>& types)
{
    for (int itemType : types)
    {
        const bool oldValue = oldFunction(MODEL_ITEM + itemType);
        const bool newValue = newFunction(MODEL_ITEM + itemType);
        if (oldValue != newValue)
            report.Add(name, itemType, "(model)", oldValue, newValue);
    }
    for (int modelType : {0, MODEL_ITEM - 1, MODEL_ITEM + MAX_ITEM, MODEL_ITEM + MAX_ITEM + 1000, -1})
    {
        if (oldFunction(modelType) != newFunction(modelType))
            report.Add(name, modelType - MODEL_ITEM, "(model out of range)", oldFunction(modelType),
                       newFunction(modelType));
    }
}

void CompareStates(DifferenceReport& report, const char* name, ItemPredicate oldFunction, ItemPredicate newFunction,
                   const std::vector<int>& types, const std::vector<ItemState>& states, const char* suffix = "")
{
    for (int itemType : types)
    {
        for (const ItemState& state : states)
        {
            const ITEM item = MakeItem(itemType, state);
            const bool oldValue = oldFunction(&item);
            const bool newValue = newFunction(&item);
            if (oldValue != newValue)
                report.Add(std::string(name) + suffix, itemType, Describe(state), oldValue, newValue);
        }
    }
}

// The flags and tags of one item, from the old code, for an item at +0 that
// is bought (not rented) and held by a player who is not a GM.
void FillFromOldCode(ItemDefinition& definition)
{
    const int itemType = MakeItemType(definition.group, definition.number);
    const ITEM item = MakeItem(itemType);

    definition.tradable = !OldRules::IsTradeBan(&item);
    definition.droppable = !OldRules::IsDropBan(&item);
    definition.storable = !OldRules::IsStoreBan(&item);
    definition.sellable = !OldRules::IsSellingBan(&item);
    definition.personalShopSellable = !OldRules::IsPersonalShopBan(&item);
    definition.repairable = !OldRules::IsRepairBan(&item);

    definition.wingTier = WingTier::None;
    if (OldRules::IsSmallWing(&item))
        definition.wingTier = WingTier::Small;
    else if (OldRules::IsThirdTierWing(&item))
        definition.wingTier = WingTier::Third;
    else if (OldRules::IsSecondTierWingExceptCape(&item) || itemType == ITEM_CAPE_OF_LORD ||
             itemType == ITEM_CAPE_OF_FIGHTER)
        definition.wingTier = WingTier::Second;
    else if (OldRules::IsWingItem(&item))
        definition.wingTier = WingTier::First;

    // An expired rental at +0: every item of the old list counts, also the
    // rented pets. The Wizard's Ring +0 stays a level exception in the code.
    const ITEM expiredRental = MakeItem(itemType, {0, true, true, 0, 0, 0});
    const bool valuable = OldRules::IsHighValueItem(&expiredRental) && itemType != ITEM_WIZARDS_RING;

    const std::pair<ItemTag, bool> tags[] = {
        {ItemTag::Mount, OldRules::IsRideableMount(&item)},
        {ItemTag::Flying, OldRules::IsFlyingMount(&item)},
        {ItemTag::DarkLordPet, OldRules::IsDarkLordPet(&item)},
        {ItemTag::GuardianPet, OldRules::IsDemonOrSpiritOfGuardian(&item)},
        {ItemTag::PandaOrSkeleton, OldRules::IsPandaOrSkeletonItem(&item)},
        {ItemTag::Jewel, OldRules::IsJewelItem(&item)},
        {ItemTag::RefineStone, OldRules::IsRefineStone(&item)},
        {ItemTag::SocketSeed, OldRules::IsSocketSeed(&item)},
        {ItemTag::SocketSphere, OldRules::IsSocketSphere(&item)},
        {ItemTag::SocketSeedSphere, OldRules::IsSocketSeedSphere(&item)},
        {ItemTag::HealingPotion, OldRules::IsHealingPotion(&item)},
        {ItemTag::ManaPotion, OldRules::IsManaPotionType(itemType)},
        {ItemTag::ComplexPotion, OldRules::IsComplexPotion(&item)},
        {ItemTag::ElitePotion, OldRules::IsElitePotion(&item)},
        {ItemTag::Elixir, OldRules::IsElixir(&item)},
        {ItemTag::BuffScroll, OldRules::IsBuffScroll(&item)},
        {ItemTag::BattleOrStrengthScroll, OldRules::IsBattleOrStrengthScroll(&item)},
        {ItemTag::Ammunition, OldRules::IsAmmunition(&item)},
        {ItemTag::BloodCastleTicketPart, OldRules::IsBloodCastleTicketPart(&item)},
        {ItemTag::SecondClassQuestItem, OldRules::IsSecondClassQuestItem(&item)},
        {ItemTag::ThirdClassQuestItem, OldRules::IsThirdClassQuestItem(&item)},
        {ItemTag::SummonerBook, OldRules::IsSummonerBook(&item)},
        {ItemTag::DivineArchangelWeapon, OldRules::IsDivineArchangelWeapon(&item)},
        {ItemTag::CashShop, OldRules::IsPartChargeItem(&item)},
        {ItemTag::GambleItem, OldRules::IsGambleItem(&item)},
        {ItemTag::GemJewelry, OldRules::IsGemJewelry(&item)},
        {ItemTag::LuckyItemTicket, OldRules::IsLuckyItemTicket(&item)},
        {ItemTag::Valuable, valuable},
    };
    definition.tags = ItemTagSet{};
    for (const auto& [tag, value] : tags)
        definition.tags.Set(tag, value);
}
} // namespace

TEST_CASE("generate the phase 3 item data from the old code" * doctest::skip())
{
    HeroScope hero;
    std::vector<ItemDefinition> items = LoadShippedItems();
    for (ItemDefinition& definition : items)
        FillFromOldCode(definition);

    std::vector<ItemDataIssue> issues;
    const ItemDataSaveResult result = SaveItemDataDirectory(ItemDirectory, items, issues);
    for (const ItemDataIssue& issue : issues)
        MESSAGE(issue.message);
    CHECK(result == ItemDataSaveResult::Saved);
}

TEST_CASE("the item categories give the same answers as the old code")
{
    HeroScope hero;
    LoadShippedItems();
    const std::vector<int> types = ExistingItemTypes();
    DifferenceReport report;

#define COMPARE_ITEM(name) CompareItem(report, #name, OldRules::name, GameLogic::Items::name, types)
#define COMPARE_TYPE(name) CompareType(report, #name, OldRules::name, GameLogic::Items::name, types)
#define COMPARE_MODEL(name) CompareModel(report, #name, OldRules::name, GameLogic::Items::name, types)

    COMPARE_ITEM(IsWingItem);
    COMPARE_ITEM(IsSecondTierWingExceptCape);
    COMPARE_ITEM(IsThirdTierWing);
    COMPARE_ITEM(IsSmallWing);
    COMPARE_ITEM(IsClothWing);
    COMPARE_MODEL(IsClothWingModel);
    COMPARE_ITEM(IsRideableMount);
    COMPARE_MODEL(IsRideableMountModel);
    COMPARE_MODEL(IsHornMountModel);
    COMPARE_ITEM(IsFlyingMount);
    COMPARE_ITEM(IsDarkLordPet);
    COMPARE_TYPE(IsDarkLordPetType);
    COMPARE_ITEM(IsDemonOrSpiritOfGuardian);
    COMPARE_TYPE(IsDemonOrSpiritOfGuardianType);
    COMPARE_MODEL(IsDemonOrSpiritOfGuardianModel);
    COMPARE_ITEM(IsPandaOrSkeletonItem);
    COMPARE_ITEM(IsJewelItem);
    COMPARE_ITEM(IsRefineStone);
    COMPARE_MODEL(IsRefineStoneModel);
    COMPARE_ITEM(IsWingMixCharm);
    COMPARE_TYPE(IsWingMixCharmType);
    COMPARE_MODEL(IsWingMixCharmModel);
    COMPARE_ITEM(IsSocketSeedOrSphere);
    COMPARE_TYPE(IsSocketSeedOrSphereType);
    COMPARE_MODEL(IsSocketSeedOrSphereModel);
    COMPARE_ITEM(IsSocketSeed);
    COMPARE_MODEL(IsSocketSeedModel);
    COMPARE_ITEM(IsSocketSphere);
    COMPARE_MODEL(IsSocketSphereModel);
    COMPARE_ITEM(IsSocketSeedSphere);
    COMPARE_TYPE(IsSocketSeedSphereType);
    COMPARE_MODEL(IsSocketSeedSphereModel);
    COMPARE_ITEM(IsHealingPotion);
    COMPARE_TYPE(IsHealingPotionType);
    COMPARE_TYPE(IsManaPotionType);
    COMPARE_ITEM(IsComplexPotion);
    COMPARE_TYPE(IsComplexPotionType);
    COMPARE_ITEM(IsElitePotion);
    COMPARE_TYPE(IsElitePotionType);
    COMPARE_MODEL(IsElitePotionModel);
    COMPARE_ITEM(IsElixir);
    COMPARE_TYPE(IsElixirType);
    COMPARE_MODEL(IsElixirModel);
    COMPARE_ITEM(IsBuffScroll);
    COMPARE_TYPE(IsBuffScrollType);
    COMPARE_MODEL(IsBuffScrollModel);
    COMPARE_ITEM(IsBattleOrStrengthScroll);
    COMPARE_TYPE(IsBattleOrStrengthScrollType);
    COMPARE_MODEL(IsBattleOrStrengthScrollModel);
    COMPARE_ITEM(IsResetFruit);
    COMPARE_TYPE(IsResetFruitType);
    COMPARE_MODEL(IsResetFruitModel);
    COMPARE_ITEM(IsSeal);
    COMPARE_TYPE(IsSealType);
    COMPARE_MODEL(IsSealModel);
    COMPARE_ITEM(IsHealingOrDivinitySeal);
    COMPARE_TYPE(IsHealingOrDivinitySealType);
    COMPARE_MODEL(IsHealingOrDivinitySealModel);
    COMPARE_ITEM(IsAmmunition);
    COMPARE_TYPE(IsAmmunitionType);
    COMPARE_MODEL(IsAmmunitionModel);
    COMPARE_ITEM(IsEventTicket);
    COMPARE_TYPE(IsEventTicketType);
    COMPARE_MODEL(IsEventTicketModel);
    COMPARE_ITEM(IsDoppelgangerOrVarkaTicket);
    COMPARE_TYPE(IsDoppelgangerOrVarkaTicketType);
    COMPARE_MODEL(IsDoppelgangerOrVarkaTicketModel);
    COMPARE_ITEM(IsLuckyItemTicket);
    COMPARE_MODEL(IsLuckyItemTicketModel);
    COMPARE_ITEM(IsAccountServiceItem);
    COMPARE_MODEL(IsAccountServiceItemModel);
    COMPARE_ITEM(IsDayPass);
    COMPARE_MODEL(IsDayPassModel);
    COMPARE_ITEM(IsHourPass);
    COMPARE_MODEL(IsHourPassModel);
    COMPARE_ITEM(IsPackageBox);
    COMPARE_MODEL(IsPackageBoxModel);
    COMPARE_ITEM(IsSilverOrGoldKey);
    COMPARE_MODEL(IsSilverOrGoldKeyModel);
    COMPARE_ITEM(IsGemJewelry);
    COMPARE_ITEM(IsGambleItem);
    COMPARE_TYPE(IsGambleItemType);
    COMPARE_MODEL(IsGambleItemModel);
    COMPARE_ITEM(IsCharacterCard);
    COMPARE_MODEL(IsCharacterCardModel);
    COMPARE_ITEM(IsDevilSquareItem);
    COMPARE_TYPE(IsDevilSquareItemType);
    COMPARE_ITEM(IsBloodCastleTicketPart);
    COMPARE_TYPE(IsBloodCastleTicketPartType);
    COMPARE_MODEL(IsBloodCastleTicketPartModel);
    COMPARE_ITEM(IsChocolateBox);
    COMPARE_TYPE(IsChocolateBoxType);
    COMPARE_MODEL(IsChocolateBoxModel);
    COMPARE_ITEM(IsRibbonBox);
    COMPARE_TYPE(IsRibbonBoxType);
    COMPARE_MODEL(IsRibbonBoxModel);
    COMPARE_ITEM(IsSecondClassQuestItem);
    COMPARE_TYPE(IsSecondClassQuestItemType);
    COMPARE_ITEM(IsThirdClassQuestItem);
    COMPARE_ITEM(IsSecromiconQuestItem);
    COMPARE_MODEL(IsSecromiconQuestItemModel);
    COMPARE_ITEM(IsDivineArchangelWeapon);
    COMPARE_TYPE(IsDivineArchangelWeaponType);
    COMPARE_MODEL(IsDivineArchangelWeaponModel);
    COMPARE_ITEM(IsSummonerBook);
    COMPARE_TYPE(IsSummonerBookType);
    COMPARE_MODEL(IsSummonerBookModel);
    COMPARE_MODEL(IsSummonerStickModel);
    COMPARE_MODEL(IsSummonerSkillParchmentModel);
    COMPARE_MODEL(IsRageFighterSkillParchmentModel);
    CompareItem(report, "IsPartChargeItem -> IsCashShopItem", OldRules::IsPartChargeItem,
                GameLogic::Items::IsCashShopItem, types);

    // HasFlightEquipment takes two items.
    for (int helper : types)
    {
        const ITEM helperItem = MakeItem(helper);
        const ITEM noWing = MakeItem(-1);
        if (OldRules::HasFlightEquipment(&helperItem, &noWing) != GameLogic::Items::HasFlightEquipment(&helperItem, &noWing))
            report.Add("HasFlightEquipment", helper, "(as helper)", true, false);
        if (OldRules::HasFlightEquipment(&noWing, &helperItem) != GameLogic::Items::HasFlightEquipment(&noWing, &helperItem))
            report.Add("HasFlightEquipment", helper, "(as wing)", true, false);
    }

#undef COMPARE_ITEM
#undef COMPARE_TYPE
#undef COMPARE_MODEL

    report.Write("item_categories_differences.txt");
    CHECK(report.Count() == 0);
}

TEST_CASE("the item rules give the same answers as the old code")
{
    HeroScope hero;
    LoadShippedItems();
    const std::vector<int> types = ExistingItemTypes();
    const std::vector<ItemState> states = AllStates();
    DifferenceReport report;

    for (bool isGameMaster : {false, true})
    {
        hero.SetGameMaster(isGameMaster);
        const char* suffix = isGameMaster ? " (GM)" : "";
        CompareStates(report, "IsTradeBan", OldRules::IsTradeBan, GameLogic::Items::IsTradeBan, types, states, suffix);
    }
    hero.SetGameMaster(false);
    CompareStates(report, "IsDropBan", OldRules::IsDropBan, GameLogic::Items::IsDropBan, types, states);
    CompareStates(report, "IsStoreBan", OldRules::IsStoreBan, GameLogic::Items::IsStoreBan, types, states);
    CompareStates(report, "IsPersonalShopBan", OldRules::IsPersonalShopBan, GameLogic::Items::IsPersonalShopBan, types,
                  states);
    CompareStates(report, "IsSellingBan", OldRules::IsSellingBan, GameLogic::Items::IsSellingBan, types, states);
    CompareStates(report, "IsRepairBan", OldRules::IsRepairBan, GameLogic::Items::IsRepairBan, types, states);
    CompareStates(report, "IsHighValueItem", OldRules::IsHighValueItem, GameLogic::Items::IsHighValueItem, types,
                  states);

    report.Write("item_rules_differences.txt");
    CHECK(report.Count() == 0);
}

// Not a pass/fail check: lists where the repair lists in ZzzInventory.cpp
// differ from IsRepairBan, before they are replaced by it.
TEST_CASE("repair lists compared with IsRepairBan" * doctest::skip())
{
    HeroScope hero;
    LoadShippedItems();
    const std::vector<int> types = ExistingItemTypes();
    DifferenceReport report;
    CompareItem(report, "RepairAllGold inventory filter", OldRepairAllSkips, GameLogic::Items::IsRepairBan, types);
    CompareItem(report, "RenderRepairInfo early returns", OldRepairInfoHidden, GameLogic::Items::IsRepairBan, types);
    report.Write("repair_list_differences.txt");
}

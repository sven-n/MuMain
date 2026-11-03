#include "stdafx.h"

#include "doctest.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Items/EquipmentRestrictions.h"
#include "GameLogic/Items/ItemCategories.h"
#include "GameLogic/Items/ShopRestrictions.h"
#include "GameLogic/Items/TradeRestrictions.h"
#include "World/MapInfra/MapManager.h"

#include <filesystem>
#include <vector>

extern CHARACTER* Hero;

using namespace Data::Items;
using namespace GameLogic::Items;

// The item rules and categories with the item data shipped in src/bin/Data.
// They catch a changed flag or tag in the data as well as a changed check in
// the rule code.
namespace
{
constexpr int KrisType = MakeItemType(0, 0);

struct ItemState
{
    int level = 0;
    bool rented = false;
    bool expired = false;
    int durability = 0;
};

ITEM MakeItem(int itemType, const ItemState& state = {})
{
    ITEM item{};
    item.Type = static_cast<short>(itemType);
    item.Level = state.level;
    item.bPeriodItem = state.rented;
    item.bExpiredPeriod = state.expired;
    item.Durability = static_cast<BYTE>(state.durability);
    return item;
}

// The item at 0 durability.
ITEM Broken(ITEM item)
{
    item.Durability = 0;
    return item;
}

// Loads the shipped items and gives the rules a Hero (for the GM Gift).
class ShippedItemsScope
{
public:
    ShippedItemsScope() : m_previousHero(Hero)
    {
        mu::log::Init();
        const ItemDataLoadResult result =
            LoadItemDataDirectory(std::filesystem::path(MU_TEST_DATA_DIR) / "Items");
        REQUIRE_FALSE(HasFatal(result.issues));
        g_ItemDatabase.Build(result.items);
        Hero = &m_character;
    }

    ~ShippedItemsScope()
    {
        Hero = m_previousHero;
    }

    void SetGameMaster(bool isGameMaster)
    {
        Hero->CtlCode = isGameMaster ? CTLCODE_20OPERATOR : 0;
    }

private:
    CHARACTER m_character{};
    CHARACTER* m_previousHero;
};

bool TradeBan(int itemType, const ItemState& state = {})
{
    const ITEM item = MakeItem(itemType, state);
    return IsTradeBan(&item);
}

bool DropBan(int itemType, const ItemState& state = {})
{
    const ITEM item = MakeItem(itemType, state);
    return IsDropBan(&item);
}

bool StoreBan(int itemType, const ItemState& state = {})
{
    const ITEM item = MakeItem(itemType, state);
    return IsStoreBan(&item);
}

bool PersonalShopBan(int itemType, const ItemState& state = {})
{
    const ITEM item = MakeItem(itemType, state);
    return IsPersonalShopBan(&item);
}

bool SellingBan(int itemType, const ItemState& state = {})
{
    const ITEM item = MakeItem(itemType, state);
    return IsSellingBan(&item);
}

bool RepairBan(int itemType, const ItemState& state = {})
{
    const ITEM item = MakeItem(itemType, state);
    return IsRepairBan(&item);
}

bool Valuable(int itemType, const ItemState& state = {})
{
    const ITEM item = MakeItem(itemType, state);
    return IsHighValueItem(&item);
}
} // namespace

TEST_CASE("A normal weapon allows every action [data][items][rules]")
{
    ShippedItemsScope items;

    CHECK_FALSE(TradeBan(KrisType));
    CHECK_FALSE(DropBan(KrisType));
    CHECK_FALSE(StoreBan(KrisType));
    CHECK_FALSE(PersonalShopBan(KrisType));
    CHECK_FALSE(SellingBan(KrisType));
    CHECK_FALSE(RepairBan(KrisType));
}

TEST_CASE("Items the client does not know allow no action [data][items][rules]")
{
    ShippedItemsScope items;
    const int emptyType = MakeItemType(0, 500);
    REQUIRE(g_ItemDatabase.Find(emptyType) == nullptr);

    CHECK(TradeBan(emptyType));
    CHECK(DropBan(emptyType));
    CHECK(StoreBan(emptyType));
    CHECK(PersonalShopBan(emptyType));
    CHECK(SellingBan(emptyType));
    CHECK(RepairBan(emptyType));
}

TEST_CASE("Item level variants have their own rules [data][items][rules]")
{
    ShippedItemsScope items;

    // Rena +3 is the Sign of Lord; Rena +1 (Stone) cannot be sold.
    CHECK(TradeBan(ITEM_RENA));
    CHECK_FALSE(TradeBan(ITEM_RENA, {3}));
    CHECK_FALSE(StoreBan(ITEM_RENA, {3}));
    CHECK_FALSE(PersonalShopBan(ITEM_RENA, {3}));
    CHECK(SellingBan(ITEM_RENA, {1}));
    CHECK_FALSE(SellingBan(ITEM_RENA));

    // Box of Luck +13 is the Heart of Dark Lord.
    CHECK_FALSE(TradeBan(ITEM_BOX_OF_LUCK));
    CHECK(TradeBan(ITEM_BOX_OF_LUCK, {13}));
    CHECK(StoreBan(ITEM_BOX_OF_LUCK, {13}));
    CHECK(PersonalShopBan(ITEM_BOX_OF_LUCK, {13}));
    CHECK(SellingBan(ITEM_BOX_OF_LUCK));

    // The Wizard's Ring is bound to the character on the server (OpenMU), so it
    // can't be traded, stored or sold at any level; it can still be dropped.
    for (int level : {0, 1, 2})
    {
        CHECK(TradeBan(ITEM_WIZARDS_RING, {level}));
        CHECK(StoreBan(ITEM_WIZARDS_RING, {level}));
        CHECK(PersonalShopBan(ITEM_WIZARDS_RING, {level}));
        CHECK(SellingBan(ITEM_WIZARDS_RING, {level}));
    }
    CHECK_FALSE(DropBan(ITEM_WIZARDS_RING));

    // Remedy of Love +1 to +5 cannot be sold.
    CHECK(SellingBan(ITEM_REMEDY_OF_LOVE, {3}));
    CHECK_FALSE(SellingBan(ITEM_REMEDY_OF_LOVE));
}

TEST_CASE("Rented items have their own rules [data][items][rules]")
{
    ShippedItemsScope items;

    CHECK(StoreBan(KrisType, {0, true}));

    CHECK_FALSE(DropBan(ITEM_CHAOS_CARD));
    CHECK(DropBan(ITEM_CHAOS_CARD, {0, true}));

    CHECK_FALSE(PersonalShopBan(ITEM_DEMON));
    CHECK(PersonalShopBan(ITEM_DEMON, {0, true}));
    CHECK_FALSE(PersonalShopBan(ITEM_SPIRIT_OF_GUARDIAN));
    CHECK(PersonalShopBan(ITEM_SPIRIT_OF_GUARDIAN, {0, true}));

    CHECK(SellingBan(ITEM_DEMON));
    CHECK(SellingBan(ITEM_DEMON, {0, true}));
    CHECK_FALSE(SellingBan(ITEM_DEMON, {0, true, true}));
}

TEST_CASE("Durability and the player change some rules [data][items][rules]")
{
    ShippedItemsScope items;

    CHECK(StoreBan(ITEM_TALISMAN_OF_MOBILITY, {0, false, false, 1}));
    CHECK_FALSE(StoreBan(ITEM_TALISMAN_OF_MOBILITY, {0, false, false, 2}));

    items.SetGameMaster(false);
    CHECK(TradeBan(ITEM_GM_GIFT));
    items.SetGameMaster(true);
    CHECK_FALSE(TradeBan(ITEM_GM_GIFT));
}

TEST_CASE("Potions cannot be repaired [data][items][rules]")
{
    ShippedItemsScope items;

    CHECK(RepairBan(ITEM_APPLE));
    CHECK(RepairBan(ITEM_JEWEL_OF_BLESS));
}

TEST_CASE("Valuable items ask for confirmation [data][items][rules]")
{
    ShippedItemsScope items;

    CHECK(Valuable(ITEM_JEWEL_OF_BLESS));
    CHECK(Valuable(ITEM_WING));
    CHECK_FALSE(Valuable(KrisType));
    CHECK(Valuable(KrisType, {7}));
    CHECK(Valuable(ITEM_WIZARDS_RING));
    CHECK_FALSE(Valuable(ITEM_WIZARDS_RING, {1}));

    // Rented items are not valuable while rented; rented pets only once
    // the rental time ran out.
    CHECK_FALSE(Valuable(ITEM_JEWEL_OF_BLESS, {0, true}));
    CHECK_FALSE(Valuable(ITEM_DEMON));
    CHECK(Valuable(ITEM_DEMON, {0, true, true}));
}

TEST_CASE("Item categories come from the item data [data][items][rules]")
{
    ShippedItemsScope items;
    const ITEM wingsOfElf = MakeItem(ITEM_WING);
    const ITEM wingsOfSpirits = MakeItem(ITEM_WINGS_OF_SPIRITS);
    const ITEM capeOfLord = MakeItem(ITEM_CAPE_OF_LORD);
    const ITEM uniria = MakeItem(ITEM_HORN_OF_UNIRIA);
    const ITEM fenrir = MakeItem(ITEM_HORN_OF_FENRIR);
    const ITEM durableFenrir = MakeItem(ITEM_HORN_OF_FENRIR, {0, false, false, 255});
    const ITEM kris = MakeItem(KrisType);

    CHECK(IsWingItem(&wingsOfElf));
    CHECK(IsWingItem(&capeOfLord));
    CHECK_FALSE(IsWingItem(&kris));
    CHECK(IsSecondTierWingExceptCape(&wingsOfSpirits));
    CHECK_FALSE(IsSecondTierWingExceptCape(&capeOfLord));
    const ITEM wingOfStorm = MakeItem(ITEM_WING_OF_STORM);
    const ITEM capeOfEmperor = MakeItem(ITEM_CAPE_OF_EMPEROR);
    const ITEM smallCapeOfLord = MakeItem(ITEM_SMALL_CAPE_OF_LORD);
    CHECK(IsSecondOrThirdTierWing(&wingsOfSpirits));
    CHECK(IsSecondOrThirdTierWing(&capeOfLord));
    CHECK(IsSecondOrThirdTierWing(&wingOfStorm));
    CHECK_FALSE(IsSecondOrThirdTierWing(&wingsOfElf));
    CHECK_FALSE(IsSecondOrThirdTierWing(&smallCapeOfLord));
    CHECK(IsSecondTierWing(&wingsOfSpirits));
    CHECK(IsSecondTierWing(&capeOfLord));
    CHECK_FALSE(IsSecondTierWing(&wingOfStorm));
    CHECK_FALSE(IsSecondTierWing(&wingsOfElf));
    CHECK_FALSE(IsSecondTierWing(&smallCapeOfLord));
    CHECK(IsSecondTierCape(&capeOfLord));
    CHECK_FALSE(IsSecondTierCape(&wingsOfSpirits));
    CHECK_FALSE(IsSecondTierCape(&capeOfEmperor));
    CHECK_FALSE(IsSecondTierCape(&smallCapeOfLord));

    CHECK(IsPetSlotModel(MODEL_ITEM + ITEM_HELPER));
    CHECK(IsPetSlotModel(MODEL_ITEM + ITEM_HORN_OF_FENRIR));
    CHECK_FALSE(IsPetSlotModel(MODEL_ITEM + KrisType));
    CHECK(IsRideableMount(&uniria));
    CHECK(IsHornMountModel(MODEL_ITEM + ITEM_HORN_OF_UNIRIA));
    CHECK_FALSE(IsHornMountModel(MODEL_ITEM + ITEM_DARK_HORSE_ITEM));
    CHECK_FALSE(IsFlyingMount(&uniria));
    CHECK(IsFlyingMount(&fenrir));
    CHECK(HasFlightEquipment(&durableFenrir, &kris));

    CHECK(IsSocketSeedOrSphereType(ITEM_SEED_FIRE));
    CHECK_FALSE(IsSocketSeedOrSphereType(KrisType));

    // Model ids outside the item range are no items.
    CHECK_FALSE(IsRideableMountModel(-1));
    CHECK_FALSE(IsRideableMountModel(MODEL_ITEM + MAX_ITEM));
    CHECK_FALSE(IsPetSlotModel(-1));
    CHECK_FALSE(IsPetSlotModel(MODEL_ITEM + MAX_ITEM));
}

TEST_CASE("Weapon kinds come from the item data [data][items][rules]")
{
    ShippedItemsScope items;
    // Numbers without item data in the bow and shield groups.
    const int noBow = ITEM_BOW + 27;
    const int noShield = ITEM_SHIELD + 27;
    const ITEM smallShield = MakeItem(ITEM_SHIELD);
    const ITEM shortBow = MakeItem(ITEM_BOW);
    const ITEM crossbow = MakeItem(ITEM_CROSSBOW);
    const ITEM arrows = MakeItem(ITEM_ARROWS);
    const ITEM bolt = MakeItem(ITEM_BOLT);
    const ITEM kris = MakeItem(KrisType);

    // Bows have the slot offHand, crossbows mainHand; arrows and bolts are neither.
    CHECK(IsBowType(ITEM_BOW));
    CHECK(IsBowType(ITEM_CELESTIAL_BOW));
    CHECK_FALSE(IsBowType(noBow));
    CHECK_FALSE(IsBowType(ITEM_CROSSBOW));
    CHECK_FALSE(IsBowType(ITEM_BOLT));
    CHECK(IsCrossbowType(ITEM_CROSSBOW));
    CHECK(IsCrossbowType(ITEM_DIVINE_CB_OF_ARCHANGEL));
    CHECK_FALSE(IsCrossbowType(ITEM_BOW));
    CHECK_FALSE(IsCrossbowType(ITEM_ARROWS));
    CHECK(IsBowModel(MODEL_ITEM + ITEM_BOW));
    CHECK(IsCrossbowModel(MODEL_ITEM + ITEM_CROSSBOW));

    CHECK(IsSpearType(ITEM_SPEAR));
    CHECK(IsSpearModel(MODEL_ITEM + ITEM_SPEAR));
    CHECK_FALSE(IsSpearType(KrisType));

    CHECK(IsShieldType(ITEM_SHIELD));
    CHECK_FALSE(IsShieldType(noShield));
    CHECK_FALSE(IsShieldType(KrisType));

    CHECK(IsBowCrossbowOrAmmunition(&shortBow));
    CHECK(IsBowCrossbowOrAmmunition(&crossbow));
    CHECK(IsBowCrossbowOrAmmunition(&arrows));
    CHECK(IsBowCrossbowOrAmmunition(&bolt));
    CHECK_FALSE(IsBowCrossbowOrAmmunition(&kris));
    CHECK_FALSE(IsBowCrossbowOrAmmunition(&smallShield));

    // Model ids outside the item range are no weapons.
    CHECK_FALSE(IsSpearModel(-1));
    CHECK_FALSE(IsBowModel(MODEL_ITEM + MAX_ITEM));
}

TEST_CASE("Swords, gloves, staffs, books, sceptres and magic swords come from the item data [data][items][rules]")
{
    ShippedItemsScope items;
    // Numbers without item data in the sword, staff and mace groups.
    const int noGlove = MakeItemType(0, 40);
    const int noBook = MakeItemType(5, 40);
    const int noSceptre = MakeItemType(2, 21);
    const ITEM kris = MakeItem(KrisType);
    const ITEM bolt = MakeItem(ITEM_BOLT);
    const ITEM bookOfSahamutt = MakeItem(ITEM_BOOK_OF_SAHAMUTT);
    const ITEM smallShield = MakeItem(ITEM_SHIELD);
    const ITEM apple = MakeItem(ITEM_APPLE);

    CHECK(IsSwordOrGloveModel(MODEL_ITEM + KrisType));
    CHECK(IsSwordOrGloveModel(MODEL_ITEM + ITEM_SACRED_GLOVE));
    CHECK_FALSE(IsSwordOrGloveModel(MODEL_ITEM + ITEM_AXE));

    CHECK(IsSwordAxeMaceOrGloveType(KrisType));
    CHECK(IsSwordAxeMaceOrGloveType(ITEM_AXE));
    CHECK(IsSwordAxeMaceOrGloveType(ITEM_FROST_MACE));
    CHECK_FALSE(IsSwordAxeMaceOrGloveType(noGlove));
    CHECK_FALSE(IsSwordAxeMaceOrGloveType(ITEM_SPEAR));
    CHECK_FALSE(IsSwordAxeMaceOrGloveType(ITEM_STAFF));

    CHECK(IsStaffOrBookType(ITEM_STAFF));
    CHECK(IsStaffOrBookType(ITEM_BOOK_OF_SAHAMUTT));
    CHECK_FALSE(IsStaffOrBookType(noBook));
    CHECK_FALSE(IsStaffOrBookType(KrisType));
    CHECK(IsStaffOrBookModel(MODEL_ITEM + ITEM_STAFF));

    CHECK(IsWeaponOrAmmunition(&kris));
    CHECK(IsWeaponOrAmmunition(&bolt));
    CHECK(IsWeaponOrAmmunition(&bookOfSahamutt));
    CHECK_FALSE(IsWeaponOrAmmunition(&smallShield));
    CHECK_FALSE(IsWeaponOrAmmunition(&apple));

    CHECK(IsSceptreType(ITEM_BATTLE_SCEPTER));
    CHECK_FALSE(IsSceptreType(noSceptre));
    CHECK_FALSE(IsSceptreType(ITEM_FROST_MACE));

    CHECK(IsMagicSwordType(ITEM_DARK_REIGN_BLADE));
    CHECK_FALSE(IsMagicSwordType(ITEM_BONE_BLADE));
}

TEST_CASE("Lucky items, shield potions and transformation rings come from the item data [data][items][rules]")
{
    ShippedItemsScope items;
    const int scaleHelmLucky = MakeItemType(7, 62);
    const int stormJahadBootsLucky = MakeItemType(11, 72);
    const int phoenixSoulHelm = MakeItemType(7, 73);

    CHECK(IsLuckyItemType(scaleHelmLucky));
    CHECK(IsLuckyItemType(stormJahadBootsLucky));
    CHECK_FALSE(IsLuckyItemType(phoenixSoulHelm));
    CHECK_FALSE(IsLuckyItemType(ITEM_HELM));

    CHECK(IsShieldPotionType(ITEM_SMALL_SHIELD_POTION));
    CHECK(IsShieldPotionType(ITEM_LARGE_SHIELD_POTION));
    CHECK_FALSE(IsShieldPotionType(ITEM_ELITE_SD_POTION));
    CHECK_FALSE(IsShieldPotionType(ITEM_SMALL_COMPLEX_POTION));

    CHECK(IsTransformationRingType(ITEM_TRANSFORMATION_RING));
    CHECK(IsTransformationRingType(ITEM_SNOWMAN_TRANSFORMATION_RING));
    CHECK(IsTransformationRingType(ITEM_PANDA_TRANSFORMATION_RING));
    CHECK_FALSE(IsTransformationRingType(ITEM_RING_OF_ICE));
    CHECK(IsPandaOrSkeletonType(ITEM_PANDA_TRANSFORMATION_RING));
    CHECK_FALSE(IsPandaOrSkeletonType(ITEM_TRANSFORMATION_RING));
}

TEST_CASE("The shipped potions are taken largest first [data][items][rules]")
{
    ShippedItemsScope items;
    const auto inUseOrder = [](ItemTag tag)
    {
        const auto itemTypes = g_ItemDatabase.GetItemTypesWithTagInUseOrder(tag);
        return std::vector<int>(itemTypes.begin(), itemTypes.end());
    };

    CHECK(inUseOrder(ItemTag::HealingPotion) == std::vector<int>{ITEM_LARGE_HEALING_POTION, ITEM_MEDIUM_HEALING_POTION,
                                                                 ITEM_SMALL_HEALING_POTION, ITEM_APPLE});
    CHECK(inUseOrder(ItemTag::ManaPotion) ==
          std::vector<int>{ITEM_LARGE_MANA_POTION, ITEM_MEDIUM_MANA_POTION, ITEM_SMALL_MANA_POTION});
    CHECK(inUseOrder(ItemTag::ShieldPotion) ==
          std::vector<int>{ITEM_LARGE_SHIELD_POTION, ITEM_MEDIUM_SHIELD_POTION, ITEM_SMALL_SHIELD_POTION});
    CHECK(inUseOrder(ItemTag::ComplexPotion) ==
          std::vector<int>{ITEM_LARGE_COMPLEX_POTION, ITEM_MEDIUM_COMPLEX_POTION, ITEM_SMALL_COMPLEX_POTION});
}

TEST_CASE("The item types with a tag come lowest first [data][items][rules]")
{
    ShippedItemsScope items;

    const auto manaPotions = g_ItemDatabase.GetItemTypesWithTag(ItemTag::ManaPotion);
    CHECK(std::vector<int>(manaPotions.begin(), manaPotions.end()) ==
          std::vector<int>{ITEM_SMALL_MANA_POTION, ITEM_MEDIUM_MANA_POTION, ITEM_LARGE_MANA_POTION});
    CHECK(g_ItemDatabase.GetItemTypesWithTag(ItemTag::Ammunition).size() == 2);
}

TEST_CASE("Wings, the Dinorant, the Dark Horse and the Fenrir are flight equipment [data][items][rules]")
{
    ShippedItemsScope items;
    const ItemState durable{0, false, false, 255};
    const ITEM none = MakeItem(-1);
    const ITEM wings = MakeItem(ITEM_WING, durable);
    const ITEM capeOfLord = MakeItem(ITEM_CAPE_OF_LORD, durable);
    const ITEM dinorant = MakeItem(ITEM_HORN_OF_DINORANT, durable);
    const ITEM fenrir = MakeItem(ITEM_HORN_OF_FENRIR, durable);
    const ITEM darkHorse = MakeItem(ITEM_DARK_HORSE_ITEM, durable);
    const ITEM uniria = MakeItem(ITEM_HORN_OF_UNIRIA, durable);
    const ITEM brokenWings = Broken(wings);
    const ITEM brokenFenrir = Broken(fenrir);

    CHECK(HasFlightEquipment(&none, &wings));
    CHECK(HasFlightEquipment(&none, &capeOfLord));
    CHECK(HasFlightEquipment(&dinorant, &none));
    CHECK(HasFlightEquipment(&fenrir, &none));
    CHECK(HasFlightEquipment(&darkHorse, &none));
    CHECK_FALSE(HasFlightEquipment(&uniria, &none));
    // A broken item (0 durability) gives nothing, so it does not fly.
    CHECK_FALSE(HasFlightEquipment(&none, &brokenWings));
    CHECK_FALSE(HasFlightEquipment(&brokenFenrir, &none));
}

TEST_CASE("In Icarus the last flight equipment cannot be taken off [data][items][rules]")
{
    ShippedItemsScope items;
    const ItemState durable{0, false, false, 255};
    const ITEM none = MakeItem(-1);
    const ITEM wings = MakeItem(ITEM_WING, durable);
    const ITEM fenrir = MakeItem(ITEM_HORN_OF_FENRIR, durable);
    const ITEM darkHorse = MakeItem(ITEM_DARK_HORSE_ITEM, durable);
    const ITEM brokenWings = Broken(wings);
    const ITEM brokenFenrir = Broken(fenrir);

    // Wings and a flying mount: either one can go, the other still flies.
    CHECK(CanTakeOff(EQUIPMENT_WING, WD_10HEAVEN, &fenrir, &wings));
    CHECK(CanTakeOff(EQUIPMENT_HELPER, WD_10HEAVEN, &fenrir, &wings));

    // Only one of them: it stays on.
    CHECK_FALSE(CanTakeOff(EQUIPMENT_WING, WD_10HEAVEN, &none, &wings));
    CHECK_FALSE(CanTakeOff(EQUIPMENT_HELPER, WD_10HEAVEN, &fenrir, &none));

    // The Dark Horse flies: with wings either one can go, alone it stays on.
    CHECK(CanTakeOff(EQUIPMENT_WING, WD_10HEAVEN, &darkHorse, &wings));
    CHECK(CanTakeOff(EQUIPMENT_HELPER, WD_10HEAVEN, &darkHorse, &wings));
    CHECK_FALSE(CanTakeOff(EQUIPMENT_HELPER, WD_10HEAVEN, &darkHorse, &none));

    // Broken Fenrir: the wings are the last flight equipment.
    CHECK_FALSE(CanTakeOff(EQUIPMENT_WING, WD_10HEAVEN, &brokenFenrir, &wings));
    // Nothing flies anyway: taking off broken wings changes nothing.
    CHECK(CanTakeOff(EQUIPMENT_WING, WD_10HEAVEN, &none, &brokenWings));

    // Other slots and other maps are not restricted; only Icarus requires flight.
    CHECK(CanTakeOff(EQUIPMENT_WEAPON_RIGHT, WD_10HEAVEN, &none, &wings));
    CHECK(CanTakeOff(EQUIPMENT_WING, WD_0LORENCIA, &none, &wings));
    CHECK(CanTakeOff(EQUIPMENT_WING, WD_39KANTURU_3RD, &none, &wings));
    CHECK(RequiresFlight(WD_10HEAVEN));
    CHECK_FALSE(RequiresFlight(WD_39KANTURU_3RD));
}

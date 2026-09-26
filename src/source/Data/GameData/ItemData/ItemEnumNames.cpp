#include "stdafx.h"

#include "ItemEnumNames.h"

#include <array>

namespace Data::Items
{
namespace
{
// ItemSlot::None has no name: items that cannot be equipped have no slot.
constexpr std::array<EnumName<ItemSlot>, 11> ItemSlotNames = {{
    {ItemSlot::MainHand, "mainHand"},
    {ItemSlot::OffHand, "offHand"},
    {ItemSlot::Helm, "helm"},
    {ItemSlot::Armor, "armor"},
    {ItemSlot::Pants, "pants"},
    {ItemSlot::Gloves, "gloves"},
    {ItemSlot::Boots, "boots"},
    {ItemSlot::Wings, "wings"},
    {ItemSlot::Pet, "pet"},
    {ItemSlot::Pendant, "pendant"},
    {ItemSlot::Ring, "ring"},
}};

constexpr std::array<EnumName<WingTier>, 4> WingTierNames = {{
    {WingTier::Small, "small"},
    {WingTier::First, "first"},
    {WingTier::Second, "second"},
    {WingTier::Third, "third"},
}};

constexpr std::array<EnumName<ItemTag>, static_cast<size_t>(ItemTag::Count)> ItemTagNames = {{
    {ItemTag::Cape, "cape"},
    {ItemTag::Mount, "mount"},
    {ItemTag::HornMount, "hornMount"},
    {ItemTag::Flying, "flying"},
    {ItemTag::DarkLordPet, "darkLordPet"},
    {ItemTag::GuardianPet, "guardianPet"},
    {ItemTag::PandaOrSkeleton, "pandaOrSkeleton"},
    {ItemTag::Jewel, "jewel"},
    {ItemTag::RefineStone, "refineStone"},
    {ItemTag::SocketSeed, "socketSeed"},
    {ItemTag::SocketSphere, "socketSphere"},
    {ItemTag::SocketSeedSphere, "socketSeedSphere"},
    {ItemTag::HealingPotion, "healingPotion"},
    {ItemTag::ManaPotion, "manaPotion"},
    {ItemTag::ComplexPotion, "complexPotion"},
    {ItemTag::ElitePotion, "elitePotion"},
    {ItemTag::Elixir, "elixir"},
    {ItemTag::BuffScroll, "buffScroll"},
    {ItemTag::BattleOrStrengthScroll, "battleOrStrengthScroll"},
    {ItemTag::Ammunition, "ammunition"},
    {ItemTag::BloodCastleTicketPart, "bloodCastleTicketPart"},
    {ItemTag::SecondClassQuestItem, "secondClassQuestItem"},
    {ItemTag::ThirdClassQuestItem, "thirdClassQuestItem"},
    {ItemTag::SummonerBook, "summonerBook"},
    {ItemTag::DivineArchangelWeapon, "divineArchangelWeapon"},
    {ItemTag::CashShop, "cashShop"},
    {ItemTag::GambleItem, "gambleItem"},
    {ItemTag::GemJewelry, "gemJewelry"},
    {ItemTag::LuckyItemTicket, "luckyItemTicket"},
    {ItemTag::Valuable, "valuable"},
}};
} // namespace

std::span<const EnumName<ItemSlot>> GetEnumNames(ItemSlot)
{
    return ItemSlotNames;
}

std::span<const EnumName<WingTier>> GetEnumNames(WingTier)
{
    return WingTierNames;
}

std::span<const EnumName<ItemTag>> GetEnumNames(ItemTag)
{
    return ItemTagNames;
}
} // namespace Data::Items

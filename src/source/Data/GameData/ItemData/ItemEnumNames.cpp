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

constexpr std::array<EnumName<WeaponKind>, 10> WeaponKindNames = {{
    {WeaponKind::Spear, "spear"},
    {WeaponKind::Bow, "bow"},
    {WeaponKind::Crossbow, "crossbow"},
    {WeaponKind::Shield, "shield"},
    {WeaponKind::Sword, "sword"},
    {WeaponKind::Axe, "axe"},
    {WeaponKind::Mace, "mace"},
    {WeaponKind::Staff, "staff"},
    {WeaponKind::Glove, "glove"},
    {WeaponKind::Book, "book"},
}};

constexpr std::array<EnumName<ItemTag>, static_cast<size_t>(ItemTag::Count)> ItemTagNames = {{
    {ItemTag::Cape, "cape"},
    {ItemTag::Mount, "mount"},
    {ItemTag::HornMount, "hornMount"},
    {ItemTag::Flying, "flying"},
    {ItemTag::DarkLordPet, "darkLordPet"},
    {ItemTag::GuardianPet, "guardianPet"},
    {ItemTag::PandaOrSkeleton, "pandaOrSkeleton"},
    {ItemTag::TransformationRing, "transformationRing"},
    {ItemTag::Jewel, "jewel"},
    {ItemTag::RefineStone, "refineStone"},
    {ItemTag::SocketSeed, "socketSeed"},
    {ItemTag::SocketSphere, "socketSphere"},
    {ItemTag::SocketSeedSphere, "socketSeedSphere"},
    {ItemTag::HealingPotion, "healingPotion"},
    {ItemTag::ManaPotion, "manaPotion"},
    {ItemTag::ComplexPotion, "complexPotion"},
    {ItemTag::ShieldPotion, "shieldPotion"},
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
    {ItemTag::Sceptre, "sceptre"},
    {ItemTag::MagicSword, "magicSword"},
    {ItemTag::LuckyItem, "luckyItem"},
    {ItemTag::CashShop, "cashShop"},
    {ItemTag::GambleItem, "gambleItem"},
    {ItemTag::GemJewelry, "gemJewelry"},
    {ItemTag::LuckyItemTicket, "luckyItemTicket"},
    {ItemTag::Valuable, "valuable"},
    {ItemTag::NoLevelRequirement, "noLevelRequirement"},
    {ItemTag::NoDurabilityLine, "noDurabilityLine"},
    {ItemTag::NoBlankAfterOptions, "noBlankAfterOptions"},
}};

constexpr std::array<EnumName<ItemRequirement>, ItemRequirementCount> ItemRequirementNames = {{
    {ItemRequirement::Level, "level"},
    {ItemRequirement::Strength, "strength"},
    {ItemRequirement::Dexterity, "dexterity"},
    {ItemRequirement::Energy, "energy"},
    {ItemRequirement::Vitality, "vitality"},
    {ItemRequirement::Leadership, "leadership"},
}};

constexpr std::array<EnumName<ItemTextColor>, ItemTextColorCount> ItemTextColorNames = {{
    {ItemTextColor::White, "white"},
    {ItemTextColor::Blue, "blue"},
    {ItemTextColor::Red, "red"},
    {ItemTextColor::Yellow, "yellow"},
    {ItemTextColor::Green, "green"},
    {ItemTextColor::DarkRed, "darkRed"},
    {ItemTextColor::Purple, "purple"},
    {ItemTextColor::DarkBlue, "darkBlue"},
    {ItemTextColor::DarkYellow, "darkYellow"},
    {ItemTextColor::GreenBlue, "greenBlue"},
    {ItemTextColor::Gray, "gray"},
    {ItemTextColor::RedPurple, "redPurple"},
    {ItemTextColor::Violet, "violet"},
    {ItemTextColor::Orange, "orange"},
}};

// One entry per color, in the order of their values: a color added to ItemTextColor without a name here fails to
// compile, instead of being saved as a number that reading the file then refuses.
constexpr bool NamesEveryTextColor()
{
    for (size_t index = 0; index < ItemTextColorNames.size(); ++index)
    {
        if (ItemTextColorNames[index].name == nullptr || static_cast<size_t>(ItemTextColorNames[index].value) != index)
        {
            return false;
        }
    }
    return true;
}
static_assert(NamesEveryTextColor(), "every ItemTextColor needs its name in ItemTextColorNames");
} // namespace

std::span<const EnumName<ItemSlot>> GetEnumNames(ItemSlot)
{
    return ItemSlotNames;
}

std::span<const EnumName<WingTier>> GetEnumNames(WingTier)
{
    return WingTierNames;
}

std::span<const EnumName<WeaponKind>> GetEnumNames(WeaponKind)
{
    return WeaponKindNames;
}

std::span<const EnumName<ItemTag>> GetEnumNames(ItemTag)
{
    return ItemTagNames;
}

std::span<const EnumName<ItemTextColor>> GetEnumNames(ItemTextColor)
{
    return ItemTextColorNames;
}

std::span<const EnumName<ItemRequirement>> GetEnumNames(ItemRequirement)
{
    return ItemRequirementNames;
}
} // namespace Data::Items

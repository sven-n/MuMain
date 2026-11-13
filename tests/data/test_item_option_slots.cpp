#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "HeroScope.h"
#include "ItemOptionsScope.h"
#include "LogCapture.h"

#include "Character/CharacterManager.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/ItemOptionSlots.h"
#include "GameLogic/Items/ItemOptions.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "I18N/All.h"
#include "Network/Server/WSclient.h"
#include "UI/Inventory/ItemMng.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

struct OptionSlot
{
    WORD option;
    WORD value;

    bool operator==(const OptionSlot& other) const = default;
};
} // namespace

// An item has MAX_ITEM_SPECIAL option slots. An option past the last is dropped and leaves the item's other fields as
// they are.
TEST_CASE("An item keeps the options its slots hold and drops the rest [items]")
{
    constexpr DWORD key = 0x12345678;
    ITEM item{};
    item.Type = ITEM_BLADE;
    item.Key = key;
    for (int i = 0; i < MAX_ITEM_SPECIAL; ++i)
    {
        GameLogic::Items::AddOptionSlot(item, static_cast<std::uint16_t>(AT_IMPROVE_DAMAGE + i), i + 1);
    }
    GameLogic::Items::AddOptionSlot(item, AT_LUCK, 99);

    REQUIRE(item.SpecialNum == MAX_ITEM_SPECIAL);
    for (int i = 0; i < MAX_ITEM_SPECIAL; ++i)
    {
        CAPTURE(i);
        CHECK(item.Special[i] == AT_IMPROVE_DAMAGE + i);
        CHECK(item.SpecialValue[i] == i + 1);
    }
    CHECK(item.Key == key);
}

// The first option an item type drops is logged, with the item and the option; the type's later drops are not. The log
// remembers the types for the whole run, so this test uses types no other test drops an option of.
TEST_CASE("A dropped option is logged once per item type [items]")
{
    const auto fullItem = [](int type)
    {
        ITEM item{};
        item.Type = static_cast<short>(type);
        for (int i = 0; i < MAX_ITEM_SPECIAL; ++i)
        {
            GameLogic::Items::AddOptionSlot(item, AT_IMPROVE_DEFENSE);
        }
        return item;
    };
    const auto droppedLine = [](int type, int option)
    {
        return "Item " + std::to_string(type) + " has more than " + std::to_string(MAX_ITEM_SPECIAL) +
               " options; option " + std::to_string(option) + " and those after it are dropped";
    };

    {
        LogCapture log("gameplay");
        ITEM shield = fullItem(ITEM_SMALL_SHIELD);
        GameLogic::Items::AddOptionSlot(shield, AT_LUCK);
        CHECK_FALSE(log.LineWith(droppedLine(ITEM_SMALL_SHIELD, AT_LUCK)).empty());
    }
    {
        LogCapture log("gameplay");
        ITEM shield = fullItem(ITEM_SMALL_SHIELD);
        GameLogic::Items::AddOptionSlot(shield, AT_LUCK);
        GameLogic::Items::AddOptionSlot(shield, AT_IMPROVE_BLOCKING);
        ITEM otherShield = fullItem(ITEM_HORN_SHIELD);
        GameLogic::Items::AddOptionSlot(otherShield, AT_IMPROVE_BLOCKING);

        CHECK(log.LineWith("Item " + std::to_string(ITEM_SMALL_SHIELD) + " ").empty());
        CHECK_FALSE(log.LineWith(droppedLine(ITEM_HORN_SHIELD, AT_IMPROVE_BLOCKING)).empty());
    }
}

// A weapon with its skill, luck, an additional option and all six excellent options has nine options. With eight
// slots, the ninth was written over the first two slots' values.
TEST_CASE("A weapon keeps its skill, luck, additional option and six excellent options [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    const WORD bladeSkill = ItemAttribute[ITEM_BLADE].m_wSkillIndex;
    REQUIRE(bladeSkill != 0);

    // Blade (0/5) +0 as the server writes it: group and number, level, durability, the option flags, then the
    // additional option's byte (level 4) and the excellent options' byte (all six).
    const auto flags = static_cast<BYTE>(ItemOptionFlags::HasSkill | ItemOptionFlags::HasLuck |
                                         ItemOptionFlags::HasOption | ItemOptionFlags::HasExcellent);
    const std::array<BYTE, 7> itemData{0x00, 5, 0, 30, flags, 0x04, 0x3F};

    // The excellent damage level option is the hero's level / 20.
    HeroScope hero;
    hero.Hero().Level = 100;
    ITEM blade;
    FillItemFromParameters(blade, ParseItemData(itemData));

    const std::array<OptionSlot, 9> expected{{
        {bladeSkill, 0},
        {AT_LUCK, 5},
        {AT_IMPROVE_DAMAGE, 16},
        {AT_EXCELLENT_DAMAGE, 0},
        {AT_IMPROVE_DAMAGE_LEVEL, 5},
        {AT_IMPROVE_DAMAGE_PERCENT, 2},
        {AT_IMPROVE_ATTACK_SPEED, 7},
        {AT_IMPROVE_GAIN_LIFE, 0},
        {AT_IMPROVE_GAIN_MANA, 0},
    }};
    REQUIRE(blade.SpecialNum == expected.size());
    for (size_t i = 0; i < expected.size(); ++i)
    {
        CAPTURE(i);
        CHECK(blade.Special[i] == expected[i].option);
        CHECK(blade.SpecialValue[i] == expected[i].value);
    }
}

// The Dark Horse's defense (5 + agility / 20 + the horse's level * 2) gets its value again on every pet info update,
// for example while its tooltip is shown. It keeps one slot, also when that slot is the first: a horse without its
// skill has the defense in slot 0, which the update took for "no defense slot yet" and added another one.
TEST_CASE("A Dark Horse keeps one defense slot over pet info updates [items]")
{
    HeroScope hero;
    hero.Hero().Dexterity = 40;
    PET_INFO pet{};

    for (const bool withSkill : {false, true})
    {
        CAPTURE(withSkill);
        ITEM horse{};
        horse.Type = ITEM_DARK_HORSE_ITEM;
        if (withSkill)
        {
            GameLogic::Items::AddOptionSlot(horse, AT_SKILL_EARTHSHAKE);
        }
        const int defenseSlot = horse.SpecialNum;

        for (const int level : {1, 2, 3})
        {
            pet.m_wLevel = static_cast<WORD>(level);
            giPetManager::SetPetItemConvert(&horse, &pet);
        }
        REQUIRE(horse.SpecialNum == defenseSlot + 1);
        CHECK(horse.Special[defenseSlot] == AT_SET_OPTION_IMPROVE_DEFENCE);
        CHECK(horse.SpecialValue[defenseSlot] == 5 + 40 / 20 + 3 * 2);
    }
}

// The Dark Horse's defense counts all of the character's agility, as OpenMU does (Pets.cs, Stats.TotalAgility): its
// base agility and the agility its items, sets and sockets add.
TEST_CASE("A Dark Horse's defense counts the agility from items [items]")
{
    HeroScope hero;
    hero.Hero().Dexterity = 40;
    hero.Hero().AddDexterity = 60;
    PET_INFO pet{};
    pet.m_wLevel = 1;
    ITEM horse{};
    horse.Type = ITEM_DARK_HORSE_ITEM;
    giPetManager::SetPetItemConvert(&horse, &pet);

    REQUIRE(horse.SpecialNum == 1);
    CHECK(horse.SpecialValue[0] == 5 + (40 + 60) / 20 + 1 * 2);
}

// An option's value has more than a byte: the Dark Horse's defense passes 255 at 3020 agility with a horse at level 50,
// and OpenMU adds all of it (Pets.cs, DefenseBase + 5 + horse level * 2 + agility / 20).
TEST_CASE("An option keeps a value above 255 [items]")
{
    HeroScope hero;
    hero.Hero().Dexterity = 3020;
    PET_INFO pet{};
    pet.m_wLevel = 50;
    ITEM horse{};
    horse.Type = ITEM_DARK_HORSE_ITEM;
    giPetManager::SetPetItemConvert(&horse, &pet);

    REQUIRE(horse.SpecialNum == 1);
    CHECK(horse.SpecialValue[0] == 256);
}

// The Wings of Despair's additional option is wizardry damage (option type 2) or curse damage (type 0), as on the other
// second tier wings. A type the client has no option for takes no slot: it took one with option 0, which has no text,
// and the tooltip stops at its first empty line, so every line after it was gone.
TEST_CASE("The Wings of Despair take no option slot for an option type they have no option for [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    const auto wingsWithOption = [](BYTE optionType)
    {
        ItemCreationParams params{};
        params.Group = ITEM_WINGS_OF_DESPAIR / MAX_ITEM_INDEX;
        params.Number = ITEM_WINGS_OF_DESPAIR % MAX_ITEM_INDEX;
        params.WithOption = true;
        params.OptionLevel = 2;
        params.OptionType = optionType;
        ITEM wings;
        FillItemFromParameters(wings, params);
        return wings;
    };

    const ITEM wizardry = wingsWithOption(2);
    REQUIRE(wizardry.SpecialNum == 1);
    CHECK(wizardry.Special[0] == AT_IMPROVE_MAGIC);
    CHECK(wizardry.SpecialValue[0] == 8);
    const ITEM curse = wingsWithOption(0);
    REQUIRE(curse.SpecialNum == 1);
    CHECK(curse.Special[0] == AT_IMPROVE_CURSE);
    CHECK(curse.SpecialValue[0] == 8);
    for (const int otherType : {1, 3})
    {
        CAPTURE(otherType);
        CHECK(wingsWithOption(static_cast<BYTE>(otherType)).SpecialNum == 0);
    }
}

namespace
{
// The items the code before the option definitions gave luck (SetItemAttributes): groups 0 to 11 but arrows and bolts,
// the first wings (12/0 to 12/6), the Wing of Curse and the Wings of Despair, the Cape of Lord and the Cape of Fighter,
// and the third tier wings.
bool OldCodeGivesLuck(int type)
{
    if (type >= ITEM_SWORD && type < ITEM_BOOTS + MAX_ITEM_INDEX)
    {
        return !g_ItemDatabase.HasTag(type, Data::Items::ItemTag::Ammunition);
    }
    if ((type >= ITEM_WING && type <= ITEM_WINGS_OF_DARKNESS) ||
        (type >= ITEM_WING_OF_CURSE && type <= ITEM_WINGS_OF_DESPAIR))
    {
        return true;
    }
    return type == ITEM_CAPE_OF_LORD || type == ITEM_CAPE_OF_FIGHTER ||
           g_ItemDatabase.GetWingTier(type) == Data::Items::WingTier::Third;
}
} // namespace

// Luck comes from the definition the item links (Data/Items/Options/Luck.json): every item gets its slot where the code
// before the definitions put it (after the skill), with the definition's value, which the character window adds up
// into the critical damage rate, and the slot keeps where it came from.
TEST_CASE("Every item's luck comes from its option links as the code before the definitions gave it [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    const int luck = g_ItemDatabase.GetOptions().Find("luck");
    REQUIRE(luck != Data::Items::ItemOptionDatabase::NoDefinition);
    // The Dark Horse's defense reads the hero's agility.
    HeroScope hero;

    int luckyItems = 0;
    for (int type = 0; type < MAX_ITEM; ++type)
    {
        if (g_ItemDatabase.Find(type) == nullptr)
        {
            continue;
        }
        CAPTURE(type);
        ItemCreationParams params{};
        params.Group = type / MAX_ITEM_INDEX;
        params.Number = type % MAX_ITEM_INDEX;
        params.WithLuck = true;
        params.WithSkill = true;
        ITEM item;
        FillItemFromParameters(item, params);

        std::vector<int> luckSlots;
        for (int slot = 0; slot < item.SpecialNum; ++slot)
        {
            if (item.Special[slot] == AT_LUCK)
            {
                luckSlots.push_back(slot);
            }
        }
        REQUIRE(luckSlots.size() == (OldCodeGivesLuck(type) ? 1u : 0u));
        if (luckSlots.empty())
        {
            continue;
        }
        ++luckyItems;
        const int slot = luckSlots.front();
        CHECK(slot == (ItemAttribute[type].m_wSkillIndex != 0 ? 1 : 0));
        CHECK(item.SpecialValue[slot] == 5);
        CHECK(GameLogic::Items::GetOptionSource(item, slot) ==
              GameLogic::Items::OptionSource{static_cast<std::uint16_t>(luck), 0});
        CHECK(GameLogic::Items::GetItemOptionValue(item, slot) == 5);
    }
    CHECK(luckyItems > 400);
}

// A slot the code wrote has no source, a definition's keeps it; past the slots there is none.
TEST_CASE("An option slot keeps where its option came from [items]")
{
    ITEM item{};
    item.Type = ITEM_BLADE;
    GameLogic::Items::AddOptionSlot(item, AT_SKILL_FALLING_SLASH);
    GameLogic::Items::AddOptionSlot(item, AT_LUCK, 0, {3, 1});
    CHECK_FALSE(GameLogic::Items::GetOptionSource(item, 0).has_value());
    CHECK(GameLogic::Items::GetOptionSource(item, 1) == GameLogic::Items::OptionSource{3, 1});
    CHECK_FALSE(GameLogic::Items::GetOptionSource(item, 2).has_value());
    CHECK_FALSE(GameLogic::Items::FindSlotOption(item, 0).has_value());
}

// Both item tooltips, the item's and the Dark Lord pets', write a slot of an option definition as its texts, and leave
// a slot the code wrote (the skill) to GetSpecialOptionText.
TEST_CASE("A slot of an option definition writes its texts into the tooltip's rows [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    ItemCreationParams params{};
    params.Group = ITEM_BLADE / MAX_ITEM_INDEX;
    params.Number = ITEM_BLADE % MAX_ITEM_INDEX;
    params.WithLuck = true;
    params.WithSkill = true;
    ITEM blade;
    FillItemFromParameters(blade, params);
    REQUIRE(blade.SpecialNum == 2);

    CHECK_FALSE(WriteDefinitionOptionLines(blade, 0, 0, MaxTextListRows).has_value());
    CHECK(WriteDefinitionOptionLines(blade, 1, 3, MaxTextListRows) == 2);
    CHECK(std::wstring(TextList[3]) == L"Luck (success rate of Jewel of Soul +25%)");
    CHECK(std::wstring(TextList[4]) == L"Luck (critical damage rate +5%)");
    CHECK(TextListColor[4] == TEXT_COLOR_BLUE);
    CHECK(TextBold[4] == 0);
    // Only the rows before the limit.
    CHECK(WriteDefinitionOptionLines(blade, 1, 3, 4) == 1);
}

namespace
{
// The rings and pendants with an additional option (Options/OptionRules.json).
bool IsJewelryWithOption(int type)
{
    if (type / MAX_ITEM_INDEX != ITEM_HELPER / MAX_ITEM_INDEX)
    {
        return false;
    }
    constexpr std::array<int, 12> numbers{8, 9, 12, 13, 21, 22, 23, 24, 25, 26, 27, 28};
    return std::find(numbers.begin(), numbers.end(), type % MAX_ITEM_INDEX) != numbers.end();
}

// The additional option the code before the definitions gave an item at `level` (SetItemAttributes up to 9a2), without
// the Horn of Dinorant and the wings, whose code stays. Its levels end at Season 6's: 4, the jewelry's 3.
std::optional<OptionSlot> OldAdditionalOption(int type, int optionLevel)
{
    if (optionLevel == 0)
    {
        return std::nullopt;
    }
    const int level = std::min(optionLevel, 4);
    const int jewelryLevel = std::min(optionLevel, 3);
    if (type >= ITEM_SWORD && type < ITEM_BOW + MAX_ITEM_INDEX)
    {
        if (g_ItemDatabase.HasTag(type, Data::Items::ItemTag::Ammunition))
        {
            return std::nullopt;
        }
        return OptionSlot{AT_IMPROVE_DAMAGE, static_cast<WORD>(level * 4)};
    }
    if (type >= ITEM_STAFF && type < ITEM_STAFF + MAX_ITEM_INDEX)
    {
        const bool book = g_ItemDatabase.HasTag(type, Data::Items::ItemTag::SummonerBook);
        return OptionSlot{static_cast<WORD>(book ? AT_IMPROVE_CURSE : AT_IMPROVE_MAGIC), static_cast<WORD>(level * 4)};
    }
    if (type >= ITEM_SHIELD && type < ITEM_SHIELD + MAX_ITEM_INDEX)
    {
        return OptionSlot{AT_IMPROVE_BLOCKING, static_cast<WORD>(level * 5)};
    }
    if (type >= ITEM_HELM && type < ITEM_BOOTS + MAX_ITEM_INDEX)
    {
        return OptionSlot{AT_IMPROVE_DEFENSE, static_cast<WORD>(level * 4)};
    }
    if (IsJewelryWithOption(type))
    {
        if (type == ITEM_RING_OF_MAGIC)
        {
            return OptionSlot{AT_IMPROVE_MAX_MANA, static_cast<WORD>(jewelryLevel)};
        }
        if (type == ITEM_PENDANT_OF_ABILITY)
        {
            return OptionSlot{AT_IMPROVE_MAX_AG, static_cast<WORD>(jewelryLevel)};
        }
        return OptionSlot{AT_LIFE_REGENERATION, static_cast<WORD>(jewelryLevel)};
    }
    return std::nullopt;
}

// All of the item's option slots.
std::vector<OptionSlot> AllSlots(const ITEM& item)
{
    std::vector<OptionSlot> slots;
    for (int slot = 0; slot < item.SpecialNum; ++slot)
    {
        slots.push_back({item.Special[slot], item.SpecialValue[slot]});
    }
    return slots;
}

// The slots of the item's option definitions.
std::vector<OptionSlot> DefinitionSlots(const ITEM& item)
{
    std::vector<OptionSlot> slots;
    for (int slot = 0; slot < item.SpecialNum; ++slot)
    {
        if (GameLogic::Items::GetOptionSource(item, slot).has_value())
        {
            slots.push_back({item.Special[slot], item.SpecialValue[slot]});
        }
    }
    return slots;
}

ITEM MakeItemWithOption(int type, int level, int optionType = 0)
{
    ItemCreationParams params{};
    params.Group = type / MAX_ITEM_INDEX;
    params.Number = type % MAX_ITEM_INDEX;
    params.WithOption = level > 0;
    params.OptionLevel = static_cast<BYTE>(level);
    params.OptionType = static_cast<BYTE>(optionType);
    ITEM item;
    FillItemFromParameters(item, params);
    return item;
}
} // namespace

// The additional option comes from the definition the item links (Data/Items/Options/Option.json) with the slot and
// value the code before the definitions gave it, at every level the server can send up to 7: the item's whole slot
// list is the one without an option level and the old code's option after it. The wings keep their option code.
TEST_CASE("Every item's additional option comes from its option links as the code before the definitions gave it "
          "[data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;

    int itemsWithOption = 0;
    for (int type = 0; type < MAX_ITEM; ++type)
    {
        if (g_ItemDatabase.Find(type) == nullptr || type == ITEM_HORN_OF_DINORANT)
        {
            continue;
        }
        CAPTURE(type);
        const bool optionInCode = g_ItemDatabase.GetSlot(type) == Data::Items::ItemSlot::Wings;
        const std::vector<OptionSlot> withoutOption = AllSlots(MakeItemWithOption(type, 0));
        for (int level = 1; level <= 7; ++level)
        {
            CAPTURE(level);
            const ITEM item = MakeItemWithOption(type, level);
            const std::optional<OptionSlot> expected = OldAdditionalOption(type, level);
            REQUIRE(DefinitionSlots(item).size() == (expected ? 1u : 0u));
            if (!optionInCode)
            {
                std::vector<OptionSlot> expectedSlots = withoutOption;
                if (expected)
                {
                    expectedSlots.push_back(*expected);
                }
                CHECK(AllSlots(item) == expectedSlots);
            }
            itemsWithOption += expected && level == 1 ? 1 : 0;
        }
        // Without a level, no option.
        CHECK(DefinitionSlots(MakeItemWithOption(type, 0)).empty());
    }
    CHECK(itemsWithOption > 400);
}

// Above the list (a server that sends more than Season 6 has) the last value counts; a longer list gives such a server
// its own values.
TEST_CASE("An additional option above its list has the list's last value [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;

    for (int level = 5; level <= 7; ++level)
    {
        CAPTURE(level);
        const std::vector<OptionSlot> kris = DefinitionSlots(MakeItemWithOption(ITEM_KRIS, level));
        REQUIRE(kris.size() == 1);
        CHECK(kris.front().value == 16);
    }
    const std::vector<OptionSlot> ring = DefinitionSlots(MakeItemWithOption(ITEM_RING_OF_ICE, 4));
    REQUIRE(ring.size() == 1);
    CHECK(ring.front().value == 3);
}

// A magic sword's one option shows two lines, the damage and the wizardry damage; the rings' lines show the per cent.
TEST_CASE("The additional option's tooltip lines come from its texts [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;
    REQUIRE(g_ItemDatabase.HasTag(ITEM_RUNE_BLADE, Data::Items::ItemTag::MagicSword));

    const ITEM sword = MakeItemWithOption(ITEM_RUNE_BLADE, 4);
    REQUIRE(sword.SpecialNum == 1);
    CHECK(sword.Special[0] == AT_IMPROVE_DAMAGE);
    CHECK(WriteDefinitionOptionLines(sword, 0, 0, MaxTextListRows) == 2);
    CHECK(std::wstring(TextList[0]) == L"Additional Dmg +16");
    CHECK(std::wstring(TextList[1]) == L"Additional Wizardry Dmg +16");

    const ITEM ring = MakeItemWithOption(ITEM_RING_OF_ICE, 3);
    REQUIRE(ring.SpecialNum == 1);
    CHECK(WriteDefinitionOptionLines(ring, 0, 0, MaxTextListRows) == 1);
    CHECK(std::wstring(TextList[0]) == L"Automatic HP recovery 3%");

    const ITEM pendant = MakeItemWithOption(ITEM_PENDANT_OF_ABILITY, 2);
    CHECK(WriteDefinitionOptionLines(pendant, 0, 0, MaxTextListRows) == 1);
    CHECK(std::wstring(TextList[0]) == L"Max AG increased by 2%");

    // Loch's Feather had a hidden line for the level the server sent; it links no option now.
    CHECK(MakeItemWithOption(ITEM_LOCHS_FEATHER, 2).SpecialNum == 0);
}

// The option type the server sends picks the option of that number, as OpenMU reads it: the additional options are
// numbered 0, so another type turns none of them on.
TEST_CASE("An additional option turns on only for its number [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;

    CHECK(DefinitionSlots(MakeItemWithOption(ITEM_KRIS, 2, 0)).size() == 1);
    for (int optionType = 1; optionType <= 3; ++optionType)
    {
        CAPTURE(optionType);
        CHECK(DefinitionSlots(MakeItemWithOption(ITEM_KRIS, 2, optionType)).empty());
        CHECK(DefinitionSlots(MakeItemWithOption(ITEM_RING_OF_ICE, 2, optionType)).empty());
    }
}

// The wings' and the Horn of Dinorant's additional options are still code, which leaves out an item that links an
// option definition, so one option level never gives two options.
TEST_CASE("The code gives an additional option only to an item without one in data [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;

    CHECK(GameLogic::Items::LinksOptionType(ITEM_KRIS, Data::Items::ItemOptionType::Option));
    CHECK(GameLogic::Items::LinksOptionType(ITEM_KRIS, Data::Items::ItemOptionType::Luck));
    CHECK_FALSE(GameLogic::Items::LinksOptionType(ITEM_WINGS_OF_SATAN, Data::Items::ItemOptionType::Option));
    CHECK(MakeItemWithOption(ITEM_KRIS, 2).SpecialNum == 1);
    CHECK(MakeItemWithOption(ITEM_WINGS_OF_SATAN, 2).SpecialNum == 1);
}

// "+Option" in the name and the ground label, and the tooltip state skillLuckOrOption, follow whether the item has an
// additional option, not the option level the server sends.
TEST_CASE("Only an item with an additional option counts as one [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;

    CHECK(GameLogic::Items::HasAdditionalOption(MakeItemWithOption(ITEM_KRIS, 1)));
    CHECK(GameLogic::Items::HasAdditionalOption(MakeItemWithOption(ITEM_RING_OF_ICE, 1)));
    CHECK(GameLogic::Items::HasAdditionalOption(MakeItemWithOption(ITEM_WINGS_OF_SATAN, 1)));
    CHECK(GameLogic::Items::HasAdditionalOption(MakeItemWithOption(ITEM_HORN_OF_DINORANT, 1)));
    CHECK_FALSE(GameLogic::Items::HasAdditionalOption(MakeItemWithOption(ITEM_KRIS, 0)));
    CHECK_FALSE(GameLogic::Items::HasAdditionalOption(MakeItemWithOption(ITEM_KRIS, 1, 2)));
    CHECK_FALSE(GameLogic::Items::HasAdditionalOption(MakeItemWithOption(ITEM_LOCHS_FEATHER, 1)));
}

// The stat code adds a magic sword's damage option to the wizardry damage too, by the option's effect, not by the
// sword's tag.
TEST_CASE("A magic sword's damage option raises the wizardry damage by its effect [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;

    CHECK(GameLogic::Items::DamageOptionRaisesWizardry(MakeItemWithOption(ITEM_RUNE_BLADE, 1)));
    CHECK_FALSE(GameLogic::Items::DamageOptionRaisesWizardry(MakeItemWithOption(ITEM_RUNE_BLADE, 0)));
    CHECK_FALSE(GameLogic::Items::DamageOptionRaisesWizardry(MakeItemWithOption(ITEM_KRIS, 4)));
    CHECK_FALSE(GameLogic::Items::DamageOptionRaisesWizardry(MakeItemWithOption(ITEM_WINGS_OF_SATAN, 4)));
}

// The strength requirement grows by the level the additional option counts at, as OpenMU adds it for the option the
// item has: nothing without one, the list's last level above the list.
TEST_CASE("The strength requirement grows by the additional option's counted level [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;

    const ITEM plain = MakeItemWithOption(ITEM_KRIS, 0);
    const ITEM levelTwo = MakeItemWithOption(ITEM_KRIS, 2);
    REQUIRE(levelTwo.RequireStrength > plain.RequireStrength);
    CHECK(GameLogic::Items::GetAdditionalOptionLevel(levelTwo) == 2);
    // An option type that turns no option on adds no strength.
    CHECK(GameLogic::Items::GetAdditionalOptionLevel(MakeItemWithOption(ITEM_KRIS, 2, 2)) == 0);
    CHECK(MakeItemWithOption(ITEM_KRIS, 2, 2).RequireStrength == plain.RequireStrength);
    // Above the list the last level counts.
    CHECK(GameLogic::Items::GetAdditionalOptionLevel(MakeItemWithOption(ITEM_KRIS, 7)) == 4);
    CHECK(MakeItemWithOption(ITEM_KRIS, 7).RequireStrength == MakeItemWithOption(ITEM_KRIS, 4).RequireStrength);
    // The wings keep their option level, their option still code.
    CHECK(GameLogic::Items::GetAdditionalOptionLevel(MakeItemWithOption(ITEM_WINGS_OF_SATAN, 3)) == 3);
    CHECK(GameLogic::Items::GetAdditionalOptionLevel(MakeItemWithOption(ITEM_LOCHS_FEATHER, 3)) == 0);
    // The Horn of Dinorant's option level is three option flags, no level; it still shows "+Option".
    CHECK(GameLogic::Items::GetAdditionalOptionLevel(MakeItemWithOption(ITEM_HORN_OF_DINORANT, 7)) == 0);
    CHECK(GameLogic::Items::HasAdditionalOption(MakeItemWithOption(ITEM_HORN_OF_DINORANT, 7)));
}

// An option with one value for every level (no list) counts at the option level the server sends, also above 4.
TEST_CASE("An additional option with a constant value counts at its option level [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;

    const ItemOptionsScope options;
    const Data::Items::ItemOptionDatabase& shipped = options.Saved();
    std::vector<Data::Items::ItemOptionDefinition> definitions(shipped.GetDefinitions().begin(),
                                                              shipped.GetDefinitions().end());
    const int physical = shipped.Find("physicalDamageOption");
    REQUIRE(physical != Data::Items::ItemOptionDatabase::NoDefinition);
    definitions[static_cast<size_t>(physical)].options[0].value = {{6}, false};
    g_ItemDatabase.SetOptions(Data::Items::ItemOptionDatabase(std::move(definitions), shipped.GetTexts()));

    CHECK(GameLogic::Items::CountAdditionalOptionLevel(ITEM_KRIS, 1, 0) == 1);
    CHECK(GameLogic::Items::CountAdditionalOptionLevel(ITEM_KRIS, 7, 0) == 7);
    CHECK(GameLogic::Items::CountAdditionalOptionLevel(ITEM_KRIS, 7, 2) == 0);
    const std::vector<OptionSlot> slots = DefinitionSlots(MakeItemWithOption(ITEM_KRIS, 7));
    REQUIRE(slots.size() == 1);
    CHECK(slots.front().value == 6);
}

namespace
{
// The value with its terms' attributes resolved, as the option database has it.
int ComputeResolved(Data::Items::ItemOptionValue value, int level)
{
    Data::Items::ResolveTermAttributes(value);
    return GameLogic::Items::ComputeOptionValue(value, level);
}
} // namespace

// A value's terms read the hero's attributes when the value is asked: level / 20 of a level 85 hero is 4, rounded
// down; level plus master level with the master level, which only a master hero has.
TEST_CASE("An option value adds its terms on the hero's attributes, each rounded down [data][items]")
{
    HeroScope hero;
    hero.Hero().Level = 85;
    hero.Hero().Class = CLASS_GRANDMASTER;
    const MASTER_LEVEL_VALUE savedMasterLevel = Master_Level_Data;
    Master_Level_Data.nMLevel = 30;

    Data::Items::ItemOptionValue value{{0}, false, {{"level", 0, 20}}};
    CHECK(ComputeResolved(value, 1) == 4);
    value.per = {{"totalLevel", 0, 20}};
    CHECK(ComputeResolved(value, 1) == 5);
    value.per = {{"masterLevel", 2, 0}, {"level", 0, 20}};
    CHECK(ComputeResolved(value, 1) == 64);
    // A list's entry plus the terms; an attribute the client doesn't read adds nothing.
    const Data::Items::ItemOptionValue list{{4, 8}, true, {{"horseLevel", 1, 0}, {"level", 0, 40}}};
    CHECK(ComputeResolved(list, 2) == 10);
    // A value with terms stops at what an option slot holds.
    value.per = {{"level", 1000, 0}};
    CHECK(ComputeResolved(value, 1) == 65535);

    // A hero who isn't a master has no master level, also when the server sent one for a hero before.
    hero.Hero().Class = CLASS_WIZARD;
    value.per = {{"totalLevel", 0, 20}};
    CHECK(ComputeResolved(value, 1) == 4);
    value.per = {{"masterLevel", 2, 0}};
    CHECK(ComputeResolved(value, 1) == 0);

    // Picking a character forgets the hero before's master level until the server sends the new hero's.
    hero.Hero().Class = CLASS_GRANDMASTER;
    gCharacterManager.ForgetHeroMasterLevel();
    value.per = {{"totalLevel", 0, 20}};
    CHECK(ComputeResolved(value, 1) == 4);

    Master_Level_Data = savedMasterLevel;
}

// A loaded term, whose attribute the option database resolved, equals an edited one with the same data.
TEST_CASE("A term compares by its data, not by its resolved attribute [data][items]")
{
    Data::Items::ItemOptionValue loaded{{0}, false, {{"level", 0, 20}}};
    Data::Items::ResolveTermAttributes(loaded);
    const Data::Items::ItemOptionValue edited{{0}, false, {{"level", 0, 20}}};
    CHECK(loaded.per[0].heroAttribute != edited.per[0].heroAttribute);
    CHECK(loaded.per[0] == edited.per[0]);
    CHECK_FALSE(loaded.per[0] == Data::Items::ItemOptionTerm{"level", 0, 25});
}

namespace
{
// The excellent options the code before the definitions gave an item (CalcExcellentOptions up to 9a3, with its fix for
// arrows and bolts), for the hero's level `heroLevel`.
std::vector<OptionSlot> OldExcellentOptions(int type, BYTE flags, int heroLevel)
{
    std::vector<OptionSlot> slots;
    const auto bit = [flags](int index) { return ((flags >> index) & 1) != 0; };
    const bool defensive = (type >= ITEM_SHIELD && type < ITEM_BOOTS + MAX_ITEM_INDEX) ||
                           (type >= ITEM_RING_OF_ICE && type <= ITEM_RING_OF_POISON) ||
                           (type >= ITEM_RING_OF_FIRE && type <= ITEM_RING_OF_MAGIC);
    if (defensive)
    {
        constexpr std::array<WORD, 6> effects{AT_IMPROVE_LIFE,      AT_IMPROVE_MANA,
                                              AT_DECREASE_DAMAGE,   AT_REFLECTION_DAMAGE,
                                              AT_IMPROVE_BLOCKING_PERCENT, AT_IMPROVE_GAIN_GOLD};
        for (int index = 5; index >= 0; --index)
        {
            if (bit(index))
            {
                // The defense rate option writes its 10 since the stat code adds it from the slot.
                const WORD effect = effects[static_cast<size_t>(5 - index)];
                slots.push_back({effect, static_cast<WORD>(effect == AT_IMPROVE_BLOCKING_PERCENT ? 10 : 0)});
            }
        }
    }
    const bool offensive = ((type >= ITEM_SWORD && type < ITEM_STAFF + MAX_ITEM_INDEX) &&
                            !g_ItemDatabase.HasTag(type, Data::Items::ItemTag::Ammunition)) ||
                           (type >= ITEM_PENDANT_OF_LIGHTING && type <= ITEM_PENDANT_OF_FIRE) ||
                           (type >= ITEM_PENDANT_OF_ICE && type <= ITEM_PENDANT_OF_ABILITY);
    if (offensive)
    {
        const bool wizardry = (type >= ITEM_STAFF && type < ITEM_STAFF + MAX_ITEM_INDEX) ||
                              type == ITEM_PENDANT_OF_LIGHTING || type == ITEM_PENDANT_OF_ICE ||
                              type == ITEM_PENDANT_OF_WATER;
        if (bit(5))
        {
            slots.push_back({AT_EXCELLENT_DAMAGE, 0});
        }
        if (bit(4))
        {
            slots.push_back(
                {static_cast<WORD>(wizardry ? AT_IMPROVE_MAGIC_LEVEL : AT_IMPROVE_DAMAGE_LEVEL), static_cast<WORD>(heroLevel / 20)});
        }
        if (bit(3))
        {
            slots.push_back({static_cast<WORD>(wizardry ? AT_IMPROVE_MAGIC_PERCENT : AT_IMPROVE_DAMAGE_PERCENT), 2});
        }
        if (bit(2))
        {
            slots.push_back({AT_IMPROVE_ATTACK_SPEED, 7});
        }
        if (bit(1))
        {
            slots.push_back({AT_IMPROVE_GAIN_LIFE, 0});
        }
        if (bit(0))
        {
            slots.push_back({AT_IMPROVE_GAIN_MANA, 0});
        }
    }
    return slots;
}

ITEM MakeItemWithExcellent(int type, BYTE flags)
{
    ItemCreationParams params{};
    params.Group = type / MAX_ITEM_INDEX;
    params.Number = type % MAX_ITEM_INDEX;
    params.HasExcellentOption = flags != 0;
    params.ExcellentFlags = flags;
    ITEM item;
    FillItemFromParameters(item, params);
    return item;
}

// The slots of the item's excellent definitions.
std::vector<OptionSlot> ExcellentDefinitionSlots(const ITEM& item)
{
    std::vector<OptionSlot> slots;
    for (int slot = 0; slot < item.SpecialNum; ++slot)
    {
        const std::optional<GameLogic::Items::SlotOption> option = GameLogic::Items::FindSlotOption(item, slot);
        if (option && option->definition->type == Data::Items::ItemOptionType::Excellent)
        {
            slots.push_back({item.Special[slot], item.SpecialValue[slot]});
        }
    }
    return slots;
}
} // namespace

// Every item's excellent options come from the definitions it links (Data/Items/Options/Excellent.json) with the
// slots, values and order (bit 5 down to bit 0) the code before the definitions gave it, for every excellent byte; an
// item whose excellent byte nothing else reads (no wing, no Horn of Fenrir) has the whole slot list it had.
TEST_CASE("Every item's excellent options come from its option links as the code before the definitions gave them "
          "[data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;
    hero.Hero().Level = 85;

    int itemsWithExcellent = 0;
    for (int type = 0; type < MAX_ITEM; ++type)
    {
        if (g_ItemDatabase.Find(type) == nullptr)
        {
            continue;
        }
        CAPTURE(type);
        const bool byteReadElsewhere =
            g_ItemDatabase.GetSlot(type) == Data::Items::ItemSlot::Wings || type == ITEM_HORN_OF_FENRIR;
        const std::vector<OptionSlot> withoutExcellent = AllSlots(MakeItemWithExcellent(type, 0));
        for (int flags = 1; flags < 64; ++flags)
        {
            CAPTURE(flags);
            const ITEM item = MakeItemWithExcellent(type, static_cast<BYTE>(flags));
            const std::vector<OptionSlot> expected = OldExcellentOptions(type, static_cast<BYTE>(flags), 85);
            REQUIRE(ExcellentDefinitionSlots(item) == expected);
            if (!byteReadElsewhere)
            {
                std::vector<OptionSlot> expectedSlots = withoutExcellent;
                expectedSlots.insert(expectedSlots.end(), expected.begin(), expected.end());
                CHECK(AllSlots(item) == expectedSlots);
            }
        }
        itemsWithExcellent += OldExcellentOptions(type, 63, 85).empty() ? 0 : 1;
    }
    CHECK(itemsWithExcellent > 400);
}

// The excellent lines come from their texts, with the definition's value where the code before wrote the number into
// the text.
TEST_CASE("The excellent options' tooltip lines come from their texts [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;
    hero.Hero().Level = 85;

    const ITEM shield = MakeItemWithExcellent(ITEM_SHIELD, 0x21);
    REQUIRE(shield.SpecialNum == 2);
    CHECK(WriteDefinitionOptionLines(shield, 0, 0, MaxTextListRows) == 1);
    CHECK(std::wstring(TextList[0]) == L"Increase Max HP +4%");
    CHECK(WriteDefinitionOptionLines(shield, 1, 0, MaxTextListRows) == 1);
    CHECK(std::wstring(TextList[0]) == L"Increases acquisition rate of Zen after hunting monsters +30%");

    const ITEM kris = MakeItemWithExcellent(ITEM_KRIS, 0x1C);
    REQUIRE(kris.SpecialNum == 3);
    CHECK(WriteDefinitionOptionLines(kris, 0, 0, MaxTextListRows) == 1);
    CHECK(std::wstring(TextList[0]) == L"Increase Damage +level/20");
    CHECK(WriteDefinitionOptionLines(kris, 1, 0, MaxTextListRows) == 1);
    CHECK(std::wstring(TextList[0]) == L"Increase Damage +2%");
    CHECK(WriteDefinitionOptionLines(kris, 2, 0, MaxTextListRows) == 1);
    CHECK(std::wstring(TextList[0]) == L"Increase Attacking(Wizardry)speed +7");

    // Arrows and bolts link none.
    CHECK(MakeItemWithExcellent(ITEM_ARROWS, 63).SpecialNum == 0);

    // The formula's numbers come from the definition: the term's divisor and the after-kill option's value.
    const ITEM pendant = MakeItemWithExcellent(ITEM_PENDANT_OF_FIRE, 0x01);
    REQUIRE(pendant.SpecialNum >= 1);
    CHECK(WriteDefinitionOptionLines(pendant, pendant.SpecialNum - 1, 0, MaxTextListRows) == 1);
    CHECK(std::wstring(TextList[0]) == L"Increases acquisition rate of Mana after hunting monsters +Mana/8");
    ItemOptionsScope restoreOptions;
    const Data::Items::ItemOptionDatabase& shipped = restoreOptions.Saved();
    std::vector<Data::Items::ItemOptionDefinition> definitions(shipped.GetDefinitions().begin(),
                                                              shipped.GetDefinitions().end());
    const int physical = shipped.Find("excellentPhysicalAttack");
    REQUIRE(physical != Data::Items::ItemOptionDatabase::NoDefinition);
    for (Data::Items::ItemOption& option : definitions[static_cast<size_t>(physical)].options)
    {
        if (option.number == 5)
        {
            option.value.per[0].divisor = 25;
        }
    }
    g_ItemDatabase.SetOptions(Data::Items::ItemOptionDatabase(std::move(definitions), shipped.GetTexts()));
    const ITEM changed = MakeItemWithExcellent(ITEM_KRIS, 0x10);
    REQUIRE(changed.SpecialNum >= 1);
    CHECK(WriteDefinitionOptionLines(changed, changed.SpecialNum - 1, 0, MaxTextListRows) == 1);
    CHECK(std::wstring(TextList[0]) == L"Increase Damage +level/25");
}

// The stat code adds an option with terms on the hero's attributes as the hero is now, not as the hero was when the
// item was made: level / 20 grows with the hero. The other options add what the item was made with.
TEST_CASE("The stat code adds the excellent level / 20 as the hero is now [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;
    hero.Hero().Level = 15;

    ITEM kris = MakeItemWithExcellent(ITEM_KRIS, 0x1C);
    kris.Durability = 20;
    REQUIRE(kris.Special[0] == AT_IMPROVE_DAMAGE_LEVEL);
    CHECK(kris.SpecialValue[0] == 0);
    CHECK(GameLogic::Items::GetCurrentSlotValue(kris, 0) == 0);
    hero.Hero().Level = 85;
    CHECK(GameLogic::Items::GetCurrentSlotValue(kris, 0) == 4);
    CHECK(kris.SpecialValue[0] == 0);
    // The others: what the item was made with (2 % and attack speed 7).
    CHECK(GameLogic::Items::GetCurrentSlotValue(kris, 1) == 2);
    CHECK(GameLogic::Items::GetCurrentSlotValue(kris, 2) == 7);
    // A defensive option the server computes writes no value and adds none.
    const ITEM shield = MakeItemWithExcellent(ITEM_SMALL_SHIELD, 0x20);
    CHECK(GameLogic::Items::GetCurrentSlotValue(shield, 0) == 0);
}

// The stats add the excellent per cent options (damage 2 %, wizardry 2 %, defense rate 10 %) with the values of their
// definitions, so a changed value changes the stats, not only the tooltip.
TEST_CASE("The stats add the excellent per cent options with their definitions' values [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;
    hero.HeroCharacter().Class = CLASS_KNIGHT;
    hero.Hero().Class = CLASS_KNIGHT;
    hero.Hero().Level = 400;
    hero.Hero().Strength = hero.Hero().Dexterity = hero.Hero().Energy = hero.Hero().Vitality = 5000;

    ITEM shield = MakeItemWithExcellent(ITEM_SMALL_SHIELD, 0x02);
    shield.Durability = 20;
    WORD blocking = 200;
    PlusSpecialPercentOfSlot(&blocking, AT_IMPROVE_BLOCKING_PERCENT, &shield);
    CHECK(blocking == 220);
    ITEM kris = MakeItemWithExcellent(ITEM_KRIS, 0x08);
    kris.Durability = 20;
    WORD damage = 200;
    PlusSpecialPercentOfSlot(&damage, AT_IMPROVE_DAMAGE_PERCENT, &kris);
    CHECK(damage == 204);

    ItemOptionsScope restoreOptions;
    const Data::Items::ItemOptionDatabase& shipped = restoreOptions.Saved();
    std::vector<Data::Items::ItemOptionDefinition> definitions(shipped.GetDefinitions().begin(),
                                                              shipped.GetDefinitions().end());
    for (const char* name : {"excellentDefense", "excellentPhysicalAttack"})
    {
        const int found = shipped.Find(name);
        REQUIRE(found != Data::Items::ItemOptionDatabase::NoDefinition);
        for (Data::Items::ItemOption& option : definitions[static_cast<size_t>(found)].options)
        {
            if (option.number == 2 || option.number == 4)
            {
                option.value = {{20}, false};
            }
        }
    }
    g_ItemDatabase.SetOptions(Data::Items::ItemOptionDatabase(std::move(definitions), shipped.GetTexts()));
    shield = MakeItemWithExcellent(ITEM_SMALL_SHIELD, 0x02);
    shield.Durability = 20;
    blocking = 200;
    PlusSpecialPercentOfSlot(&blocking, AT_IMPROVE_BLOCKING_PERCENT, &shield);
    CHECK(blocking == 240);
    kris = MakeItemWithExcellent(ITEM_KRIS, 0x08);
    kris.Durability = 20;
    damage = 200;
    PlusSpecialPercentOfSlot(&damage, AT_IMPROVE_DAMAGE_PERCENT, &kris);
    CHECK(damage == 240);

    // A stat the per cent would carry past what it holds stops there instead of wrapping around.
    damage = 60000;
    PlusSpecialPercentOfSlot(&damage, AT_IMPROVE_DAMAGE_PERCENT, &kris);
    CHECK(damage == 65535);
}

// A ring's wizardry per cent comes from each of its own slots, also when a linked option comes before the slots its
// code gives: the Wizard's Ring with the excellent wizardry +2 % linked has the 2 % in slot 0 and its own 10 % in slot 1.
TEST_CASE("A ring's wizardry per cent comes from its own option slots [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;
    hero.HeroCharacter().Class = CLASS_WIZARD;
    hero.Hero().Class = CLASS_WIZARD;
    hero.Hero().Level = 400;
    hero.Hero().Strength = hero.Hero().Dexterity = hero.Hero().Energy = hero.Hero().Vitality = 5000;

    const Data::Items::ItemDefinition shipped = *g_ItemDatabase.Find(ITEM_WIZARDS_RING);
    Data::Items::ItemDefinition linked = shipped;
    linked.options.push_back("excellentWizardryAttack");
    g_ItemDatabase.Set(linked);
    ITEM ring = MakeItemWithExcellent(ITEM_WIZARDS_RING, 0x08);
    g_ItemDatabase.Set(shipped);
    ring.Durability = 20;
    REQUIRE(ring.SpecialNum >= 2);
    CHECK(ring.Special[0] == AT_IMPROVE_MAGIC_PERCENT);
    CHECK(GameLogic::Items::GetCurrentSlotValue(ring, 0) == 2);
    CHECK(ring.Special[1] == AT_IMPROVE_MAGIC_PERCENT);
    CHECK(ring.SpecialValue[1] == 10);

    // The magic damage with and without the ring: 900 energy gives 100 to 225; the ring adds 2 %, then 10 %.
    const auto magicDamage = [](const ITEM* worn)
    {
        auto machine = std::make_unique<CHARACTER_MACHINE>();
        for (ITEM& equipped : machine->Equipment)
        {
            equipped = ITEM{};
            equipped.Type = -1;
        }
        if (worn != nullptr)
        {
            machine->Equipment[EQUIPMENT_RING_RIGHT] = *worn;
        }
        machine->Character.Energy = 900;
        machine->CalculateMagicDamage();
        return std::make_pair(machine->Character.MagicDamageMin, machine->Character.MagicDamageMax);
    };
    const auto [plainMin, plainMax] = magicDamage(nullptr);
    const auto [ringMin, ringMax] = magicDamage(&ring);
    CHECK(ringMin - plainMin == 12);
    CHECK(ringMax - plainMax == 26);
}

// "Excellent" is having an active excellent option: an item that links an excellent definition with an option its byte
// turns on. Wings with wing options and the Horn of Fenrir with a kind use the byte for something else and are not
// excellent by it.
TEST_CASE("An item is excellent when its excellent byte turns on an option it links [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    HeroScope hero;

    CHECK(GameLogic::Items::HasExcellentOptions(ITEM_KRIS, 0x01));
    CHECK(GameLogic::Items::HasExcellentOptions(ITEM_RING_OF_ICE, 0x20));
    CHECK(GameLogic::Items::HasExcellentOptions(MakeItemWithExcellent(ITEM_SMALL_SHIELD, 0x04)));
    CHECK_FALSE(GameLogic::Items::HasExcellentOptions(ITEM_KRIS, 0));
    CHECK_FALSE(GameLogic::Items::HasExcellentOptions(ITEM_WINGS_OF_SPIRITS, 0x01));
    CHECK_FALSE(GameLogic::Items::HasExcellentOptions(ITEM_HORN_OF_FENRIR, 0x02));
    CHECK_FALSE(GameLogic::Items::HasExcellentOptions(ITEM_TRANSFORMATION_RING, 0x3F));
    CHECK_FALSE(GameLogic::Items::HasExcellentOptions(ITEM_ARROWS, 0x3F));

    // The excellent +15 durability: an excellent item, not a wing with options or the Horn of Fenrir with a kind.
    const auto durability = [](int type, BYTE flags)
    {
        const ITEM item = MakeItemWithExcellent(type, flags);
        return CalcMaxDurability(&item, &ItemAttribute[type], 0);
    };
    CHECK(durability(ITEM_KRIS, 0x01) == durability(ITEM_KRIS, 0) + 15);
    CHECK(durability(ITEM_HORN_OF_FENRIR, 0x02) == durability(ITEM_HORN_OF_FENRIR, 0));
    CHECK(durability(ITEM_WINGS_OF_SPIRITS, 0x01) == durability(ITEM_WINGS_OF_SPIRITS, 0));
}


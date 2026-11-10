#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "HeroScope.h"
#include "TooltipDataScope.h"

#include "Core/Utilities/StringUtils.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Combat/CriticalDamageBonus.h"
#include "GameLogic/Combat/CriticalDamageRate.h"
#include "GameLogic/Items/CSItemOption.h"
#include "I18N/All.h"
#include "Network/Server/SocketSystem.h"
#include "UI/Character/CriticalDamageRateLine.h"
#include "UI/Character/CriticalHitTexts.h"
#include "UI/Character/DamageHint.h"
#include "UI/Core/UIManager.h"
#include "UI/Inventory/UIJewelHarmony.h"
#include "UI/Inventory/ItemMng.h"

#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

using GameLogic::Combat::CriticalDamageBonus;
using GameLogic::Combat::CriticalDamageRate;
using GameLogic::Combat::GetHeroCriticalDamageBonus;
using GameLogic::Combat::GetHeroCriticalDamageRate;

// The Lightning seed sphere option "Critical damage rate": its seed id in the item's socket byte (OpenMU: the
// Lightning option 3, CriticalDamageChance +8 % to +12 % by sphere level).
constexpr BYTE CriticalRateSeed = 32;
// The Lightning options "Critical damage" and "Excellent damage" (OpenMU's options 2 and 0).
constexpr BYTE CriticalDamageSeed = 31;
constexpr BYTE ExcellentDamageSeed = 29;
// The weapons' harmony option "Increase critical damage".
constexpr BYTE WeaponCriticalDamageHarmony = 6;
// The staffs' harmony option "Increase critical damage".
constexpr BYTE StaffCriticalDamageHarmony = 5;

// A Blade Knight who meets every item's requirements.
void MakeStrongKnight(HeroScope& hero)
{
    hero.Hero().Class = CLASS_BLADEKNIGHT;
    hero.HeroCharacter().Class = CLASS_BLADEKNIGHT;
    hero.Hero().Level = 400;
    hero.Hero().Strength = 3000;
    hero.Hero().Dexterity = 3000;
    hero.Hero().Vitality = 3000;
    hero.Hero().Energy = 3000;
    hero.Hero().Charisma = 3000;
}

ItemCreationParams Item(int group, int number, bool luck = false)
{
    ItemCreationParams params{};
    params.Group = group;
    params.Number = number;
    params.Durability = 20;
    params.WithLuck = luck;
    params.SocketBonusOption = SOCKET_EMPTY;
    for (BYTE& socket : params.SocketOptions)
    {
        socket = SOCKET_EMPTY;
    }
    return params;
}

// A piece of the Warrior set (the ancient Leather set, OpenMU: the full set adds 5 % critical damage chance).
ItemCreationParams WarriorPiece(int group, int number, bool luck)
{
    ItemCreationParams params = Item(group, number, luck);
    params.AncientDiscriminator = 1;
    return params;
}

// A hero of the test's own (Hero()) who wears only what the test puts on, in a character machine of its own. When the
// scope goes, the previous hero and machine are back, and the set and socket bonuses are added up again for them.
class EquipmentScope
{
public:
    EquipmentScope() : m_previous(CharacterMachine), m_machine(std::make_unique<CHARACTER_MACHINE>())
    {
        m_hero.emplace();
        TakeOffAll();
        CharacterMachine = m_machine.get();
    }

    // Without a previous machine the bonuses are added up for nothing worn, with the test's hero; otherwise for the
    // previous machine with the previous hero (set options check the hero's class and stats).
    ~EquipmentScope()
    {
        if (m_previous == nullptr)
        {
            TakeOffAll();
            AddUpBonuses();
        }
        CharacterMachine = m_previous;
        m_hero.reset();
        if (m_previous != nullptr && CharacterAttribute != nullptr && ::Hero != nullptr)
        {
            AddUpBonuses();
        }
    }

    EquipmentScope(const EquipmentScope&) = delete;
    EquipmentScope& operator=(const EquipmentScope&) = delete;

    HeroScope& Hero()
    {
        return *m_hero;
    }

    ITEM& Wear(int slot, const ItemCreationParams& params)
    {
        ITEM& item = m_machine->Equipment[slot];
        FillItemFromParameters(item, params);
        return item;
    }

    // What the game does when the equipment changes.
    static void AddUpBonuses()
    {
        g_csItemOption.CheckItemSetOptions();
        g_SocketItemMgr.CalcSocketStatusBonus();
    }

private:
    // Empty slots as the game has them: no item, no durability, no requirements. A slot keeping a worn item's
    // durability and requirements passed the set options' checks and read ItemAttribute[-1].
    void TakeOffAll()
    {
        for (ITEM& item : m_machine->Equipment)
        {
            item = ITEM{};
            item.Type = -1;
        }
    }

    CHARACTER_MACHINE* m_previous;
    std::unique_ptr<CHARACTER_MACHINE> m_machine;
    std::optional<HeroScope> m_hero;
};

std::vector<std::string> Texts(const std::vector<UI::RmlBridge::Tooltip::Line>& lines)
{
    std::vector<std::string> texts;
    for (const UI::RmlBridge::Tooltip::Line& line : lines)
    {
        texts.push_back(line.text);
    }
    return texts;
}
} // namespace

// Each worn item's luck adds its value (Data/Items/Options/Luck.json: 5), the full Warrior set its set option and a
// Lightning seed sphere its socket option, as the server adds them up into the critical damage chance.
TEST_CASE("The critical damage rate adds up the worn items' luck, set options and socket options [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    EquipmentScope equipment;
    HeroScope& hero = equipment.Hero();
    MakeStrongKnight(hero);

    CHECK(GetHeroCriticalDamageRate().Total() == 0);

    // The Warrior set: the Leather pieces, the Morning Star and the Ring of Ice; luck on the armor's five pieces.
    equipment.Wear(EQUIPMENT_HELM, WarriorPiece(7, 5, true));
    equipment.Wear(EQUIPMENT_ARMOR, WarriorPiece(8, 5, true));
    equipment.Wear(EQUIPMENT_PANTS, WarriorPiece(9, 5, true));
    equipment.Wear(EQUIPMENT_GLOVES, WarriorPiece(10, 5, true));
    equipment.Wear(EQUIPMENT_BOOTS, WarriorPiece(11, 5, true));
    equipment.Wear(EQUIPMENT_WEAPON_RIGHT, WarriorPiece(2, 1, false));
    equipment.Wear(EQUIPMENT_RING_RIGHT, WarriorPiece(13, 8, false));
    EquipmentScope::AddUpBonuses();

    CriticalDamageRate rate = GetHeroCriticalDamageRate();
    CHECK(rate.luckItems == 5);
    CHECK(rate.luck == 25);
    CHECK(rate.setOptions == 5);
    CHECK(rate.socketOptions == 0);
    CHECK(rate.Total() == 30);

    // A Flamberge with the seed sphere in its first socket at level 1 and in its second at level 5, in the left hand.
    ItemCreationParams flamberge = Item(0, 26);
    flamberge.SocketCount = 2;
    flamberge.SocketOptions[0] = CriticalRateSeed;
    flamberge.SocketOptions[1] = CriticalRateSeed + 4 * SEASON4A::MAX_SOCKET_OPTION;
    REQUIRE(g_SocketItemMgr.IsSocketItem(&equipment.Wear(EQUIPMENT_WEAPON_LEFT, flamberge)));
    EquipmentScope::AddUpBonuses();

    rate = GetHeroCriticalDamageRate();
    CHECK(rate.socketOptions == 8 + 12);
    CHECK(rate.Total() == 25 + 5 + 20);
    CHECK(rate.buffSkill == AT_SKILL_UNDEFINED);
}

// The server drops a broken item's options, and nobody wears an item without meeting its requirements.
TEST_CASE("An item's luck counts while it is not broken and the hero can wear it [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    EquipmentScope equipment;
    HeroScope& hero = equipment.Hero();
    MakeStrongKnight(hero);

    ITEM& helm = equipment.Wear(EQUIPMENT_HELM, Item(7, 5, true));
    equipment.Wear(EQUIPMENT_BOOTS, Item(11, 5, true));
    CHECK(GetHeroCriticalDamageRate().luck == 10);

    helm.Durability = 0;
    CHECK(GetHeroCriticalDamageRate().luckItems == 1);
    helm.Durability = 20;

    hero.Hero().Strength = 0;
    CHECK(GetHeroCriticalDamageRate().luckItems == 0);
}

// Only the hero's own mastered buff counts, with its master skill's value as the server sent it; a buff from a party
// member has a value the client doesn't know.
TEST_CASE("The hero's own mastered buff adds its master skill's value to the critical damage rate [data][items]")
{
    // The Scroll of Battle's value is in the item option table (ItemAddOption.bmd).
    ClientDataScope client(ClientDirectory);
    TooltipDataScope data;
    EquipmentScope equipment;
    HeroScope& hero = equipment.Hero();

    hero.HeroCharacter().Object.m_BuffMap.RegisterBuff(EFFECT_MAGIC_CIRCLE_ENHANCED);
    CHECK(GetHeroCriticalDamageRate().buffSkill == AT_SKILL_UNDEFINED);

    hero.Hero().MasterSkillInfo[AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY] = CSkillTreeInfo(20, 9.016f, 9.4f);
    CriticalDamageRate rate = GetHeroCriticalDamageRate();
    CHECK(rate.buffSkill == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY);
    CHECK(rate.buff == doctest::Approx(9.016f));
    CHECK(rate.Buff() == 9);
    CHECK(rate.Total() == 9);

    hero.HeroCharacter().Object.m_BuffMap.UnRegisterBuff(EFFECT_MAGIC_CIRCLE_ENHANCED);
    CHECK(GetHeroCriticalDamageRate().Buff() == 0);

    // The Dark Lord's Critical Damage Increase with its third power-up.
    hero.HeroCharacter().Object.m_BuffMap.RegisterBuff(EFFECT_GREATER_CRITICAL_DAMAGE_MASTERED);
    hero.Hero().MasterSkillInfo[AT_SKILL_ADD_CRITICAL_STR3] = CSkillTreeInfo(1, 1.806f, 2.1f);
    rate = GetHeroCriticalDamageRate();
    CHECK(rate.buffSkill == AT_SKILL_ADD_CRITICAL_STR3);
    CHECK(rate.Buff() == 2);
    hero.HeroCharacter().Object.m_BuffMap.UnRegisterBuff(EFFECT_GREATER_CRITICAL_DAMAGE_MASTERED);

    // The Scroll of Battle adds its 20 % to the buffs' part; the hero reads it, so it is always the hero's own.
    hero.HeroCharacter().Object.m_BuffMap.RegisterBuff(eBuff_Scroll_Battle);
    rate = GetHeroCriticalDamageRate();
    CHECK(rate.scrollOfBattle == 20);
    CHECK(rate.MasterSkillBuff() == 0);
    CHECK(rate.Buff() == 20);
    CHECK(rate.Total() == 20);
    hero.HeroCharacter().Object.m_BuffMap.UnRegisterBuff(eBuff_Scroll_Battle);
}

// The line shows what the worn items add, and what the own buff adds in brackets; its hint lists every part above 0.
TEST_CASE("The character window's critical damage rate line and its hint [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;

    CriticalDamageRate rate;
    UI::Character::CriticalDamageRateLine line = UI::Character::BuildCriticalDamageRateLine(rate);
    CHECK(line.text == "Critical damage rate: 0%");
    CHECK(line.source == "base");
    CHECK(Texts(UI::Character::BuildCriticalDamageRateHint(rate)) ==
          std::vector<std::string>{"Critical damage rate", "Total: 0%"});

    rate.luckItems = 3;
    rate.luck = 15;
    rate.setOptions = 10;
    rate.socketOptions = 8;
    line = UI::Character::BuildCriticalDamageRateLine(rate);
    CHECK(line.text == "Critical damage rate: 33%");
    CHECK(line.source == "boosted");

    rate.buffSkill = AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY;
    rate.buff = 9.016f;
    line = UI::Character::BuildCriticalDamageRateLine(rate);
    CHECK(line.text == "Critical damage rate: 33% (+9%)");
    CHECK(line.source == "buffed");

    const std::string buffName = StringUtils::WideToNarrow(SkillAttribute[AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY].Name);
    REQUIRE_FALSE(buffName.empty());
    const std::vector<UI::RmlBridge::Tooltip::Line> hint = UI::Character::BuildCriticalDamageRateHint(rate);
    CHECK(Texts(hint) == std::vector<std::string>{"Critical damage rate",
                                                  "Luck (\xC3\x97"
                                                  "3): 15%",
                                                  "Set options: 10%", "Socket options: 8%", buffName + ": +9%",
                                                  "Total: 42%"});
    CHECK(hint.front().bold);
    CHECK(hint.back().bold);

    rate.scrollOfBattle = 20;
    CHECK(UI::Character::BuildCriticalDamageRateLine(rate).text == "Critical damage rate: 33% (+29%)");
    CHECK(Texts(UI::Character::BuildCriticalDamageRateHint(rate)) ==
          std::vector<std::string>{"Critical damage rate",
                                   "Luck (\xC3\x97"
                                   "3): 15%",
                                   "Set options: 10%", "Socket options: 8%", buffName + ": +9%",
                                   "Scroll of Battle: +20%", "Total: 62%"});

    // A missing name shows the skill's number or the item's group and number, so the parts stay apart.
    const wchar_t skillName = SkillAttribute[AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY].Name[0];
    const wchar_t scrollName = ItemAttribute[ITEM_SCROLL_OF_BATTLE].Name[0];
    SkillAttribute[AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY].Name[0] = L'\0';
    ItemAttribute[ITEM_SCROLL_OF_BATTLE].Name[0] = L'\0';
    const std::vector<std::string> unnamed = Texts(UI::Character::BuildCriticalDamageRateHint(rate));
    SkillAttribute[AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY].Name[0] = skillName;
    ItemAttribute[ITEM_SCROLL_OF_BATTLE].Name[0] = scrollName;
    CHECK(unnamed[4] == "#383: +9%");
    CHECK(unnamed[5] == "14/97: +20%");

    I18N::SetLocale("de");
    CHECK(UI::Character::BuildCriticalDamageRateLine(rate).text == "Kritische Schadensrate: 33% (+29%)");
    I18N::SetLocale("en");
}

// The Cloud set (the Brass Helm's set 1 and the Brass Pants' set 2) adds 50 and 20 %, a weapon's harmony option
// "Increase critical damage" its value while the item's level reaches the option's, and the Lightning seed sphere
// "Critical damage" its value (the client's 30 at level 1; OpenMU has 15, a known difference). The excellent damage
// bonus has its own seed sphere option.
TEST_CASE("The critical damage bonus adds up the worn items' set, harmony and socket options [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    EquipmentScope equipment;
    HeroScope& hero = equipment.Hero();
    MakeStrongKnight(hero);

    CHECK(GetHeroCriticalDamageBonus().Total() == 0);
    CHECK(GameLogic::Combat::GetHeroExcellentDamageBonus() == 0);

    ItemCreationParams helm = Item(7, 8);
    helm.AncientDiscriminator = 1;
    ItemCreationParams pants = Item(9, 8);
    pants.AncientDiscriminator = 2;
    equipment.Wear(EQUIPMENT_HELM, helm);
    equipment.Wear(EQUIPMENT_PANTS, pants);
    EquipmentScope::AddUpBonuses();
    CHECK(GetHeroCriticalDamageBonus().setOptions == 50);
    CHECK(GetHeroCriticalDamageRate().setOptions == 20);

    // A Kris +13 with the harmony option at its top level (the option has values from a higher level on).
    ItemCreationParams kris = Item(0, 0);
    kris.Level = 13;
    kris.HasHarmonyOption = true;
    kris.HarmonyOptionType = WeaponCriticalDamageHarmony;
    kris.HarmonyOptionLevel = 13;
    const HARMONYJEWELOPTION& harmony =
        g_pUIJewelHarmonyinfo->GetHarmonyJewelOptionInfo(SI_Weapon, WeaponCriticalDamageHarmony);
    std::wstring name = harmony.Name;
    std::transform(name.begin(), name.end(), name.begin(), [](wchar_t c) { return std::towlower(c); });
    CHECK(name.find(L"critical") != std::wstring::npos);
    REQUIRE(harmony.HarmonyJewelLevel[13] > 0);
    ITEM& weapon = equipment.Wear(EQUIPMENT_WEAPON_RIGHT, kris);
    CHECK(GetHeroCriticalDamageBonus().harmonyOptions == harmony.HarmonyJewelLevel[13]);
    weapon.Level = 12;
    CHECK(GetHeroCriticalDamageBonus().harmonyOptions == 0);
    // An item +15 with the option at level 15, past the table's levels 0 to 13.
    weapon.Level = 15;
    weapon.Jewel_Of_Harmony_OptionLevel = 15;
    CHECK(GetHeroCriticalDamageBonus().harmonyOptions == 0);

    ItemCreationParams flamberge = Item(0, 26);
    flamberge.SocketCount = 2;
    flamberge.SocketOptions[0] = CriticalDamageSeed;
    flamberge.SocketOptions[1] = ExcellentDamageSeed;
    ITEM& sockets = equipment.Wear(EQUIPMENT_WEAPON_LEFT, flamberge);
    EquipmentScope::AddUpBonuses();
    CHECK(GetHeroCriticalDamageBonus().socketOptions == 30);
    CHECK(GameLogic::Combat::GetHeroExcellentDamageBonus() == g_SocketItemMgr.GetSocketOptionValue(&sockets, 1));
    CHECK(GameLogic::Combat::GetHeroExcellentDamageBonus() > 0);

    // A broken item adds no socket options, as on the server.
    sockets.Durability = 0;
    EquipmentScope::AddUpBonuses();
    CHECK(GetHeroCriticalDamageBonus().socketOptions == 0);
    CHECK(GameLogic::Combat::GetHeroExcellentDamageBonus() == 0);
}

// A staff's harmony option "Increase critical damage" is the staffs' option 5, not the weapons' 6.
TEST_CASE("A staff's harmony option adds critical damage like a weapon's [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    EquipmentScope equipment;
    HeroScope& hero = equipment.Hero();
    MakeStrongKnight(hero);
    hero.Hero().Class = CLASS_SOULMASTER;
    hero.HeroCharacter().Class = CLASS_SOULMASTER;

    const HARMONYJEWELOPTION& harmony =
        g_pUIJewelHarmonyinfo->GetHarmonyJewelOptionInfo(SI_Staff, StaffCriticalDamageHarmony);
    std::wstring name = harmony.Name;
    std::transform(name.begin(), name.end(), name.begin(), [](wchar_t c) { return std::towlower(c); });
    CHECK(name.find(L"critical") != std::wstring::npos);
    REQUIRE(harmony.HarmonyJewelLevel[13] > 0);

    // A Skull Staff +13 with the option at its top level.
    ItemCreationParams staff = Item(5, 0);
    staff.Level = 13;
    staff.HasHarmonyOption = true;
    staff.HarmonyOptionType = StaffCriticalDamageHarmony;
    staff.HarmonyOptionLevel = 13;
    ITEM& worn = equipment.Wear(EQUIPMENT_WEAPON_RIGHT, staff);
    CHECK(GetHeroCriticalDamageBonus().harmonyOptions == harmony.HarmonyJewelLevel[13]);

    // The item's kind picks the table: a staff's option 6 and a weapon's option 5 are other options.
    worn.Jewel_Of_Harmony_Option = WeaponCriticalDamageHarmony;
    CHECK(GetHeroCriticalDamageBonus().harmonyOptions == 0);
    hero.Hero().Class = CLASS_BLADEKNIGHT;
    hero.HeroCharacter().Class = CLASS_BLADEKNIGHT;
    ItemCreationParams kris = Item(0, 0);
    kris.Level = 13;
    kris.HasHarmonyOption = true;
    kris.HarmonyOptionType = StaffCriticalDamageHarmony;
    kris.HarmonyOptionLevel = 13;
    equipment.Wear(EQUIPMENT_WEAPON_RIGHT, kris);
    CHECK(GetHeroCriticalDamageBonus().harmonyOptions == 0);
}

// Increase Critical Damage adds 1 per 30 energy and 1 per 25 command of the one who cast it, and its first power-up's
// value. Only the hero's own buff counts: a party member's adds what that member's stats give.
TEST_CASE("The hero's own Increase Critical Damage adds to every critical hit [data][items]")
{
    EquipmentScope equipment;
    HeroScope& hero = equipment.Hero();
    hero.Hero().Class = CLASS_DARK_LORD;
    hero.HeroCharacter().Class = CLASS_DARK_LORD;
    hero.Hero().Energy = 2000;
    hero.Hero().Charisma = 2000;
    hero.HeroCharacter().Object.m_BuffMap.RegisterBuff(eBuff_AddCriticalDamage);
    CHECK(GetHeroCriticalDamageBonus().Buff() == 0);

    hero.Hero().Skill[0] = AT_SKILL_ADD_CRITICAL;
    hero.Hero().SkillNumber = 1;
    CriticalDamageBonus bonus = GetHeroCriticalDamageBonus();
    CHECK(bonus.buffSkill == AT_SKILL_ADD_CRITICAL);
    CHECK(bonus.Buff() == 146);
    CHECK(bonus.Total() == 146);

    hero.Hero().MasterSkillInfo[AT_SKILL_ADD_CRITICAL_STR1] = CSkillTreeInfo(1, 5.5f, 6.f);
    CHECK(GetHeroCriticalDamageBonus().Buff() == 152);

    hero.HeroCharacter().Object.m_BuffMap.UnRegisterBuff(eBuff_AddCriticalDamage);
    CHECK(GetHeroCriticalDamageBonus().Buff() == 0);
}

// The damage line's hint: the hits before the target's defense, then the critical damage's parts.
TEST_CASE("The character window's damage line hint [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    TooltipDataScope data;
    const UI::Character::DamageRange range{1909, 2575};

    CriticalDamageBonus bonus;
    CHECK(Texts(UI::Character::BuildDamageHint(range, bonus, 0)) ==
          std::vector<std::string>{"Damage", "Normal hit: 1909~2575", "Critical hit: 2575", "Excellent hit: 3090",
                                   "before the target's defense"});

    bonus.buffSkill = AT_SKILL_ADD_CRITICAL;
    bonus.buff = 146.67f;
    const std::string buffName = StringUtils::WideToNarrow(SkillAttribute[AT_SKILL_ADD_CRITICAL].Name);
    REQUIRE_FALSE(buffName.empty());
    std::vector<UI::RmlBridge::Tooltip::Line> hint = UI::Character::BuildDamageHint(range, bonus, 0);
    CHECK(Texts(hint) == std::vector<std::string>{"Damage", "Normal hit: 1909~2575", "Critical hit: 2721",
                                                  "Excellent hit: 3236", "before the target's defense", "",
                                                  "Critical damage", buffName + ": +146", "Total: +146"});
    CHECK(hint[2].color == UI::RmlBridge::Tooltip::LineColor::Purple);
    CHECK(hint[5].kind == UI::RmlBridge::Tooltip::Line::Kind::HalfSpacer);

    bonus = {};
    bonus.setOptions = 50;
    bonus.harmonyOptions = 12;
    bonus.socketOptions = 30;
    hint = UI::Character::BuildDamageHint(range, bonus, 20);
    CHECK(Texts(hint) == std::vector<std::string>{"Damage", "Normal hit: 1909~2575", "Critical hit: 2667",
                                                  "Excellent hit: 3202", "before the target's defense", "",
                                                  "Critical damage", "Set options: +50", "Harmony options: +12",
                                                  "Socket options: +30", "Total: +92"});
    CHECK(hint[2].color == UI::RmlBridge::Tooltip::LineColor::White);
}

// The window's critical hit texts are formatted again only when what they show changes: the rate, the critical and
// excellent damage bonus, the damage range, the language or a name they show.
TEST_CASE("The critical hit texts are built again only when what they show changes [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    UI::Character::CriticalHitTexts texts;
    UI::Character::CriticalHitTexts::Parts parts;
    parts.damageRange = {1909, 2575};
    parts.locale = "en";

    CHECK(texts.Update(parts));
    CHECK(texts.RateLine().text == "Critical damage rate: 0%");
    CHECK_FALSE(texts.Update(parts));

    parts.rate.luckItems = 1;
    parts.rate.luck = 5;
    CHECK(texts.Update(parts));
    CHECK(texts.RateLine().text == "Critical damage rate: 5%");
    CHECK(Texts(texts.RateHintLines()).back() == "Total: 5%");

    parts.bonus.setOptions = 50;
    CHECK(texts.Update(parts));
    CHECK(Texts(texts.DamageHintLines())[2] == "Critical hit: 2625");
    parts.excellentBonus = 20;
    CHECK(texts.Update(parts));
    CHECK(Texts(texts.DamageHintLines())[3] == "Excellent hit: 3160");
    parts.damageRange.maximum = 2600;
    CHECK(texts.Update(parts));
    CHECK(Texts(texts.DamageHintLines())[1] == "Normal hit: 1909~2600");

    I18N::SetLocale("de");
    parts.locale = "de";
    CHECK(texts.Update(parts));
    CHECK(texts.RateLine().text == "Kritische Schadensrate: 5%");
    I18N::SetLocale("en");
    parts.locale = "en";
    CHECK(texts.Update(parts));

    // A buff's skill renamed (the skill editor) and the Scroll of Battle renamed ("Translate item names", the item
    // editor): the tables change, not the parts.
    parts.rate.buffSkill = AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY;
    parts.rate.buff = 9.f;
    CHECK(texts.Update(parts));
    wchar_t& skillName = SkillAttribute[AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY].Name[0];
    const wchar_t skillLetter = skillName;
    skillName = L'X';
    CHECK(texts.Update(parts));
    CHECK_FALSE(texts.Update(parts));
    skillName = skillLetter;
    CHECK(texts.Update(parts));
    wchar_t& scrollName = ItemAttribute[ITEM_SCROLL_OF_BATTLE].Name[0];
    const wchar_t scrollLetter = scrollName;
    scrollName = L'X';
    CHECK(texts.Update(parts));
    CHECK_FALSE(texts.Update(parts));
    scrollName = scrollLetter;
}

#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "HeroScope.h"
#include "TooltipDataScope.h"

#include "Core/Utilities/StringUtils.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Combat/CriticalDamageRate.h"
#include "GameLogic/Items/CSItemOption.h"
#include "I18N/All.h"
#include "Network/Server/SocketSystem.h"
#include "UI/Character/CriticalDamageRateLine.h"
#include "UI/Inventory/ItemMng.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

using GameLogic::Combat::CriticalDamageRate;
using GameLogic::Combat::GetHeroCriticalDamageRate;

// The Lightning seed sphere option "Critical damage rate": its seed id in the item's socket byte (OpenMU: the
// Lightning option 3, CriticalDamageChance +8 % to +12 % by sphere level).
constexpr BYTE CriticalRateSeed = 32;

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

// The hero wears only what the test puts on, in a character machine of its own. The set and socket bonuses the client
// adds up for it are added up again for nothing worn when the scope goes; make it after the hero's HeroScope.
class EquipmentScope
{
public:
    EquipmentScope() : m_previous(CharacterMachine), m_machine(std::make_unique<CHARACTER_MACHINE>())
    {
        TakeOffAll();
        CharacterMachine = m_machine.get();
    }

    ~EquipmentScope()
    {
        TakeOffAll();
        AddUpBonuses();
        CharacterMachine = m_previous;
    }

    EquipmentScope(const EquipmentScope&) = delete;
    EquipmentScope& operator=(const EquipmentScope&) = delete;

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
    void TakeOffAll()
    {
        for (ITEM& item : m_machine->Equipment)
        {
            item.Type = -1;
        }
    }

    CHARACTER_MACHINE* m_previous;
    std::unique_ptr<CHARACTER_MACHINE> m_machine;
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
    HeroScope hero;
    MakeStrongKnight(hero);
    EquipmentScope equipment;

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
    HeroScope hero;
    MakeStrongKnight(hero);
    EquipmentScope equipment;

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
    HeroScope hero;
    EquipmentScope equipment;

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
}

// The line shows what the worn items add, and what the own buff adds in brackets; its hint lists every part above 0.
TEST_CASE("The character window's critical damage rate line and its hint [data][items]")
{
    ClientDataScope client(ClientDirectory);
    I18N::SetLocale("en");
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

    I18N::SetLocale("de");
    CHECK(UI::Character::BuildCriticalDamageRateLine(rate).text == "Kritische Schadensrate: 33% (+9%)");
    I18N::SetLocale("en");
}

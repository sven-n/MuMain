#include "stdafx.h"

#include "GameLogic/Combat/CriticalDamageRate.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Items/CSItemOption.h"
#include "Network/Server/SocketSystem.h"

#include <array>
#include <cmath>

namespace GameLogic::Combat
{
namespace
{
// A mastered buff whose critical damage rate is the value of a master skill of its caster (OpenMU's magic effects
// 139 and 148: their critical damage chance is 0 plus the master skill's value).
struct CriticalRateBuff
{
    eBuffState buff;
    ActionSkillType masterSkill;
};

constexpr std::array<CriticalRateBuff, 2> CriticalRateBuffs{{
    {EFFECT_MAGIC_CIRCLE_ENHANCED, AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY},
    {EFFECT_GREATER_CRITICAL_DAMAGE_MASTERED, AT_SKILL_ADD_CRITICAL_STR3},
}};

// The Scroll of Battle (14/97) adds 20 % while its buff is on (OpenMU's BuffItems: CriticalDamageChance +0.2). The
// hero reads it from the inventory, so the buff is always the hero's own.
constexpr int ScrollOfBattleCriticalRate = 20;

void AddWornLuck(CriticalDamageRate& rate)
{
    for (ITEM& item : CharacterMachine->Equipment)
    {
        WORD luck = 0;
        PlusSpecial(&luck, AT_LUCK, &item);
        if (luck > 0)
        {
            ++rate.luckItems;
            rate.luck += luck;
        }
    }
}

// The hero's own buff: one whose master skill the hero has. The value of a party member's buff depends on that
// member's master skill, which the client doesn't know, so it isn't counted.
void AddOwnBuff(CriticalDamageRate& rate)
{
    if (Hero == nullptr || CharacterAttribute == nullptr)
    {
        return;
    }
    for (const CriticalRateBuff& buff : CriticalRateBuffs)
    {
        const CSkillTreeInfo& masterSkill = CharacterAttribute->MasterSkillInfo[buff.masterSkill];
        if (masterSkill.GetSkillLevel() > 0 && g_isCharacterBuff((&Hero->Object), buff.buff))
        {
            rate.buffSkill = buff.masterSkill;
            rate.buff = masterSkill.GetSkillValue();
            return;
        }
    }
}

void AddScrollOfBattle(CriticalDamageRate& rate)
{
    if (Hero != nullptr && g_isCharacterBuff((&Hero->Object), eBuff_Scroll_Battle))
    {
        rate.scrollOfBattle = ScrollOfBattleCriticalRate;
    }
}
} // namespace

int CriticalDamageRate::MasterSkillBuff() const
{
    return static_cast<int>(std::lround(buff));
}

CriticalDamageRate GetHeroCriticalDamageRate()
{
    CriticalDamageRate rate;
    if (CharacterMachine == nullptr)
    {
        return rate;
    }
    AddWornLuck(rate);
    rate.setOptions = g_csItemOption.GetSetOptionValue(AT_SET_OPTION_IMPROVE_CRITICAL_DAMAGE_PERCENT);
    rate.socketOptions = g_SocketItemMgr.m_StatusBonus.m_iCriticalDamageRateBonus;
    AddOwnBuff(rate);
    AddScrollOfBattle(rate);
    return rate;
}
} // namespace GameLogic::Combat

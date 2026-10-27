#include "stdafx.h"

#include "GameLogic/Automation/AttackSkillRotation.h"
#include "MUHelper/MuHelperData.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "GameLogic/Skills/SkillManager.h"

#include <tuple>

namespace GameLogic::Automation
{
using MUHelper::ConfigData;

bool IsAttackSkillAvailable(ActionSkillType skill)
{
    if (skill <= AT_SKILL_UNDEFINED || skill >= MAX_SKILLS)
        return false;
    const int slot = gSkillManager.FindSkillIndex(skill);
    if (slot < 0 ||
        (SkillAttribute[CharacterAttribute->Skill[slot]].Delay > 0 && CharacterAttribute->SkillDelay[slot] > 0))
        return false;
    // The normal cast path owns mana/AG checks and automatic potion use.
    // CheckSkillDelay starts a cooldown; selection must not call it.
    return gSkillManager.AreSkillAttributeRequirementsMet(skill) && CheckSkillUseCondition(&Hero->Object, skill);
}

namespace
{
constexpr int AttackSlotCount = static_cast<int>(std::tuple_size_v<decltype(ConfigData::aiSkill)>);

bool IsValidSkill(uint32_t skill)
{
    return skill > AT_SKILL_UNDEFINED && skill < MAX_SKILLS;
}

bool IsAutomaticSlot(const ConfigData& config, int slot)
{
    return IsValidSkill(config.aiSkill[slot]) &&
           (slot == 0 || !(config.aiSkillCondition[slot] & (MUHelper::ON_TIMER | MUHelper::ON_CONDITION)));
}

bool IsConditionMet(uint32_t condition, std::size_t nearby, std::size_t attacking)
{
    std::size_t count = 0;
    if (condition & MUHelper::ON_MOBS_NEARBY)
        count = nearby;
    else if (condition & MUHelper::ON_MOBS_ATTACKING)
        count = attacking;
    else
        return false;

    constexpr std::size_t TwoMobs = 2;
    constexpr std::size_t ThreeMobs = 3;
    constexpr std::size_t FourMobs = 4;
    constexpr std::size_t FiveMobs = 5;
    return ((condition & MUHelper::ON_MORE_THAN_TWO_MOBS) && count >= TwoMobs) ||
           ((condition & MUHelper::ON_MORE_THAN_THREE_MOBS) && count >= ThreeMobs) ||
           ((condition & MUHelper::ON_MORE_THAN_FOUR_MOBS) && count >= FourMobs) ||
           ((condition & MUHelper::ON_MORE_THAN_FIVE_MOBS) && count >= FiveMobs);
}

ActionSkillType SelectTriggeredSkill(const ConfigData& config, int secondsElapsed, std::size_t nearby,
                                     std::size_t attacking)
{
    for (int slot = 1; slot < static_cast<int>(config.aiSkill.size()); ++slot)
    {
        if (!IsValidSkill(config.aiSkill[slot]))
            continue;

        const auto condition = config.aiSkillCondition[slot];
        const auto interval = config.aiSkillInterval[slot];
        const bool timerDue =
            (condition & MUHelper::ON_TIMER) && interval != 0 && secondsElapsed > 0 && secondsElapsed % interval == 0;
        const bool conditionMet = (condition & MUHelper::ON_CONDITION) && IsConditionMet(condition, nearby, attacking);
        if (timerDue || conditionMet)
            return static_cast<ActionSkillType>(config.aiSkill[slot]);
    }
    return AT_SKILL_UNDEFINED;
}
} // namespace

void AttackSkillRotation::Reset()
{
    m_nextSlot = 0;
    m_selectedSlot = -1;
    m_notReadyAttempts = 0;
}

ActionSkillType AttackSkillRotation::Select(const ConfigData& config, int secondsElapsed, std::size_t nearby,
                                            std::size_t attacking)
{
    const int previousSlot = m_selectedSlot;
    m_selectedSlot = -1;
    const auto triggered = SelectTriggeredSkill(config, secondsElapsed, nearby, attacking);
    if (triggered > AT_SKILL_UNDEFINED)
        return triggered;

    bool hasAutomaticExtra = false;
    for (int slot = 1; slot < AttackSlotCount; ++slot)
        hasAutomaticExtra |= IsAutomaticSlot(config, slot);

    if (hasAutomaticExtra)
    {
        for (int offset = 0; offset < AttackSlotCount; ++offset)
        {
            const int slot = (m_nextSlot + offset) % AttackSlotCount;
            if (!IsAutomaticSlot(config, slot))
                continue;

            const auto skill = static_cast<ActionSkillType>(config.aiSkill[slot]);
            if (!IsAttackSkillAvailable(skill))
                continue;

            m_selectedSlot = slot;
            if (previousSlot != slot)
                m_notReadyAttempts = 0;
            return skill;
        }
    }

    // Keep legacy low-resource handling and basic-attack fallback when none is ready.
    return IsValidSkill(config.aiSkill[0]) ? static_cast<ActionSkillType>(config.aiSkill[0]) : AT_SKILL_UNDEFINED;
}

void AttackSkillRotation::Complete(GameLogic::Automation::SkillResult result)
{
    using GameLogic::Automation::SkillResult;
    if (m_selectedSlot < 0)
        return;

    // Select can skip the starting cursor. Retain the slot actually chosen,
    // including across a lost target and the walk/animation before a cast.
    m_nextSlot = m_selectedSlot;
    constexpr int MaxNotReadyAttempts = 4;
    const bool retryExhausted = result == SkillResult::NotReady && ++m_notReadyAttempts >= MaxNotReadyAttempts;
    if (result != SkillResult::NotReady)
        m_notReadyAttempts = 0;
    if (result == SkillResult::Cast || result == SkillResult::Refused || retryExhausted)
    {
        m_nextSlot = (m_selectedSlot + 1) % AttackSlotCount;
        m_notReadyAttempts = 0;
    }
}
} // namespace GameLogic::Automation

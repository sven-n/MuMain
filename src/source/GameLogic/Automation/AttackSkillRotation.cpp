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
static_assert(AttackSlotCount == AttackSkillRotation::SlotCount);
constexpr int MaxNotReadyAttempts = 4;
constexpr int ConditionYieldSeconds = 1;

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

} // namespace

int AttackSkillRotation::SelectTriggeredSlot(const ConfigData& config, int secondsElapsed, std::size_t nearby,
                                             std::size_t attacking)
{
    // A deadline, not a due second: a second spans several ticks, and a busy one must not lose the activation.
    for (int slot = 1; slot < AttackSlotCount; ++slot)
    {
        if ((config.aiSkillCondition[slot] & MUHelper::ON_TIMER) && config.aiSkillInterval[slot] > 0 &&
            m_timerDueAt[slot] == 0)
            m_timerDueAt[slot] = secondsElapsed + static_cast<int>(config.aiSkillInterval[slot]);
    }

    for (int slot = 1; slot < AttackSlotCount; ++slot)
    {
        const auto skill = config.aiSkill[slot];
        if (!IsValidSkill(skill))
            continue;

        const auto condition = config.aiSkillCondition[slot];
        const auto interval = static_cast<int>(config.aiSkillInterval[slot]);
        const bool timerDue = (condition & MUHelper::ON_TIMER) && interval > 0 && secondsElapsed >= m_timerDueAt[slot];
        const bool conditionMet = (condition & MUHelper::ON_CONDITION) &&
                                  secondsElapsed >= m_conditionHeldUntil[slot] &&
                                  IsConditionMet(condition, nearby, attacking);
        if ((timerDue || conditionMet) && IsAttackSkillAvailable(static_cast<ActionSkillType>(skill)))
        {
            m_triggeredSlot = slot;
            m_timerSlotDue = timerDue ? slot : -1;
            m_timerInterval = interval;
            m_selectSeconds = secondsElapsed;
            return slot;
        }
    }
    return -1;
}

void AttackSkillRotation::Reset()
{
    m_nextSlot = 0;
    m_selectedSlot = -1;
    m_notReadyAttempts = 0;
    m_timerDueAt.fill(0);
    m_conditionHeldUntil.fill(0);
    m_triggeredSlot = -1;
    m_timerSlotDue = -1;
    m_triggeredNotReadyAttempts = 0;
}

ActionSkillType AttackSkillRotation::Select(const ConfigData& config, int secondsElapsed, std::size_t nearby,
                                            std::size_t attacking)
{
    const int previousSlot = m_selectedSlot;
    m_selectedSlot = -1;
    m_triggeredSlot = -1;
    m_timerSlotDue = -1;
    const int triggered = SelectTriggeredSlot(config, secondsElapsed, nearby, attacking);
    if (triggered >= 0)
        return static_cast<ActionSkillType>(config.aiSkill[triggered]);

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
    // A triggered cast that is refused or keeps producing no action must not claim every tick: a timer waits
    // for its next interval, and a condition yields to the rotation for a helper second. Mana and AG are only
    // checked by the cast itself, so a resource-starved skill still passes selection.
    if (m_triggeredSlot >= 0)
    {
        const bool retryExhausted =
            result == SkillResult::NotReady && ++m_triggeredNotReadyAttempts >= MaxNotReadyAttempts;
        if (result == SkillResult::Cast || result == SkillResult::Refused || retryExhausted)
        {
            if (m_timerSlotDue >= 0)
                m_timerDueAt[m_timerSlotDue] = m_selectSeconds + m_timerInterval;
            if (result != SkillResult::Cast)
                m_conditionHeldUntil[m_triggeredSlot] = m_selectSeconds + ConditionYieldSeconds;
            m_triggeredNotReadyAttempts = 0;
        }
    }
    m_triggeredSlot = -1;
    m_timerSlotDue = -1;
    if (m_selectedSlot < 0)
        return;

    // Select can skip the starting cursor. Retain the slot actually chosen,
    // including across a lost target and the walk/animation before a cast.
    m_nextSlot = m_selectedSlot;
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

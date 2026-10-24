#include "stdafx.h"

#include "MUHelper/AttackSkillRotation.h"
#include "MUHelper/MuHelperData.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "GameLogic/Skills/SkillManager.h"

#include <algorithm>
#include <iterator>
#include <span>

namespace MUHelper
{
bool IsAttackSkillReady(ActionSkillType skill)
{
    if (skill <= AT_SKILL_UNDEFINED || skill >= MAX_SKILLS)
        return false;

    // Nova's charging action uses the learned Nova slot, as in the skill bar.
    const auto learnedSkill = skill == AT_SKILL_NOVA_BEGIN ? AT_SKILL_NOVA : skill;
    const auto skills = std::span(CharacterAttribute->Skill, MAX_MAGIC);
    const auto found = std::find(skills.begin(), skills.end(), learnedSkill);
    if (found == skills.end() || CharacterAttribute->SkillDelay[std::distance(skills.begin(), found)] > 0)
        return false;

    int mana = 0;
    int ag = 0;
    gSkillManager.GetSkillInformation(skill, 1, nullptr, &mana, nullptr, &ag);
    if (CharacterAttribute->Mana < mana || CharacterAttribute->SkillMana < ag)
        return false;

    // CheckSkillDelay writes a cooldown; the ordinary cast path alone owns it.
    return gSkillManager.AreSkillAttributeRequirementsMet(skill) && CheckSkillUseCondition(&Hero->Object, skill);
}

namespace
{
bool IsValidSkill(uint32_t skill)
{
    return skill > AT_SKILL_UNDEFINED && skill < MAX_SKILLS;
}

bool IsAutomaticSlot(const ConfigData& config, int slot)
{
    return slot == 0 || !(config.aiSkillCondition[slot] & (ON_TIMER | ON_CONDITION));
}

bool IsConditionMet(uint32_t condition, std::size_t nearby, std::size_t attacking)
{
    std::size_t count = 0;
    if (condition & ON_MOBS_NEARBY)
        count = nearby;
    else if (condition & ON_MOBS_ATTACKING)
        count = attacking;
    else
        return false;

    constexpr std::size_t TwoMobs = 2;
    constexpr std::size_t ThreeMobs = 3;
    constexpr std::size_t FourMobs = 4;
    constexpr std::size_t FiveMobs = 5;
    return ((condition & ON_MORE_THAN_TWO_MOBS) && count >= TwoMobs) ||
           ((condition & ON_MORE_THAN_THREE_MOBS) && count >= ThreeMobs) ||
           ((condition & ON_MORE_THAN_FOUR_MOBS) && count >= FourMobs) ||
           ((condition & ON_MORE_THAN_FIVE_MOBS) && count >= FiveMobs);
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
            (condition & ON_TIMER) && interval != 0 && secondsElapsed > 0 && secondsElapsed % interval == 0;
        const bool conditionMet = (condition & ON_CONDITION) && IsConditionMet(condition, nearby, attacking);
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
}

ActionSkillType AttackSkillRotation::Select(const ConfigData& config, int secondsElapsed, std::size_t nearby,
                                            std::size_t attacking)
{
    m_selectedSlot = -1;
    m_slotCount = static_cast<int>(config.aiSkill.size());
    const auto triggered = SelectTriggeredSkill(config, secondsElapsed, nearby, attacking);
    if (triggered > AT_SKILL_UNDEFINED)
        return triggered;

    bool hasAutomaticExtra = false;
    for (int slot = 1; slot < m_slotCount; ++slot)
        hasAutomaticExtra |= IsAutomaticSlot(config, slot) && IsValidSkill(config.aiSkill[slot]);

    if (hasAutomaticExtra)
    {
        for (int offset = 0; offset < m_slotCount; ++offset)
        {
            const int slot = (m_nextSlot + offset) % m_slotCount;
            if (!IsAutomaticSlot(config, slot) || !IsValidSkill(config.aiSkill[slot]))
                continue;

            const auto skill = static_cast<ActionSkillType>(config.aiSkill[slot]);
            if (!IsAttackSkillReady(skill))
                continue;

            m_selectedSlot = slot;
            return skill;
        }
    }

    // Keep legacy low-resource handling and basic-attack fallback when none is ready.
    return config.aiSkill[0] > 0 ? static_cast<ActionSkillType>(config.aiSkill[0]) : AT_SKILL_UNDEFINED;
}

void AttackSkillRotation::Complete(GameLogic::Automation::SkillResult result)
{
    using GameLogic::Automation::SkillResult;
    if (m_selectedSlot < 0 || result == SkillResult::Busy || result == SkillResult::Approaching)
        return;

    m_nextSlot = (m_selectedSlot + 1) % m_slotCount;
}
} // namespace MUHelper

#pragma once

#include "GameLogic/Automation/Skill.h"

#include <cstddef>

namespace MUHelper
{
struct _PetAttackConfig;

// Read-only eligibility: selection must not start a skill's cooldown.
bool IsAttackSkillReady(ActionSkillType skill);

class AttackSkillRotation
{
public:
    void Reset();
    ActionSkillType Select(const _PetAttackConfig& config, int secondsElapsed, std::size_t nearby,
                           std::size_t attacking);
    void Complete(GameLogic::Automation::SkillResult result);

private:
    int m_nextSlot = 0;
    int m_selectedSlot = -1;
    int m_slotCount = 0;
};
} // namespace MUHelper

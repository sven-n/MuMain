#pragma once

#include "GameLogic/Automation/Skill.h"

#include <cstddef>

namespace MUHelper
{
struct _PetAttackConfig;
}

namespace GameLogic::Automation
{

// Read-only availability. Resource checks, including automatic mana potion use,
// belong to the ordinary execution path, which alone starts cooldowns.
bool IsAttackSkillAvailable(ActionSkillType skill);

class AttackSkillRotation
{
public:
    void Reset();
    ActionSkillType Select(const MUHelper::_PetAttackConfig& config, int secondsElapsed, std::size_t nearby,
                           std::size_t attacking);
    void Complete(GameLogic::Automation::SkillResult result);

private:
    int m_nextSlot = 0;
    int m_selectedSlot = -1;
    int m_notReadyAttempts = 0;
};
} // namespace GameLogic::Automation

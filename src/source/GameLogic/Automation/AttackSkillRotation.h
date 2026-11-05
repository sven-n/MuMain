#pragma once

#include "GameLogic/Automation/Skill.h"

#include <array>
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

    // Matches MUHelper::ConfigData::aiSkill; checked where that type is complete.
    static constexpr int SlotCount = 3;

private:
    int SelectTriggeredSlot(const MUHelper::_PetAttackConfig& config, int secondsElapsed, std::size_t nearby,
                            std::size_t attacking);

    int m_nextSlot = 0;
    int m_selectedSlot = -1;
    int m_notReadyAttempts = 0;
    // Helper second at which each timed slot is next due; 0 until first scheduled.
    std::array<int, SlotCount> m_timerDueAt{};
    // Helper second until which a refused condition slot leaves the turn to the rotation.
    std::array<int, SlotCount> m_conditionHeldUntil{};
    int m_triggeredSlot = -1;
    int m_timerSlotDue = -1;
    int m_triggeredNotReadyAttempts = 0;
    int m_timerInterval = 0;
    int m_selectSeconds = 0;
};
} // namespace GameLogic::Automation

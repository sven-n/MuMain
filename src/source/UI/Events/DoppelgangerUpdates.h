#pragma once

#include <cstdint>
#include <span>

namespace UI::Doppelganger
{
struct PartyMemberPosition
{
    std::uint16_t userIndex;
    std::uint8_t positionIndex;
};

void OpenEntry(std::uint8_t remainingTime);
void SetEntryLocked(bool locked);
void SetMonsterPosition(std::uint8_t positionIndex);
void ShowMatchFrame();
void SetIcewalkerPosition(bool present, std::uint8_t positionIndex);
void UpdateParty(std::uint16_t remainingSeconds, std::span<const PartyMemberPosition> members);
void FinishMatch(bool succeeded);
void SetMonsterGoal(std::uint8_t maximum, std::uint8_t entered);
}

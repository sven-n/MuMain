#pragma once

#include <cstdint>

namespace UI::EmpireGuardian
{
void SetEntryInfo(int day, int zone, int remainingTick);
void UpdateTimer(int type, int remainingTick, int monsterCount);
void ShowZoneCleared();
void ShowFinalReward(int experience);
void HideTimer();
}

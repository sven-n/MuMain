#include "stdafx.h"

#include "UI/Events/DoppelgangerUpdates.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"

namespace UI::Doppelganger
{
void OpenEntry(std::uint8_t remainingTime)
{
    UI::Windows::Show(mu::ui::window::INTERFACE_DOPPELGANGER_NPC);
    g_pDoppelGangerWindow->SetRemainTime(remainingTime);
}

void SetEntryLocked(bool locked)
{
    g_pDoppelGangerWindow->LockEnterButton(locked ? TRUE : FALSE);
}

void SetMonsterPosition(std::uint8_t positionIndex)
{
    g_pDoppelGangerFrame->SetMonsterGauge(static_cast<float>(positionIndex) / 22.0f);
}

void ShowMatchFrame()
{
    UI::Windows::Show(mu::ui::window::INTERFACE_DOPPELGANGER_FRAME);
}

void SetIcewalkerPosition(bool present, std::uint8_t positionIndex)
{
    g_pDoppelGangerFrame->SetIceWalkerMap(present ? TRUE : FALSE,
                                          present ? static_cast<float>(22 - positionIndex) / 22.0f : 0.0f);
}

void UpdateParty(std::uint16_t remainingSeconds, std::span<const PartyMemberPosition> members)
{
    g_pDoppelGangerFrame->SetRemainTime(remainingSeconds);
    g_pDoppelGangerFrame->SetPartyMemberRcvd();
    for (const PartyMemberPosition& member : members)
        g_pDoppelGangerFrame->SetPartyMemberInfo(member.userIndex,
                                                  static_cast<float>(22 - member.positionIndex) / 22.0f);
}

void FinishMatch(bool succeeded)
{
    g_pDoppelGangerFrame->StopTimer(TRUE);
    g_pDoppelGangerFrame->EnabledDoppelGangerEvent(FALSE);
    if (succeeded)
        g_pDoppelGangerFrame->SetRemainTime(0);
}

void SetMonsterGoal(std::uint8_t maximum, std::uint8_t entered)
{
    g_pDoppelGangerFrame->SetMaxMonsters(maximum);
    g_pDoppelGangerFrame->SetEnteredMonsters(entered);
}
}

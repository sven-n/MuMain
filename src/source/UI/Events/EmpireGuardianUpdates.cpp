#include "stdafx.h"

#include "UI/Events/EmpireGuardianUpdates.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Dialogs/ConfirmRequest.h"
#include "I18N/All.h"

namespace UI::EmpireGuardian
{
void SetEntryInfo(int day, int zone, int remainingTick)
{
    g_pEmpireGuardianTimer->SetDay(day);
    g_pEmpireGuardianTimer->SetZone(zone);
    g_pEmpireGuardianTimer->SetRemainTime(remainingTick);
}

void UpdateTimer(int type, int remainingTick, int monsterCount)
{
    if (!UI::Windows::IsVisible(mu::ui::window::INTERFACE_EMPIREGUARDIAN_TIMER))
        UI::Windows::Show(mu::ui::window::INTERFACE_EMPIREGUARDIAN_TIMER);

    g_pEmpireGuardianTimer->SetType(type);
    g_pEmpireGuardianTimer->SetRemainTime(remainingTick);
    g_pEmpireGuardianTimer->SetMonsterCount(monsterCount);
}

void ShowZoneCleared()
{
    wchar_t text[256]{};
    UI::Dialogs::ConfirmRequest request;
    mu_swprintf(text, I18N::Game::FortressOfEmpireGuardiansRoundD, g_pEmpireGuardianTimer->GetDay());
    request.lines.push_back({text, false});
    mu_swprintf(text, L"%d%ls", g_pEmpireGuardianTimer->GetZone(), I18N::Game::ZoneCleared);
    request.lines.push_back({text, false});
    UI::Dialogs::ShowConfirm(std::move(request));
}

void ShowFinalReward(int experience)
{
    wchar_t text[256]{};
    UI::Dialogs::ConfirmRequest request;
    mu_swprintf(text, I18N::Game::FortressOfEmpireGuardiansRoundD, g_pEmpireGuardianTimer->GetDay());
    request.lines.push_back({text, false});
    request.lines.push_back({I18N::Game::HasBeenCleared, false});
    mu_swprintf(text, I18N::Game::RewardedExpD, experience);
    request.lines.push_back({text, false});
    UI::Dialogs::ShowConfirm(std::move(request));
}

void HideTimer()
{
    if (UI::Windows::IsVisible(mu::ui::window::INTERFACE_EMPIREGUARDIAN_TIMER))
        UI::Windows::Hide(mu::ui::window::INTERFACE_EMPIREGUARDIAN_TIMER);
}
}

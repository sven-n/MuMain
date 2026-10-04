#include "stdafx.h"

#include "UI/HUD/HudUpdates.h"

#include "UI/Core/WindowSystem.h"
#include "UI/HUD/GensRanking.h"

namespace UI::Hud
{
void SetGensStanding(int contribution, int ranking, int nextContribution)
{
    g_pNewUIGensRanking->SetContribution(contribution);
    g_pNewUIGensRanking->SetRanking(ranking);
    g_pNewUIGensRanking->SetNextContribution(nextContribution);
}
}

#include "stdafx.h"

#include "UI/Events/LuckyCoinUpdates.h"

#include "UI/Core/WindowSystem.h"

namespace UI::LuckyCoin
{
void SetRegistrationCount(int count)
{
    g_pLuckyCoinRegistration->SetRegistCount(count);
}

void UnlockRegistration()
{
    g_pLuckyCoinRegistration->UnLockLuckyCoinRegBtn();
}

void UnlockExchange()
{
    g_pExchangeLuckyCoinWindow->UnLockExchangeBtn();
}
}

#include "stdafx.h"

#include "UI/Events/CryWolfUpdates.h"

#include "UI/Core/WindowSystem.h"

namespace UI::CryWolf
{
void SetCountdown(std::uint8_t hour, std::uint8_t minute)
{
    g_pCryWolfInterface->SetTime(hour, minute);
}
}

#include "stdafx.h"

#include "UI/Events/KanturuUpdates.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"

namespace UI::Kanturu
{
void ShowEntryInfo(Stage stage, Detail detail, bool canEnter, std::uint8_t userCount, int remainingSeconds)
{
    g_pKanturu2ndEnterNpc->ReceiveKanturu3rdInfo(stage, detail, canEnter, userCount, remainingSeconds);
}

void CompleteEntry(EntryResult result)
{
    g_pKanturu2ndEnterNpc->ReceiveKanturu3rdEnter(result);
}

void SetBattleInfoVisible(bool visible)
{
    const bool currentlyVisible = UI::Windows::IsVisible(mu::ui::window::INTERFACE_KANTURU_INFO);
    if (visible && !currentlyVisible)
        UI::Windows::Show(mu::ui::window::INTERFACE_KANTURU_INFO);
    else if (!visible && currentlyVisible)
        UI::Windows::Hide(mu::ui::window::INTERFACE_KANTURU_INFO);
}

void SetBattleTime(std::uint8_t timeLimit)
{
    g_pKanturuInfoWindow->SetTime(timeLimit);
}
}

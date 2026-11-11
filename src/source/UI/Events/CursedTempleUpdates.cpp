#include "stdafx.h"

#include "UI/Events/CursedTempleUpdates.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"
#include "Audio/DSPlaySound.h"

namespace UI::CursedTemple
{
void OpenEntryOffer(std::uint8_t remainingTime, std::uint8_t entryCount)
{
    UI::Windows::Show(mu::ui::window::INTERFACE_CURSEDTEMPLE_NPC);
    g_pCursedTempleEnterWindow->SetEntryOffer(remainingTime, entryCount);
}

void UpdateEntryCounts(std::span<const std::uint8_t, 6> counts)
{
    g_pCursedTempleEnterWindow->SetEntryCounts(counts);
}

void UpdateMatchStatus(const MatchStatus& status)
{
    g_pCursedTempleWindow->SetMatchStatus(status);
}

void ResolveSkill(const SkillResult& result)
{
    g_pCursedTempleWindow->ResolveSkill(result);
}

void EndSkill(std::uint16_t skill, std::uint16_t targetKey)
{
    g_pCursedTempleWindow->EndSkill(skill, targetKey);
}

void SetSkillPoints(std::uint8_t points)
{
    g_pCursedTempleWindow->SetSkillPoints(points);
}

void ShowMatchResult(const MatchResult& result)
{
    UI::Windows::HideAll();
    if (UI::Windows::IsVisible(mu::ui::window::INTERFACE_CURSEDTEMPLE_GAMESYSTEM))
    {
        g_pCursedTempleResultWindow->ResetGameResultInfo();
        g_pCursedTempleResultWindow->SetMyTeam(g_pCursedTempleWindow->GetMyTeam());
        UI::Windows::Hide(mu::ui::window::INTERFACE_CURSEDTEMPLE_GAMESYSTEM);
    }
    PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM5);
    UI::Windows::Show(mu::ui::window::INTERFACE_CURSEDTEMPLE_RESULT);
    g_pCursedTempleResultWindow->SetResult(result);
}

void BeginReadyPhase()
{
    UI::Windows::HideAll();
    g_pCursedTempleWindow->ResetCursedTempleSystemInfo();
    g_pCursedTempleWindow->StartTutorialStep();
    PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM1);
    UI::Windows::Show(mu::ui::window::INTERFACE_CURSEDTEMPLE_GAMESYSTEM);
}
}

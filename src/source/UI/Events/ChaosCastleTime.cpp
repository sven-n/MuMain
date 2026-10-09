
#include "stdafx.h"
#include "World/MapInfra/MapManager.h"
#include "UI/Events/EventPreview.h"
#include "UI/Events/ChaosCastleTime.h"
#include "UI/Core/WindowSystem.h"
#include "GameLogic/Events/MatchEvent.h"
#include "I18N/All.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlTheme.h"
#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;

CChaosCastleTime::CChaosCastleTime()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iTime = 0;
    // The original left the text unset until the first time packet: shown before it, it drew
    // whatever the buffer held.
    m_szTime[0] = L'\0';
    m_iTimeState = CC_TIME_STATE_NORMAL;
    m_iMaxKillMonster = MAX_KILL_MONSTER;
    m_iKilledMonster = 0;
}

CChaosCastleTime::~CChaosCastleTime()
{
    Release();
}

bool CChaosCastleTime::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_CHAOSCASTLE_TIME, this);

    SetPos(x, y);

    m_View.Build();

    Show(false);

    return true;
}

void CChaosCastleTime::Release()
{
    m_View.Release();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CChaosCastleTime::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CChaosCastleTime::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    // The timer takes no pointer events, but its box still holds the pointer, as the original's did.
    if (UI::RmlBridge::IsPointerWithin(m_View.Document() != nullptr ? m_View.Document()->GetElementById("panel") : nullptr))
        return false;

    return true;
}

bool CChaosCastleTime::UpdateKeyEvent()
{
    return true;
}

bool CChaosCastleTime::Update()
{
    if (!IsVisible())
    {
        SyncView();
        return true;
    }

    if (gMapManager.InChaosCastle() == false && !UI::EventPreview::IsShowing(UI::EventPreview::Event::ChaosCastle))
    {
        Show(false);
    }

    SyncView();
    return true;
}

bool CChaosCastleTime::Render()
{
    // Nothing native left: the frame and the texts are RmlUi (SyncView()). Kept because CObject
    // requires the override.
    return true;
}

void CChaosCastleTime::SyncView()
{
    // The original's Render(): the kill count once a target was received, "Time left", the time.
    const bool shown = IsVisible();
    std::wstring kills;
    if (shown)
    {
        if (m_iMaxKillMonster != MAX_KILL_MONSTER)
        {
            wchar_t szText[256] = {};
            mu_swprintf(szText, I18N::Game::CharacterDD, m_iKilledMonster, m_iMaxKillMonster);
            kills = szText;
        }
    }
    // the theme's colours, the time turning imminent under five minutes.
    m_View.Sync(shown, {kills, "normal"}, {I18N::Game::TimeLeft, "normal"},
                {m_szTime, m_iTimeState == CC_TIME_STATE_IMMINENCE ? "imminent" : "normal"});
}

bool CChaosCastleTime::BtnProcess()
{
    return false;
}

float CChaosCastleTime::GetLayerDepth()
{
    return 1.3f;
}

void CChaosCastleTime::OpenningProcess()
{
}

void CChaosCastleTime::ClosingProcess()
{
}

void CChaosCastleTime::SetTime(int iTime)
{
    m_iTime = iTime;

    int iMinute = m_iTime / 60;
    mu_swprintf(m_szTime, L" %.2d:%.2d:%.2d", iMinute, m_iTime % 60, (int)WorldTime % 60);

    if (iMinute < 5)
    {
        m_iTimeState = CC_TIME_STATE_IMMINENCE;
    }
    else
    {
        m_iTimeState = CC_TIME_STATE_NORMAL;
    }
}

void CChaosCastleTime::SetKillMonsterStatue(int iKilled, int iMaxKill)
{
    m_iKilledMonster = iKilled;
    m_iMaxKillMonster = iMaxKill;
}

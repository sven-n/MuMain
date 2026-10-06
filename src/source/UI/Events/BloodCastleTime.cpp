
#include "stdafx.h"
#include "World/MapInfra/MapManager.h"
#include "UI/Events/BloodCastleTime.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "GameLogic/Events/MatchEvent.h"
#include "I18N/All.h"
#include "UI/RmlBridge/RmlTheme.h"

using namespace SEASON3B;
using namespace mu::ui::window;
using namespace matchEvent;

CBloodCastle::CBloodCastle()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iTime = 0;
    // The original left the text unset until the first time packet: shown before it, it drew
    // whatever the buffer held.
    m_szTime[0] = L'\0';
    m_iTimeState = BC_TIME_STATE_NORMAL;
    m_iMaxKillMonster = MAX_KILL_MONSTER;
    m_iKilledMonster = 0;
}

CBloodCastle::~CBloodCastle()
{
    Release();
}

bool CBloodCastle::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_BLOODCASTLE_TIME, this);

    SetPos(x, y);

    m_View.Build();

    Show(false);

    return true;
}

void CBloodCastle::Release()
{
    m_View.Release();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CBloodCastle::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CBloodCastle::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, BLOODCASTLE_TIME_WINDOW_WIDTH, BLOODCASTLE_TIME_WINDOW_HEIGHT).Contains(MouseX, MouseY))
        return false;

    return true;
}

bool CBloodCastle::UpdateKeyEvent()
{
    return true;
}

bool CBloodCastle::Update()
{
    if (!IsVisible())
    {
        SyncView();
        return true;
    }

    if ((g_csMatchInfo == NULL) || (gMapManager.InBloodCastle() == false))
    {
        Show(false);
    }

    SyncView();
    return true;
}

bool CBloodCastle::Render()
{
    // Nothing native left: the frame and the texts are RmlUi (SyncView()). Kept because CObject
    // requires the override.
    return true;
}

void CBloodCastle::SyncView()
{
    // The original's Render(): the kill count once a target was received, "Time left", the time.
    const bool shown = IsVisible() && g_csMatchInfo != NULL;
    std::wstring kills;
    if (shown)
    {
        if (m_iMaxKillMonster != MAX_KILL_MONSTER && g_csMatchInfo != NULL)
        {
            wchar_t szText[256] = {};
            if (g_csMatchInfo->GetMatchType() == 5)
                mu_swprintf(szText, I18N::Game::MagicSkeletonDD, m_iKilledMonster, m_iMaxKillMonster);
            else
                mu_swprintf(szText, I18N::Game::MonsterDD, m_iKilledMonster, m_iMaxKillMonster);
            kills = szText;
        }
    }
    // the theme's colours, the time turning imminent under five minutes.
    m_View.Sync(shown, m_Pos, {kills, "normal"}, {I18N::Game::TimeLeft, "normal"},
                {m_szTime, m_iTimeState == BC_TIME_STATE_IMMINENCE ? "imminent" : "normal"});
}

bool CBloodCastle::BtnProcess()
{
    return false;
}

float CBloodCastle::GetLayerDepth()
{
    return 1.2f;
}

void CBloodCastle::OpenningProcess()
{
}

void CBloodCastle::ClosingProcess()
{
}

void CBloodCastle::SetTime(int iTime)
{
    m_iTime = iTime;

    int iMinute = m_iTime / 60;
    mu_swprintf(m_szTime, L" %.2d:%.2d:%.2d", iMinute, m_iTime % 60, (int)WorldTime % 60);

    if (iMinute < 5)
    {
        m_iTimeState = BC_TIME_STATE_IMMINENCE;
    }
    else
    {
        m_iTimeState = BC_TIME_STATE_NORMAL;
    }
}

void CBloodCastle::SetKillMonsterStatue(int iKilled, int iMaxKill)
{
    m_iKilledMonster = iKilled;
    m_iMaxKillMonster = iMaxKill;
}

#include "stdafx.h"
#include "UI/Core/WindowSystem.h"
#include "UI/NPCs/EmpireGuardianTimer.h"
#include "I18N/All.h"
#include "UI/RmlBridge/RmlTheme.h"

using namespace SEASON3B;
using namespace mu::ui::window;

CEmpireGuardianTimer::CEmpireGuardianTimer()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_dTime = 600000;
    m_iType = 1;
    m_iDay = EG_MONDAY;//EG_DAY_MAP_LIST::EG_MONDAY;
    m_iZone = 1;
    m_iMonsterCount = 0;
}

CEmpireGuardianTimer::~CEmpireGuardianTimer()
{
    Release();
}

bool CEmpireGuardianTimer::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_EMPIREGUARDIAN_TIMER, this);

    SetPos(x, y);

    m_View.Build();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CEmpireGuardianTimer::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CEmpireGuardianTimer::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CEmpireGuardianTimer::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;
    return true;
}

bool CEmpireGuardianTimer::UpdateKeyEvent()
{
    return true;
}

bool CEmpireGuardianTimer::Update()
{
    SyncView();

    return true;
}

bool CEmpireGuardianTimer::Render()
{
    // Nothing native left: the frame and the texts are RmlUi (SyncView()). Kept because CObject
    // requires the override.
    return true;
}

void CEmpireGuardianTimer::SyncView()
{
    // The original's Render(): the round and zone, the standby / time-left caption, then the time
    // and the monsters left in the big font, orange, red-orange under three minutes, red under one;
    // every line centred on 110 units from x 7.
    EventTimerView::Line round;
    EventTimerView::Line caption;
    EventTimerView::Line time;
    if (IsVisible())
    {
        wchar_t szText[256] = {};
        mu_swprintf(szText, I18N::Game::RoundDZoneD, m_iDay, m_iZone);
        // The original set no colour for the round: it took whatever the text renderer was left
        // in by the window drawn before it, white in every capture (the windows drawn before it
        // reset to white). White here.
        round = {szText, RGBA(255, 255, 255, 255)};
        switch (m_iType)
        {
        case 0:
        case 1:
            caption = {I18N::Game::StandbyTime, RGBA(10, 200, 10, 255)};
            break;
        case 2:
            mu_swprintf(szText, L"%ls (%ls)", I18N::Game::TimeLeft, I18N::Game::RemainingMonsters);
            caption = {szText, RGBA(255, 150, 0, 255)};
            break;
        default:
            break;
        }
        const int iSecond = static_cast<int>(m_dTime / 1000);
        const int iMinute = iSecond / 60;
        unsigned long timeColor = caption.color;
        if (2 < iMinute)
            timeColor = RGBA(255, 150, 0, 255);
        else if (0 < iMinute && iMinute <= 2)
            timeColor = RGBA(255, 70, 0, 255);
        else if (iMinute == 0)
            timeColor = RGBA(255, 0, 0, 255);
        mu_swprintf(szText, L"%.2d:%.2d(%d)", iMinute, iSecond % 60, m_iMonsterCount);
        time = {szText, timeColor};
    }
    m_View.Sync(IsVisible(), m_Pos, round, caption, time, TIMER_WINDOW_WIDTH / 2.f - 55.f, 110.f);
}

void CEmpireGuardianTimer::ReloadRmlTheme()
{
    m_View.ReloadTheme();
}

bool CEmpireGuardianTimer::BtnProcess()
{
    return false;
}

float CEmpireGuardianTimer::GetLayerDepth()
{
    return 1.2f;
}

void CEmpireGuardianTimer::OpenningProcess()
{
}

void CEmpireGuardianTimer::ClosingProcess() {}

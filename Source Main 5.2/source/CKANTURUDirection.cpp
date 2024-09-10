#include "stdafx.h"
#include "CComGem.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzEffect.h"
#include "ZzzAI.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzInterface.h"
#include "CDirection.h"
#include "CKanturuDirection.h"
#include "MapManager.h"

CKanturuDirection::CKanturuDirection()
{
    m_iKanturuState = 0;
    m_iMayaState = 0;
    m_iNightmareState = 0;
    Init();
}

CKanturuDirection::~CKanturuDirection()
{
}

void CKanturuDirection::Init()
{
    m_bDirectionEnd = false;
    m_bKanturuDirection = false;
    m_bMayaDie = false;
    m_bMayaAppear = false;
}

bool CKanturuDirection::IsKanturuDirection()
{
    if (m_bKanturuDirection)
        return true;

    return false;
}

bool CKanturuDirection::IsMayaScene()
{
    if (gMapManager.WorldActive == WD_39KANTURU_3RD)
    {
        if (m_iKanturuState == KANTURU_STATE_NIGHTMARE_BATTLE || m_iKanturuState == KANTURU_STATE_TOWER || m_iKanturuState == KANTURU_STATE_END)
            return false;
        else
            return true;
    }
    else
        return false;
}

bool CKanturuDirection::IsKanturu3rdTimer()
{
    if (gMapManager.WorldActive == WD_39KANTURU_3RD)
    {
        if (m_iMayaState == KANTURU_MAYA_DIRECTION_MONSTER1 || m_iMayaState == KANTURU_MAYA_DIRECTION_MAYA1)
            return true;
        else if (m_iMayaState == KANTURU_MAYA_DIRECTION_MONSTER2 || m_iMayaState == KANTURU_MAYA_DIRECTION_MAYA2)
            return true;
        else if (m_iMayaState == KANTURU_MAYA_DIRECTION_MONSTER3 || m_iMayaState == KANTURU_MAYA_DIRECTION_MAYA3)
            return true;
        else if (m_iNightmareState == KANTURU_NIGHTMARE_DIRECTION_BATTLE)
            return true;
        else
            return false;
    }
    else
        return false;
}

void CKanturuDirection::GetKanturuAllState(BYTE State, BYTE DetailState)
{
    m_iKanturuState = State;

    if (m_iKanturuState == KANTURU_STATE_STANDBY)
    {
        g_Direction.Init();
        Init();
        m_iMayaState = 0;
        m_iNightmareState = 0;
    }
    else if (m_iKanturuState == KANTURU_STATE_MAYA_BATTLE)
    {
        m_iNightmareState = 0;
        GetKanturuMayaState(DetailState);
    }
    else if (m_iKanturuState == KANTURU_STATE_NIGHTMARE_BATTLE)
    {
        m_iMayaState = 0;
        GetKanturuNightmareState(DetailState);
    }
}

void CKanturuDirection::GetKanturuMayaState(BYTE DetailState)
{
    m_iMayaState = DetailState;

    if (gMapManager.WorldActive != WD_39KANTURU_3RD)
        return;

    switch (m_iMayaState)
    {
    case KANTURU_MAYA_DIRECTION_NOTIFY:
        g_Direction.CloseAllWindows();
        g_Direction.m_CameraLevel = 5;
        if (g_Direction.m_fCameraViewFar <= 1200.0f)
            g_Direction.m_fCameraViewFar += 10.0f;
        if (!m_bDirectionEnd && !m_bKanturuDirection)
            m_bKanturuDirection = true;
        break;
    case KANTURU_MAYA_DIRECTION_ENDCYCLE_MAYA3:
        g_Direction.CloseAllWindows();
        g_Direction.m_CameraLevel = 5;
        if (g_Direction.m_fCameraViewFar <= 1200.0f)
            g_Direction.m_fCameraViewFar += 10.0f;
        if (!m_bDirectionEnd && !m_bKanturuDirection)
            m_bKanturuDirection = true;
        break;
    default:
        g_Direction.Init();
        Init();
        break;
    }
}

void CKanturuDirection::GetKanturuNightmareState(BYTE DetailState)
{
    m_iNightmareState = DetailState;

    if (gMapManager.WorldActive != WD_39KANTURU_3RD)
        return;

    switch (m_iNightmareState)
    {
    case KANTURU_NIGHTMARE_DIRECTION_NIGHTMARE:
        g_Direction.CloseAllWindows();
        g_Direction.m_CameraLevel = 5;
        if (!m_bDirectionEnd && !m_bKanturuDirection)
            m_bKanturuDirection = true;
        break;
    case KANTURU_NIGHTMARE_DIRECTION_END:
        g_Direction.m_CameraLevel = 5;
        if (!m_bDirectionEnd && !m_bKanturuDirection)
            m_bKanturuDirection = true;
    default:
        g_Direction.DeleteMonster();
        g_Direction.Init();
        Init();
        break;
    }
}

void CKanturuDirection::KanturuAllDirection()
{
    if (m_bDirectionEnd || gMapManager.WorldActive != WD_39KANTURU_3RD) return;

    if (m_iMayaState == KANTURU_MAYA_DIRECTION_END)
        g_Direction.m_bDownHero = true;

    g_Direction.HeroFallingDownDirection();
    g_Direction.HeroFallingDownInit();

    if (m_iKanturuState == KANTURU_STATE_MAYA_BATTLE)
        KanturuMayaDirection();
    else if (m_iKanturuState == KANTURU_STATE_NIGHTMARE_BATTLE)
        KanturuNightmareDirection();
    else if (m_iKanturuState == KANTURU_STATE_STANDBY)
    {
        g_Direction.HeroFallingDownInit();
        g_Direction.Init();
        Init();
    }
}

void CKanturuDirection::KanturuMayaDirection()
{
    switch (m_iMayaState)
    {
    case KANTURU_MAYA_DIRECTION_NOTIFY:
        Move1stDirection();
        break;
    case KANTURU_MAYA_DIRECTION_ENDCYCLE_MAYA3:
        Move2ndDirection();
        break;
    }
}

void CKanturuDirection::Move1stDirection()
{
    if (g_Direction.DirectionCameraMove()) return;

    switch (g_Direction.m_iTimeSchedule)
    {
    case 0:		Direction1st0();		break;
    case 1:		Direction1st1();		break;
    }
}

void CKanturuDirection::Direction1st0()
{
    g_Direction.SetNextDirectionPosition(196, 85, 0, 100.0f);
    g_Direction.m_iTimeSchedule--;
}

void CKanturuDirection::Direction1st1()
{
    if (g_Direction.m_bAction)
    {
        if (g_Direction.m_iCheckTime == 0)
        {
            m_bMayaAppear = true;
            g_Direction.m_iCheckTime++;
        }
        else if (g_Direction.m_iCheckTime == 1)
        {
            if (!m_bMayaAppear)
                g_Direction.m_iCheckTime++;
        }
        else if (g_Direction.m_iCheckTime == 2)
        {
            g_Direction.GetTimeCheck(3000);
        }
        else if (g_Direction.m_iCheckTime == 3)
        {
            g_Direction.Init();
            Init();
            m_bDirectionEnd = true;
        }
    }
}

void CKanturuDirection::Move2ndDirection()
{
    if (g_Direction.DirectionCameraMove()) return;

    switch (g_Direction.m_iTimeSchedule)
    {
    case 0:		Direction2nd0();		break;
    case 1:		Direction2nd1();		break;
    case 2:		Direction2nd2();		break;
    }
}

void CKanturuDirection::Direction2nd0()
{
    g_Direction.SetNextDirectionPosition(196, 85, 0, 100.0f);
    g_Direction.m_iTimeSchedule--;
}

void CKanturuDirection::Direction2nd1()
{
    if (g_Direction.m_bAction)
    {
        if (g_Direction.m_iCheckTime == 0)
        {
            m_bMayaDie = true;
            g_Direction.m_iCheckTime++;
        }
        else if (g_Direction.m_iCheckTime == 1)
        {
            if (!m_bMayaDie)
                g_Direction.m_iCheckTime++;
        }
        else if (g_Direction.m_iCheckTime == 2)
        {
            Init();
            g_Direction.Init();
            g_Direction.m_bAction = true;
            g_Direction.m_iTimeSchedule = 2;
        }
    }
}

void CKanturuDirection::Direction2nd2()
{
    if (g_Direction.m_bAction)
    {
        if (g_Direction.m_iCheckTime == 0)
            g_Direction.m_bDownHero = true;
    }
}

bool CKanturuDirection::GetMayaExplotion()
{
    return m_bMayaDie;
}

void CKanturuDirection::SetMayaExplotion(bool MayaDie)
{
    m_bMayaDie = MayaDie;
}

bool CKanturuDirection::GetMayaAppear()
{
    return m_bMayaAppear;
}

void CKanturuDirection::SetMayaAppear(bool MayaAppear)
{
    m_bMayaAppear = MayaAppear;
}

void CKanturuDirection::KanturuNightmareDirection()
{
    switch (m_iNightmareState)
    {
    case KANTURU_NIGHTMARE_DIRECTION_NIGHTMARE:
        Move3rdDirection();
        break;
    case KANTURU_NIGHTMARE_DIRECTION_END:
        Move4thDirection();
        break;
    }
}

void CKanturuDirection::Move3rdDirection()
{
    if (g_Direction.DirectionCameraMove()) return;

    switch (g_Direction.m_iTimeSchedule)
    {
    case 0:		Direction3rd0();		break;
    case 1:		Direction3rd1();		break;
    }
}

void CKanturuDirection::Direction3rd0()
{
    g_Direction.SetNextDirectionPosition(80, 142, 0, 100.0f);
    g_Direction.m_iTimeSchedule--;
}

void CKanturuDirection::Direction3rd1()
{
    if (g_Direction.m_bAction)
    {
        if (g_Direction.m_iCheckTime == 0) g_Direction.GetTimeCheck(1000);
        else if (g_Direction.m_iCheckTime == 1)
            g_Direction.SummonCreateMonster(MONSTER_NIGHTMARE, 79, 142, 0, true, true, 0.25f);
        else if (g_Direction.m_iCheckTime == 2) g_Direction.GetTimeCheck(5000);
        else if (g_Direction.m_iCheckTime == 3)
        {
            Init();
            g_Direction.Init();
            m_bDirectionEnd = true;
            g_Direction.m_bOrderExit = true;
        }
    }
}

void CKanturuDirection::Move4thDirection()
{
    if (g_Direction.DirectionCameraMove()) return;

    switch (g_Direction.m_iTimeSchedule)
    {
    case 0:		Direction4th0();		break;
    case 1:		Direction4th1();		break;
    }
}

void CKanturuDirection::Direction4th0()
{
    g_Direction.SetNextDirectionPosition(113, 232, 0, 300.0f);
    g_Direction.m_iTimeSchedule--;
}

void CKanturuDirection::Direction4th1()
{
}
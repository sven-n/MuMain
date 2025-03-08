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
#include "CMVP1stDirection.h"
#include "MapManager.h"

CMVP1STDirection::CMVP1STDirection()
{
    Init();
}

CMVP1STDirection::~CMVP1STDirection()
{
}

void CMVP1STDirection::Init()
{
    //g_Direction.Init();
    m_bTimerCheck = true;
    m_iCryWolfState = 0;
}

void CMVP1STDirection::GetCryWolfState(BYTE CryWolfState)
{
    m_iCryWolfState = CryWolfState;
}

bool CMVP1STDirection::IsCryWolfDirection()
{
    if (!g_Direction.m_bOrderExit && !m_bTimerCheck && m_iCryWolfState == CRYWOLF_STATE_NOTIFY_2)
        return true;

    return false;
}

void CMVP1STDirection::IsCryWolfDirectionTimer()
{
    if (m_iCryWolfState == CRYWOLF_STATE_NOTIFY_2 && m_bTimerCheck && GetTimeCheck(5000))
    {
        m_bTimerCheck = false;
    }

    if (m_iCryWolfState == CRYWOLF_STATE_READY)
    {
        g_Direction.DeleteMonster();
        Init();
    }
}

void CMVP1STDirection::CryWolfDirection()
{
    if (gMapManager.WorldActive != WD_34CRYWOLF_1ST)
        return;

    IsCryWolfDirectionTimer();

    if (!IsCryWolfDirection())
        return;

    switch (m_iCryWolfState)
    {
    case CRYWOLF_STATE_NOTIFY_2:
        g_Direction.CloseAllWindows();
        MoveBeginDirection();
        break;
    default:
        g_Direction.DeleteMonster();
        Init();
        break;
    }
}

void CMVP1STDirection::MoveBeginDirection()
{
    if (g_Direction.DirectionCameraMove()) return;

    switch (g_Direction.m_iTimeSchedule)
    {
    case 0:		BeginDirection0();		break;
    case 1:		BeginDirection1();		break;
    case 2:		BeginDirection2();		break;
    case 3:		BeginDirection3();		break;
    case 4:		BeginDirection4();		break;
    case 5:		BeginDirection5();		break;
    }
}

void CMVP1STDirection::BeginDirection0()
{
    g_Direction.SetNextDirectionPosition(113, 232, 0, 300.0f);
    g_Direction.m_iTimeSchedule--;
}

void CMVP1STDirection::BeginDirection1()
{
    if (g_Direction.m_bAction)
    {
        if (g_Direction.m_iCheckTime == 0) g_Direction.SummonCreateMonster(MONSTER_BALGASS, 114, 238, 0, true, true, 0.18f);
        else if (g_Direction.m_iCheckTime == 1) g_Direction.SummonCreateMonster(MONSTER_DARKELF, 113, 238, 0, true, true, 0.227f);
        else if (g_Direction.m_iCheckTime == 2) g_Direction.SummonCreateMonster(MONSTER_DARKELF, 115, 238, 0, true, true, 0.227f);
        else if (g_Direction.m_iCheckTime == 3) g_Direction.GetTimeCheck(1000);
        else if (g_Direction.m_iCheckTime == 4)
        {
            bool bSuccess[3];
            bSuccess[0] = g_Direction.MoveCreatedMonster(0, 114, 234, 0, 9);
            bSuccess[1] = g_Direction.MoveCreatedMonster(1, 112, 232, 0, 12);
            bSuccess[2] = g_Direction.MoveCreatedMonster(2, 116, 232, 0, 12);

            if (bSuccess[0] && bSuccess[1] && bSuccess[2]) g_Direction.m_iCheckTime++;
        }
        else if (g_Direction.m_iCheckTime == 5) g_Direction.GetTimeCheck(1000);
        else if (g_Direction.m_iCheckTime == 6)
        {
            if (g_Direction.ActionCreatedMonster(1, MONSTER01_ATTACK3, 1))
                g_Direction.SummonCreateMonster(MONSTER_DEATH_SPIRIT, 109, 229, 0);
        }
        else if (g_Direction.m_iCheckTime == 7) g_Direction.GetTimeCheck(660);
        else if (g_Direction.m_iCheckTime == 8) g_Direction.SummonCreateMonster(MONSTER_DEATH_SPIRIT, 110, 230, 0);
        else if (g_Direction.m_iCheckTime == 9) g_Direction.GetTimeCheck(660);
        else if (g_Direction.m_iCheckTime == 10)	g_Direction.SummonCreateMonster(MONSTER_DEATH_SPIRIT, 112, 230, 0);
        else if (g_Direction.m_iCheckTime == 11)	g_Direction.GetTimeCheck(660);
        else if (g_Direction.m_iCheckTime == 12)	g_Direction.SummonCreateMonster(MONSTER_DEATH_SPIRIT, 113, 229, 0);
        else if (g_Direction.m_iCheckTime == 13) g_Direction.GetTimeCheck(1000);
        else if (g_Direction.m_iCheckTime == 14)
        {
            if (g_Direction.ActionCreatedMonster(2, MONSTER01_ATTACK3, 1))
                g_Direction.SummonCreateMonster(MONSTER_BALRAM, 114, 229, 0);
        }
        else if (g_Direction.m_iCheckTime == 15) g_Direction.GetTimeCheck(660);
        else if (g_Direction.m_iCheckTime == 16)	g_Direction.SummonCreateMonster(MONSTER_BALRAM, 115, 230, 0);
        else if (g_Direction.m_iCheckTime == 17) g_Direction.GetTimeCheck(660);
        else if (g_Direction.m_iCheckTime == 18)	g_Direction.SummonCreateMonster(MONSTER_BALRAM, 117, 230, 0);
        else if (g_Direction.m_iCheckTime == 19) g_Direction.GetTimeCheck(660);
        else if (g_Direction.m_iCheckTime == 20)	g_Direction.SummonCreateMonster(MONSTER_BALRAM, 118, 229, 0);
        else if (g_Direction.m_iCheckTime == 21) g_Direction.GetTimeCheck(1000);
        else if (g_Direction.m_iCheckTime == 22)
        {
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 110, 227, 0);
            g_Direction.CameraLevelUp();
        }
        else if (g_Direction.m_iCheckTime == 23) g_Direction.GetTimeCheck(660);
        else if (g_Direction.m_iCheckTime == 24)
        {
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 112, 227, 0);
            g_Direction.CameraLevelUp();
        }
        else if (g_Direction.m_iCheckTime == 25) g_Direction.GetTimeCheck(660);
        else if (g_Direction.m_iCheckTime == 26)
        {
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 114, 227, 0);
            g_Direction.CameraLevelUp();
        }
        else if (g_Direction.m_iCheckTime == 27) g_Direction.GetTimeCheck(660);
        else if (g_Direction.m_iCheckTime == 28)
        {
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 116, 227, 0);
            g_Direction.CameraLevelUp();
        }
        else if (g_Direction.m_iCheckTime == 29) g_Direction.GetTimeCheck(1000);
        else if (g_Direction.m_iCheckTime == 30)	g_Direction.ActionCreatedMonster(0, MONSTER01_ATTACK3, 1, false, true);
        else if (g_Direction.m_iCheckTime == 31) g_Direction.GetTimeCheck(2000);
        else g_Direction.m_bAction = false;
    }
    else
    {
        g_Direction.SetNextDirectionPosition(114, 220, 0, 40.0f);
    }
}

void CMVP1STDirection::BeginDirection2()
{
    if (g_Direction.m_bAction)
    {
        if (g_Direction.m_iCheckTime == 0)
        {
            g_Direction.SummonCreateMonster(MONSTER_BALLISTA, 110, 240, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_BALLISTA, 114, 240, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_BALLISTA, 118, 240, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 110, 242, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 112, 242, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 114, 242, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 116, 242, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 118, 242, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 110, 244, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 112, 244, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 114, 244, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 116, 244, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 118, 244, 0);
        }
        else if (g_Direction.m_iCheckTime == 1)
        {
            g_Direction.GetTimeCheck(1000);
        }
        else if (g_Direction.m_iCheckTime == 2)
        {
            bool bSuccess[28];
            int iMovingMonCount = sizeof(bSuccess);

            bSuccess[0] = g_Direction.MoveCreatedMonster(0, 114, 222, 0, 9);
            bSuccess[1] = g_Direction.MoveCreatedMonster(1, 110, 222, 0, 12);
            bSuccess[2] = g_Direction.MoveCreatedMonster(2, 118, 222, 0, 12);
            bSuccess[3] = g_Direction.MoveCreatedMonster(3, 107, 219, 0, 12);
            bSuccess[4] = g_Direction.MoveCreatedMonster(4, 108, 220, 0, 12);
            bSuccess[5] = g_Direction.MoveCreatedMonster(5, 110, 220, 0, 12);
            bSuccess[6] = g_Direction.MoveCreatedMonster(6, 111, 219, 0, 12);
            bSuccess[7] = g_Direction.MoveCreatedMonster(7, 116, 219, 0, 12);
            bSuccess[8] = g_Direction.MoveCreatedMonster(8, 117, 220, 0, 12);
            bSuccess[9] = g_Direction.MoveCreatedMonster(9, 119, 220, 0, 12);
            bSuccess[10] = g_Direction.MoveCreatedMonster(10, 120, 219, 0, 12);
            bSuccess[11] = g_Direction.MoveCreatedMonster(11, 110, 217, 0, 12);
            bSuccess[12] = g_Direction.MoveCreatedMonster(12, 112, 217, 0, 12);
            bSuccess[13] = g_Direction.MoveCreatedMonster(13, 114, 217, 0, 12);
            bSuccess[14] = g_Direction.MoveCreatedMonster(14, 116, 217, 0, 12);
            bSuccess[15] = g_Direction.MoveCreatedMonster(15, 110, 227, 0, 12);
            bSuccess[16] = g_Direction.MoveCreatedMonster(16, 114, 227, 0, 12);
            bSuccess[17] = g_Direction.MoveCreatedMonster(17, 118, 227, 0, 12);
            bSuccess[18] = g_Direction.MoveCreatedMonster(18, 110, 229, 0, 12);
            bSuccess[19] = g_Direction.MoveCreatedMonster(19, 112, 229, 0, 12);
            bSuccess[20] = g_Direction.MoveCreatedMonster(20, 114, 229, 0, 12);
            bSuccess[21] = g_Direction.MoveCreatedMonster(21, 116, 229, 0, 12);
            bSuccess[22] = g_Direction.MoveCreatedMonster(22, 118, 229, 0, 12);
            bSuccess[23] = g_Direction.MoveCreatedMonster(23, 110, 231, 0, 12);
            bSuccess[24] = g_Direction.MoveCreatedMonster(24, 112, 231, 0, 12);
            bSuccess[25] = g_Direction.MoveCreatedMonster(25, 114, 231, 0, 12);
            bSuccess[26] = g_Direction.MoveCreatedMonster(26, 116, 231, 0, 12);
            bSuccess[27] = g_Direction.MoveCreatedMonster(27, 118, 231, 0, 12);

            for (int i = 0; i < iMovingMonCount; i++)
            {
                if (!bSuccess[i]) break;
                if (i == iMovingMonCount - 1) g_Direction.m_iCheckTime++;
            }
        }
        else if (g_Direction.m_iCheckTime == 3) g_Direction.GetTimeCheck(1000);
        else if (g_Direction.m_iCheckTime == 4) g_Direction.ActionCreatedMonster(0, MONSTER01_ATTACK3, 1, false, true);
        else if (g_Direction.m_iCheckTime == 5) g_Direction.GetTimeCheck(2700);
        else if (g_Direction.m_iCheckTime == 6) g_Direction.ActionCreatedMonster(0, MONSTER01_ATTACK4, 1, false, true);
        else if (g_Direction.m_iCheckTime == 7) g_Direction.GetTimeCheck(3000);
        else g_Direction.m_bAction = false;
    }
    else
        g_Direction.SetNextDirectionPosition(114, 160, 0, 300.0f);
}

void CMVP1STDirection::BeginDirection3()
{
    if (g_Direction.m_bAction)
    {
        if (g_Direction.m_iCheckTime == 0)
        {
            g_Direction.DeleteMonster();
            g_Direction.m_iCheckTime++;
        }
        else if (g_Direction.m_iCheckTime == 1)
        {
            g_Direction.SummonCreateMonster(MONSTER_DARKELF, 110, 77, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_DARKELF, 125, 77, 0, false, false);

            g_Direction.SummonCreateMonster(MONSTER_SORAM, 90, 37, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 108, 73, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 109, 75, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 110, 73, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 111, 75, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 112, 73, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 123, 73, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 124, 75, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 125, 73, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 126, 75, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 127, 73, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 176, 20, 0, false, false);

            g_Direction.SummonCreateMonster(MONSTER_SORAM, 117, 77, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 119, 77, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_SORAM, 121, 77, 0, true, false);

            g_Direction.SummonCreateMonster(MONSTER_DARKELF, 119, 83, 0, false, false);

            g_Direction.SummonCreateMonster(MONSTER_BALRAM, 118, 79, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_BALRAM, 120, 79, 0, false, false);

            g_Direction.SummonCreateMonster(MONSTER_BALRAM, 119, 90, 0, false, false);

            g_Direction.SummonCreateMonster(MONSTER_DEATH_SPIRIT, 116, 81, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_DEATH_SPIRIT, 119, 81, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_DEATH_SPIRIT, 122, 81, 0, false, false);

            g_Direction.SummonCreateMonster(MONSTER_DARKELF, 119, 87, 0, false, false);

            g_Direction.SummonCreateMonster(MONSTER_BALLISTA, 116, 90, 0, false, false);
            g_Direction.SummonCreateMonster(MONSTER_BALLISTA, 122, 90, 0, true, false);
        }
        else g_Direction.m_bAction = false;
    }
    else
        g_Direction.SetNextDirectionPosition(121, 75, 0, 300.0f);
}

void CMVP1STDirection::BeginDirection4()
{
    if (g_Direction.m_bAction)
    {
        if (g_Direction.m_iCheckTime == 0) g_Direction.GetTimeCheck(3000);
        else g_Direction.m_bAction = false;
    }
    else
        g_Direction.SetNextDirectionPosition(121, 87, 0, 40.0f);
}

void CMVP1STDirection::BeginDirection5()
{
    if (g_Direction.m_bAction)
    {
        if (g_Direction.m_iCheckTime == 0) g_Direction.GetTimeCheck(2000);
        else if (g_Direction.m_iCheckTime == 1)
        {
            g_Direction.ActionCreatedMonster(25, MONSTER01_ATTACK1, 1, true);
            g_Direction.ActionCreatedMonster(26, MONSTER01_ATTACK1, 1, true, true);
        }
        else if (g_Direction.m_iCheckTime == 2) g_Direction.GetTimeCheck(4000);
        else g_Direction.m_bAction = false;
    }
    else
    {
        g_Direction.DeleteMonster();
        g_Direction.m_bOrderExit = true;
    }
}
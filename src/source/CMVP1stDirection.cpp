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

#include <array>
#include <cstdint>

namespace
{
    constexpr int kCrywolfNotifyDelayMs = 5000;
    constexpr int kCrywolfBeginCameraSpeedFast = 300;
    constexpr int kCrywolfBeginCameraSpeedSlow = 40;
    constexpr int kCrywolfSecondPhaseCameraSpeed = 300;
    constexpr int kCrywolfThirdPhaseCameraSpeed = 300;
    constexpr int kCrywolfFourthPhaseCameraSpeed = 40;

    struct CameraTarget
    {
        int x;
        int y;
        int z;
        float speed;
    };

    struct MonsterSpawnCommand
    {
        EMonsterType type;
        int x;
        int y;
        float angle;
        bool nextCheck;
        bool summonAnimation;
        float animationSpeed;
    };

    struct MonsterMoveCommand
    {
        int index;
        int x;
        int y;
        float angle;
        int speed;
    };

    constexpr CameraTarget kBeginDirection0Target{113, 232, 0, static_cast<float>(kCrywolfBeginCameraSpeedFast)};
    constexpr CameraTarget kBeginDirection1Fallback{114, 220, 0, static_cast<float>(kCrywolfBeginCameraSpeedSlow)};
    constexpr CameraTarget kBeginDirection2Fallback{114, 160, 0, static_cast<float>(kCrywolfSecondPhaseCameraSpeed)};
    constexpr CameraTarget kBeginDirection3Fallback{121, 75, 0, static_cast<float>(kCrywolfThirdPhaseCameraSpeed)}; 
    constexpr CameraTarget kBeginDirection4Fallback{121, 87, 0, static_cast<float>(kCrywolfFourthPhaseCameraSpeed)}; 

    constexpr std::array<MonsterSpawnCommand, 13> kBeginDirection2WaveOne = {
        MonsterSpawnCommand{MONSTER_BALLISTA, 110, 240, 0.0f, false, false, -1.0f},
        {MONSTER_BALLISTA, 114, 240, 0.0f, false, false, -1.0f},
        {MONSTER_BALLISTA, 118, 240, 0.0f, false, false, -1.0f},
        {MONSTER_SORAM, 110, 242, 0.0f, false, false, -1.0f},
        {MONSTER_SORAM, 112, 242, 0.0f, false, false, -1.0f},
        {MONSTER_SORAM, 114, 242, 0.0f, false, false, -1.0f},
        {MONSTER_SORAM, 116, 242, 0.0f, false, false, -1.0f},
        {MONSTER_SORAM, 118, 242, 0.0f, false, false, -1.0f},
        {MONSTER_SORAM, 110, 244, 0.0f, false, false, -1.0f},
        {MONSTER_SORAM, 112, 244, 0.0f, false, false, -1.0f},
        {MONSTER_SORAM, 114, 244, 0.0f, false, false, -1.0f},
        {MONSTER_SORAM, 116, 244, 0.0f, false, false, -1.0f},
        {MONSTER_SORAM, 118, 244, 0.0f, true, true, -1.0f},
    };

    constexpr std::array<MonsterMoveCommand, 28> kBeginDirection2MoveCommands = {
        MonsterMoveCommand{0, 114, 222, 0.0f, 9},
        {1, 110, 222, 0.0f, 12},
        {2, 118, 222, 0.0f, 12},
        {3, 107, 219, 0.0f, 12},
        {4, 108, 220, 0.0f, 12},
        {5, 110, 220, 0.0f, 12},
        {6, 111, 219, 0.0f, 12},
        {7, 116, 219, 0.0f, 12},
        {8, 117, 220, 0.0f, 12},
        {9, 119, 220, 0.0f, 12},
        {10, 120, 219, 0.0f, 12},
        {11, 110, 217, 0.0f, 12},
        {12, 112, 217, 0.0f, 12},
        {13, 114, 217, 0.0f, 12},
        {14, 116, 217, 0.0f, 12},
        {15, 110, 227, 0.0f, 12},
        {16, 114, 227, 0.0f, 12},
        {17, 118, 227, 0.0f, 12},
        {18, 110, 229, 0.0f, 12},
        {19, 112, 229, 0.0f, 12},
        {20, 114, 229, 0.0f, 12},
        {21, 116, 229, 0.0f, 12},
        {22, 118, 229, 0.0f, 12},
        {23, 110, 231, 0.0f, 12},
        {24, 112, 231, 0.0f, 12},
        {25, 114, 231, 0.0f, 12},
        {26, 116, 231, 0.0f, 12},
        {27, 118, 231, 0.0f, 12},
    };
}

CMVP1STDirection::CMVP1STDirection()
{
    Init();
}

CMVP1STDirection::~CMVP1STDirection()
{
}

void CMVP1STDirection::Init()
{
    m_bTimerCheck = true;
    m_iCryWolfState = 0;
}

void CMVP1STDirection::GetCryWolfState(std::uint8_t CryWolfState)
{
    m_iCryWolfState = CryWolfState;
}

bool CMVP1STDirection::IsCryWolfDirection() const
{
    return IsSequenceReady() && !m_bTimerCheck && m_iCryWolfState == CRYWOLF_STATE_NOTIFY_2;
}

void CMVP1STDirection::IsCryWolfDirectionTimer()
{
    if (m_iCryWolfState == CRYWOLF_STATE_NOTIFY_2 && m_bTimerCheck && GetTimeCheck(kCrywolfNotifyDelayMs))
    {
        m_bTimerCheck = false;
    }

    if (m_iCryWolfState == CRYWOLF_STATE_READY)
    {
        ResetSequence();
    }
}

void CMVP1STDirection::CryWolfDirection()
{
    if (!IsCrywolfWorldActive())
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
        ResetSequence();
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
    QueueCameraMove(kBeginDirection0Target.x, kBeginDirection0Target.y, kBeginDirection0Target.z, kBeginDirection0Target.speed);
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
        QueueCameraMove(kBeginDirection1Fallback.x, kBeginDirection1Fallback.y, kBeginDirection1Fallback.z, kBeginDirection1Fallback.speed);
    }
}

void CMVP1STDirection::BeginDirection2()
{
    if (g_Direction.m_bAction)
    {
        if (g_Direction.m_iCheckTime == 0)
        {
            for (const auto& command : kBeginDirection2WaveOne)
            {
                g_Direction.SummonCreateMonster(
                    command.type,
                    command.x,
                    command.y,
                    command.angle,
                    command.nextCheck,
                    command.summonAnimation,
                    command.animationSpeed);
            }
        }
        else if (g_Direction.m_iCheckTime == 1)
        {
            g_Direction.GetTimeCheck(1000);
        }
        else if (g_Direction.m_iCheckTime == 2)
        {
            bool allMovesSucceeded = true;
            for (const auto& command : kBeginDirection2MoveCommands)
            {
                if (!g_Direction.MoveCreatedMonster(
                        command.index,
                        command.x,
                        command.y,
                        command.angle,
                        command.speed))
                {
                    allMovesSucceeded = false;
                    break;
                }
            }

            if (allMovesSucceeded)
            {
                g_Direction.m_iCheckTime++;
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
        QueueCameraMove(kBeginDirection2Fallback.x, kBeginDirection2Fallback.y, kBeginDirection2Fallback.z, kBeginDirection2Fallback.speed);
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
        QueueCameraMove(kBeginDirection3Fallback.x, kBeginDirection3Fallback.y, kBeginDirection3Fallback.z, kBeginDirection3Fallback.speed);
}

void CMVP1STDirection::BeginDirection4()
{
    if (g_Direction.m_bAction)
    {
        if (g_Direction.m_iCheckTime == 0) g_Direction.GetTimeCheck(3000);
        else g_Direction.m_bAction = false;
    }
    else
        QueueCameraMove(kBeginDirection4Fallback.x, kBeginDirection4Fallback.y, kBeginDirection4Fallback.z, kBeginDirection4Fallback.speed);
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

bool CMVP1STDirection::IsCrywolfWorldActive() const
{
    return gMapManager.WorldActive == WD_34CRYWOLF_1ST;
}

bool CMVP1STDirection::IsSequenceReady() const
{
    return !g_Direction.m_bOrderExit;
}

void CMVP1STDirection::ResetSequence()
{
    g_Direction.DeleteMonster();
    g_Direction.m_bOrderExit = false;
    Init();
}

void CMVP1STDirection::QueueCameraMove(int x, int y, int z, float speed)
{
    g_Direction.SetNextDirectionPosition(x, y, z, speed);
}
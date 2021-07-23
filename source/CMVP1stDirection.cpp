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
	if(!g_Direction.m_bOrderExit && !m_bTimerCheck && m_iCryWolfState == CRYWOLF_STATE_NOTIFY_2)
		return true;

	return false;
}

void CMVP1STDirection::IsCryWolfDirectionTimer()
{
	// 처음 크라이울프 맵에 들어와서 연출 중이면 5초후에 연출 보여줌
	if(m_iCryWolfState == CRYWOLF_STATE_NOTIFY_2 && m_bTimerCheck && GetTimeCheck(5000))
	{
		m_bTimerCheck = false;
	}

	if(m_iCryWolfState == CRYWOLF_STATE_READY)
	{
		g_Direction.DeleteMonster();
		Init();
	}
}

void CMVP1STDirection::CryWolfDirection()
{
	if(World != WD_34CRYWOLF_1ST)
		return;

	IsCryWolfDirectionTimer();

	if(!IsCryWolfDirection())	// 연출이 아니면 리턴
		return;

	switch(m_iCryWolfState)
	{
	case CRYWOLF_STATE_NOTIFY_2:
		g_Direction.CloseAllWindows();
		MoveBeginDirection();
		break;
	default:	//초기화
		g_Direction.DeleteMonster();
		Init();
		break;
	}
}

void CMVP1STDirection::MoveBeginDirection()
{
	if(g_Direction.DirectionCameraMove()) return;

	//연출 시작 - 몹 걸어 나와서 소환
	switch(g_Direction.m_iTimeSchedule)
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
	if(g_Direction.m_bAction)
	{
		if(g_Direction.m_iCheckTime == 0) g_Direction.SummonCreateMonster(349, 114, 238, 0, true, true, 0.18f);	// 발가스 소환
		else if(g_Direction.m_iCheckTime == 1) g_Direction.SummonCreateMonster(340, 113, 238, 0, true, true, 0.227f);	// 엘프1 소환
		else if(g_Direction.m_iCheckTime == 2) g_Direction.SummonCreateMonster(340, 115, 238, 0, true, true, 0.227f);	// 엘프2 소환
		else if(g_Direction.m_iCheckTime == 3) g_Direction.GetTimeCheck(1000);
		else if(g_Direction.m_iCheckTime == 4)
		{
			bool bSuccess[2];
			bSuccess[0] = g_Direction.MoveCreatedMonster(0, 114, 234, 0, 9);	// 발가스 걸어 나오기
			bSuccess[1] = g_Direction.MoveCreatedMonster(1, 112, 232, 0, 12);	// 엘프1 걸어 나오기
			bSuccess[2] = g_Direction.MoveCreatedMonster(2, 116, 232, 0, 12);	// 엘프2 걸어 나오기

			if(bSuccess[0] && bSuccess[1] && bSuccess[2]) g_Direction.m_iCheckTime++;	
		}
		else if(g_Direction.m_iCheckTime == 5) g_Direction.GetTimeCheck(1000);
		else if(g_Direction.m_iCheckTime == 6)
		{
			if(g_Direction.ActionCreatedMonster(1, MONSTER01_ATTACK3, 1))
				g_Direction.SummonCreateMonster(345, 109, 229, 0);	// 데스스피릿1 소환
		}
		else if(g_Direction.m_iCheckTime == 7) g_Direction.GetTimeCheck(660);
		else if(g_Direction.m_iCheckTime == 8) g_Direction.SummonCreateMonster(345, 110, 230, 0);	// 데스스피릿2 소환
		else if(g_Direction.m_iCheckTime == 9) g_Direction.GetTimeCheck(660);
		else if(g_Direction.m_iCheckTime == 10)	g_Direction.SummonCreateMonster(345, 112, 230, 0);	// 데스스피릿3 소환
		else if(g_Direction.m_iCheckTime == 11)	g_Direction.GetTimeCheck(660);
		else if(g_Direction.m_iCheckTime == 12)	g_Direction.SummonCreateMonster(345, 113, 229, 0);	// 데스스피릿4 소환
		else if(g_Direction.m_iCheckTime == 13) g_Direction.GetTimeCheck(1000);
		else if(g_Direction.m_iCheckTime == 14)
		{
			if(g_Direction.ActionCreatedMonster(2, MONSTER01_ATTACK3, 1))
				g_Direction.SummonCreateMonster(344, 114, 229, 0);	// 발람1 소환
		}
		else if(g_Direction.m_iCheckTime == 15) g_Direction.GetTimeCheck(660);
		else if(g_Direction.m_iCheckTime == 16)	g_Direction.SummonCreateMonster(344, 115, 230, 0);	// 발람2 소환
		else if(g_Direction.m_iCheckTime == 17) g_Direction.GetTimeCheck(660);
		else if(g_Direction.m_iCheckTime == 18)	g_Direction.SummonCreateMonster(344, 117, 230, 0);	// 발람3 소환
		else if(g_Direction.m_iCheckTime == 19) g_Direction.GetTimeCheck(660);
		else if(g_Direction.m_iCheckTime == 20)	g_Direction.SummonCreateMonster(344, 118, 229, 0);	// 발람4 소환
		else if(g_Direction.m_iCheckTime == 21) g_Direction.GetTimeCheck(1000);
		else if(g_Direction.m_iCheckTime == 22)
		{
			g_Direction.SummonCreateMonster(341, 110, 227, 0);	// 소람1 소환
			g_Direction.CameraLevelUp();
		}
		else if(g_Direction.m_iCheckTime == 23) g_Direction.GetTimeCheck(660);
		else if(g_Direction.m_iCheckTime == 24)
		{
			g_Direction.SummonCreateMonster(341, 112, 227, 0);	// 소람2 소환
			g_Direction.CameraLevelUp();
		}
		else if(g_Direction.m_iCheckTime == 25) g_Direction.GetTimeCheck(660);
		else if(g_Direction.m_iCheckTime == 26)
		{
			g_Direction.SummonCreateMonster(341, 114, 227, 0);	// 소람3 소환
			g_Direction.CameraLevelUp();
		}
		else if(g_Direction.m_iCheckTime == 27) g_Direction.GetTimeCheck(660);
		else if(g_Direction.m_iCheckTime == 28)
		{
			g_Direction.SummonCreateMonster(341, 116, 227, 0);	// 소람4 소환
			g_Direction.CameraLevelUp();
		}
		else if(g_Direction.m_iCheckTime == 29) g_Direction.GetTimeCheck(1000);
		else if(g_Direction.m_iCheckTime == 30)	g_Direction.ActionCreatedMonster(0, MONSTER01_ATTACK3, 1, false, true);	// 발가스 전진 명령 동작
		else if(g_Direction.m_iCheckTime == 31) g_Direction.GetTimeCheck(2000);
		else g_Direction.m_bAction = false;
	}
	else
	{
		g_Direction.SetNextDirectionPosition(114, 220, 0, 40.0f);
	}
}

void CMVP1STDirection::BeginDirection2()
{
	if(g_Direction.m_bAction)
	{
		if(g_Direction.m_iCheckTime == 0)
		{
			g_Direction.SummonCreateMonster(348, 110, 240, 0, false, false); // 탱커1 소환
			g_Direction.SummonCreateMonster(348, 114, 240, 0, false, false); // 탱커2 소환
			g_Direction.SummonCreateMonster(348, 118, 240, 0, false, false); // 탱커3 소환
			g_Direction.SummonCreateMonster(341, 110, 242, 0, false, false); // 소람5 소환
			g_Direction.SummonCreateMonster(341, 112, 242, 0, false, false); // 소람6 소환
			g_Direction.SummonCreateMonster(341, 114, 242, 0, false, false); // 소람7 소환
			g_Direction.SummonCreateMonster(341, 116, 242, 0, false, false); // 소람8 소환
			g_Direction.SummonCreateMonster(341, 118, 242, 0, false, false); // 소람9 소환
			g_Direction.SummonCreateMonster(341, 110, 244, 0, false, false); // 소람10 소환
			g_Direction.SummonCreateMonster(341, 112, 244, 0, false, false); // 소람11 소환
			g_Direction.SummonCreateMonster(341, 114, 244, 0, false, false); // 소람12 소환
			g_Direction.SummonCreateMonster(341, 116, 244, 0, false, false); // 소람13 소환
			g_Direction.SummonCreateMonster(341, 118, 244, 0);				 // 소람14 소환
		}
		else if(g_Direction.m_iCheckTime == 1)
		{
			g_Direction.GetTimeCheck(1000);
		}
		else if(g_Direction.m_iCheckTime == 2)
		{
			bool bSuccess[28];
			int iMovingMonCount = sizeof(bSuccess);

			bSuccess[0] = g_Direction.MoveCreatedMonster(0, 114, 222, 0, 9);	// 발가스 걸어 나오기
			bSuccess[1] = g_Direction.MoveCreatedMonster(1, 110, 222, 0, 12);	// 엘프1 걸어 나오기
			bSuccess[2] = g_Direction.MoveCreatedMonster(2, 118, 222, 0, 12);	// 엘프2 걸어 나오기
			bSuccess[3] = g_Direction.MoveCreatedMonster(3, 107, 219, 0, 12);	// 데스스피릿1 걸어 나오기
			bSuccess[4] = g_Direction.MoveCreatedMonster(4, 108, 220, 0, 12);	// 데스스피릿2 걸어 나오기
			bSuccess[5] = g_Direction.MoveCreatedMonster(5, 110, 220, 0, 12);	// 데스스피릿3 걸어 나오기
			bSuccess[6] = g_Direction.MoveCreatedMonster(6, 111, 219, 0, 12);	// 데스스피릿4 걸어 나오기
			bSuccess[7] = g_Direction.MoveCreatedMonster(7, 116, 219, 0, 12);	// 발람1 걸어 나오기
			bSuccess[8] = g_Direction.MoveCreatedMonster(8, 117, 220, 0, 12);	// 발람2 걸어 나오기
			bSuccess[9] = g_Direction.MoveCreatedMonster(9, 119, 220, 0, 12);	// 발람3 걸어 나오기
			bSuccess[10] = g_Direction.MoveCreatedMonster(10, 120, 219, 0, 12); // 발람4 걸어 나오기
			bSuccess[11] = g_Direction.MoveCreatedMonster(11, 110, 217, 0, 12); // 소람2 걸어 나오기
			bSuccess[12] = g_Direction.MoveCreatedMonster(12, 112, 217, 0, 12); // 소람3 걸어 나오기
			bSuccess[13] = g_Direction.MoveCreatedMonster(13, 114, 217, 0, 12); // 소람4 걸어 나오기
			bSuccess[14] = g_Direction.MoveCreatedMonster(14, 116, 217, 0, 12); // 소람5 걸어 나오기
			bSuccess[15] = g_Direction.MoveCreatedMonster(15, 110, 227, 0, 12); // 탱커1 걸어 나오기
			bSuccess[16] = g_Direction.MoveCreatedMonster(16, 114, 227, 0, 12); // 탱커2 걸어 나오기
			bSuccess[17] = g_Direction.MoveCreatedMonster(17, 118, 227, 0, 12); // 탱커3 걸어 나오기
			bSuccess[18] = g_Direction.MoveCreatedMonster(18, 110, 229, 0, 12); // 소람5 걸어 나오기
			bSuccess[19] = g_Direction.MoveCreatedMonster(19, 112, 229, 0, 12); // 소람6 걸어 나오기
			bSuccess[20] = g_Direction.MoveCreatedMonster(20, 114, 229, 0, 12); // 소람7 걸어 나오기
			bSuccess[21] = g_Direction.MoveCreatedMonster(21, 116, 229, 0, 12); // 소람8 걸어 나오기
			bSuccess[22] = g_Direction.MoveCreatedMonster(22, 118, 229, 0, 12); // 소람9 걸어 나오기
			bSuccess[23] = g_Direction.MoveCreatedMonster(23, 110, 231, 0, 12); // 소람10 걸어 나오기
			bSuccess[24] = g_Direction.MoveCreatedMonster(24, 112, 231, 0, 12); // 소람11 걸어 나오기
			bSuccess[25] = g_Direction.MoveCreatedMonster(25, 114, 231, 0, 12); // 소람12 걸어 나오기
			bSuccess[26] = g_Direction.MoveCreatedMonster(26, 116, 231, 0, 12); // 소람13 걸어 나오기
			bSuccess[27] = g_Direction.MoveCreatedMonster(27, 118, 231, 0, 12); // 소람14 걸어 나오기

			for(int i = 0; i < iMovingMonCount; i++)
			{
				if(!bSuccess[i]) break;
				if(i == iMovingMonCount-1) g_Direction.m_iCheckTime++;
			}
		}
		else if(g_Direction.m_iCheckTime == 3) g_Direction.GetTimeCheck(1000);
		else if(g_Direction.m_iCheckTime == 4) g_Direction.ActionCreatedMonster(0, MONSTER01_ATTACK3, 1, false, true); // 발가스 표효 동작
		else if(g_Direction.m_iCheckTime == 5) g_Direction.GetTimeCheck(2700);
		else if(g_Direction.m_iCheckTime == 6) g_Direction.ActionCreatedMonster(0, MONSTER01_ATTACK4, 1, false, true); // 발가스 표효 동작
		else if(g_Direction.m_iCheckTime == 7) g_Direction.GetTimeCheck(3000);
		else g_Direction.m_bAction = false;
	}
	else
		g_Direction.SetNextDirectionPosition(114, 160, 0, 300.0f);
}

void CMVP1STDirection::BeginDirection3()
{
	if(g_Direction.m_bAction)
	{
		if(g_Direction.m_iCheckTime == 0)
		{
			g_Direction.DeleteMonster();
			g_Direction.m_iCheckTime++;
		}
		else if(g_Direction.m_iCheckTime == 1)
		{
			g_Direction.SummonCreateMonster(340, 110, 77, 0, false, false); // 다크엘프1 소환
			g_Direction.SummonCreateMonster(340, 125, 77, 0, false, false); // 다크엘프2 소환

			g_Direction.SummonCreateMonster(341, 90, 37, 0, false, false); // 소람1 소환
			g_Direction.SummonCreateMonster(341, 108, 73, 0, false, false); // 소람2 소환
			g_Direction.SummonCreateMonster(341, 109, 75, 0, false, false); // 소람3 소환
			g_Direction.SummonCreateMonster(341, 110, 73, 0, false, false); // 소람4 소환
			g_Direction.SummonCreateMonster(341, 111, 75, 0, false, false); // 소람5 소환
			g_Direction.SummonCreateMonster(341, 112, 73, 0, false, false); // 소람6 소환
			g_Direction.SummonCreateMonster(341, 123, 73, 0, false, false); // 소람7 소환
			g_Direction.SummonCreateMonster(341, 124, 75, 0, false, false); // 소람8 소환
			g_Direction.SummonCreateMonster(341, 125, 73, 0, false, false); // 소람9 소환
			g_Direction.SummonCreateMonster(341, 126, 75, 0, false, false); // 소람10 소환
			g_Direction.SummonCreateMonster(341, 127, 73, 0, false, false); // 소람11 소환
			g_Direction.SummonCreateMonster(341, 176, 20, 0, false, false);  // 소람12 소환

			g_Direction.SummonCreateMonster(341, 117, 77, 0, false, false); // 소람13 소환
			g_Direction.SummonCreateMonster(341, 119, 77, 0, false, false); // 소람14 소환
			g_Direction.SummonCreateMonster(341, 121, 77, 0, true, false);  // 소람15 소환

			g_Direction.SummonCreateMonster(340, 119, 83, 0, false, false); // 다크엘프3 소환
	
			g_Direction.SummonCreateMonster(344, 118, 79, 0, false, false);	// 발람1 소환
			g_Direction.SummonCreateMonster(344, 120, 79, 0, false, false);	// 발람2 소환		

			g_Direction.SummonCreateMonster(344, 119, 90, 0, false, false);	// 발람3 소환
			
			g_Direction.SummonCreateMonster(345, 116, 81, 0, false, false);	// 데스스피릿1 소환
			g_Direction.SummonCreateMonster(345, 119, 81, 0, false, false);	// 데스스피릿2 소환
			g_Direction.SummonCreateMonster(345, 122, 81, 0, false, false);	// 데스스피릿3 소환

			g_Direction.SummonCreateMonster(340, 119, 87, 0, false, false); // 다크엘프4 소환

			g_Direction.SummonCreateMonster(348, 116, 90, 0, false, false); // 탱커1 소환
			g_Direction.SummonCreateMonster(348, 122, 90, 0, true, false);  // 탱커2 소환
		}
		else g_Direction.m_bAction = false;
	}
	else
		g_Direction.SetNextDirectionPosition(121, 75, 0, 300.0f);
}

void CMVP1STDirection::BeginDirection4()
{
	if(g_Direction.m_bAction)
	{
		if(g_Direction.m_iCheckTime == 0) g_Direction.GetTimeCheck(3000);
		else g_Direction.m_bAction = false;
	}
	else
		g_Direction.SetNextDirectionPosition(121, 87, 0, 40.0f);
}

void CMVP1STDirection::BeginDirection5()
{
	if(g_Direction.m_bAction)
	{
		if(g_Direction.m_iCheckTime == 0) g_Direction.GetTimeCheck(2000);
		else if(g_Direction.m_iCheckTime == 1)
		{
			g_Direction.ActionCreatedMonster(25, MONSTER01_ATTACK1, 1, true); // 탱커1 공격
			g_Direction.ActionCreatedMonster(26, MONSTER01_ATTACK1, 1, true, true); // 탱커2 공격
		}
		else if(g_Direction.m_iCheckTime == 2) g_Direction.GetTimeCheck(4000);
		else g_Direction.m_bAction = false;
	}
	else
	{
		g_Direction.DeleteMonster();
		g_Direction.m_bOrderExit = true;
	}
}
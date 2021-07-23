// PortalMgr.cpp: implementation of the CPortalMgr class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PortalMgr.h"

#ifdef YDG_ADD_CS5_PORTAL_CHARM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern int World;
const char* GetMapName( int iMap);
bool InChaosCastle(int iMap = World);
bool InBloodCastle(int iMap = World);
bool InDevilSquare();

#include "ZzzBMD.h"
#include "ZzzCharacter.h"

CPortalMgr g_PortalMgr;

CPortalMgr::CPortalMgr()
{
	Reset();
}

CPortalMgr::~CPortalMgr()
{
	Reset();
}

void CPortalMgr::Reset()
{
	ResetPortalPosition();
	ResetRevivePosition();
}

void CPortalMgr::ResetPortalPosition()
{
	m_iPortalWorld = -1;
	m_iPortalPosition_x = 0;
	m_iPortalPosition_y = 0;
}

void CPortalMgr::ResetRevivePosition()
{
	m_iReviveWorld = -1;
	m_iRevivePosition_x = 0;
	m_iRevivePosition_y = 0;
}

BOOL CPortalMgr::IsPortalUsable()
{
	// 이동 명령으로 이동 가능한 지역에서만 이동의 부적을 사용할 수 있다.
	switch(World)
	{
	case WD_6STADIUM:				// 6 경기장
	case WD_0LORENCIA:				// 0 로랜시아
	case WD_3NORIA:					// 3 노리아
	case WD_51HOME_6TH_CHAR:		// 51 엘베란드
	case WD_2DEVIAS:				// 2 데비아스
	case WD_1DUNGEON:				// 1 던전
	case WD_7ATLANSE:				// 7 아틀란스
	case WD_4LOSTTOWER:				// 4 로스트타워
	case WD_8TARKAN:				// 8 타르칸.
	case WD_33AIDA:					// 33 아이다
	case WD_10HEAVEN:				// 10 이카루스
	case WD_37KANTURU_1ST:			// 37 칸투르 폐허
	case WD_38KANTURU_2ND:			// 38 칸투르 유적
#ifdef CSK_ADD_MAP_ICECITY
	case WD_57ICECITY:				// 57 라클리온
#endif // CSK_ADD_MAP_ICECITY
	case WD_56MAP_SWAMP_OF_QUIET:	// 56 평온의 늪
		return TRUE;
	}
	return FALSE;
}

void CPortalMgr::SavePortalPosition()
{
	m_iPortalWorld = World;
	m_iPortalPosition_x = Hero->PositionX;
	m_iPortalPosition_y = Hero->PositionY;
}

void CPortalMgr::SaveRevivePosition()
{
	m_iReviveWorld = World;
	m_iRevivePosition_x = Hero->PositionX;
	m_iRevivePosition_y = Hero->PositionY;
}

BOOL CPortalMgr::IsPortalPositionSaved()
{
	return (m_iPortalWorld != -1);
}

BOOL CPortalMgr::IsRevivePositionSaved()
{
	return (m_iReviveWorld != -1);
}

void CPortalMgr::GetPortalPositionText(char * pszOut)
{
	if (pszOut == NULL) return;

	if (m_iPortalWorld == -1)
	{
		assert(!"이동 위치가 지정되지 않은 상태임");
	}
	else
	{
		sprintf(pszOut, "%s (%d, %d)", GetMapName(m_iPortalWorld), m_iPortalPosition_x, m_iPortalPosition_y);
	}
}

void CPortalMgr::GetRevivePositionText(char * pszOut)
{
	if (pszOut == NULL) return;

	if (m_iReviveWorld == -1)
	{
		assert(!"이동 위치가 지정되지 않은 상태임");
	}
	else
	{
		sprintf(pszOut, "%s (%d, %d)", GetMapName(m_iReviveWorld), m_iRevivePosition_x, m_iRevivePosition_y);
	}
}

#endif	// YDG_ADD_CS5_PORTAL_CHARM
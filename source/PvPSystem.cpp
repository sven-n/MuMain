#include "stdafx.h"
#include "wsctlc.h"
#include "UIWindows.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzLodTerrain.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzTexture.h"
#include "ZzzAI.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzOpenData.h"
#include "ZzzEffect.h"
#include "ZzzScene.h"
#include "DSPlaySound.h"
#include "DIMouse.h"
#include "WSClient.h"
#include "./Utilities/Log/ErrorReport.h"
#include "PvPSystem.h"

#ifdef _PVP_BLOCK_PVP_CHAR

#ifdef _PVP_DYNAMIC_SERVER_TYPE
extern BOOL g_bIsCurrentServerPvP;
#endif	// _PVP_DYNAMIC_SERVER_TYPE

bool CanJoinServer(int PK)
{
#ifdef _PVP_DYNAMIC_SERVER_TYPE
	if (g_bIsCurrentServerPvP == TRUE)
		return TRUE;
	else
		return (PK < PVP_MURDERER1);
#else	// _PVP_DYNAMIC_SERVER_TYPE
	return TRUE;
#endif	// _PVP_DYNAMIC_SERVER_TYPE
}
#endif	// _PVP_BLOCK_PVP_CHAR

#ifdef _PVP_ADD_MOVE_SCROLL

CMurdererMove g_MurdererMove;

void CMurdererMove::Reset()
{
	if (m_szMoveText[0] != '\0')
		memset(m_szMoveText, 0, 256);
	m_iItemIndex = -1;
	m_dwTimer = 0;
	m_iCheckTime = -1;
}

void CMurdererMove::MurdererMove(char * pszMoveText)
{
	if (m_szMoveText[0] != '\0' || m_iItemIndex != -1) return;

	strncpy(m_szMoveText, pszMoveText, 256);
	m_dwTimer = WorldTime;
	m_iCheckTime = -1;
	
	SendRequestCharacterEffect(Hero->Key, 0x20);
	OBJECT * o = &Hero->Object;
	CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,6,o);
}

void CMurdererMove::MurdererMoveByScroll(int iItemIndex)
{
	if (m_szMoveText[0] != '\0' || m_iItemIndex != -1) return;

	m_iItemIndex = iItemIndex;
	m_dwTimer = WorldTime;
	m_iCheckTime = -1;
	
	SendRequestCharacterEffect(Hero->Key, 0x20);
	OBJECT * o = &Hero->Object;
	CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,6,o);
}

void CMurdererMove::CancelMove()
{
	// 이펙트 끈다
	if (m_szMoveText[0] != '\0' || m_iItemIndex != -1)
	{
		g_pChatListBox->AddText("", "이동이 취소되었습니다.", SEASON3B::TYPE_ERROR_MESSAGE);
		Reset();
	}
}

void CMurdererMove::MurdererMoveCheck()
{
	if (m_szMoveText[0] == '\0' && m_iItemIndex == -1) return;

	if ((m_szMoveText[0] != '\0' && Hero->PK < PVP_MURDERER2) || m_dwTimer + g_ciMurderMoveDelay * 1000 < WorldTime)
	{
		m_iCheckTime = g_ciMurderMoveDelay;
		if (m_iItemIndex != -1)
		{
			SendRequestUse(m_iItemIndex, 0);
		}
		else
		{
			SendChat(m_szMoveText);
		}
		Reset();
	}
	else
	{
		char szMurderTemp[256];
		for (int i = g_ciMurderMoveDelay - 1; i >= 0; --i)
		{
			if (m_dwTimer + i * 1000 < WorldTime)
			{
				if (m_iCheckTime < i)
				{
					sprintf(szMurderTemp, "%d초 후에 이동합니다", g_ciMurderMoveDelay - i);
					g_pChatListBox->AddText("", szMurderTemp, SEASON3B::TYPE_SYSTEM_MESSAGE);
					m_iCheckTime = i;
				}
				break;
			}
		}
	}
}
#endif	// _PVP_ADD_MOVE_SCROLL


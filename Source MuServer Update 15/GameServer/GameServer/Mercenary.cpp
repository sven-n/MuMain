// MemScript.cpp: implementation of the CMemScript class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mercenary.h"
#include "CastleSiege.h"
#include "CastleSiegeSync.h"
#include "Guild.h"
#include "Map.h"
#include "Message.h"
#include "Monster.h"
#include "MonsterManager.h"
#include "Notice.h"
#include "Util.h"

CMercenary gMercenary;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMercenary::CMercenary() // OK
{
	#if(GAMESERVER_TYPE==1)

	this->m_MercenaryCount = 0;

	#endif
}

CMercenary::~CMercenary() // OK
{

}

bool CMercenary::CreateMercenary(int aIndex,int MonsterClass,BYTE x,BYTE y) // OK
{
	#if(GAMESERVER_TYPE==1)

	int iIndex = aIndex;
	int iMercenaryTypeIndex = MonsterClass;
	BYTE cTX = x;
	BYTE cTY = y;

	LPOBJ lpObj = &gObj[iIndex];
	int iMonsterIndex = -1;
	BYTE cX = cTX;
	BYTE cY = cTY;

	if(lpObj->Map != MAP_CASTLE_SIEGE || gCastleSiegeSync.GetCastleState() != CASTLESIEGE_STATE_STARTSIEGE)
	{
		gNotice.GCNoticeSend(iIndex,1,0,0,0,0,0,gMessage.GetMessage(464));
		return FALSE;
	}

	if( iMercenaryTypeIndex == 286 || iMercenaryTypeIndex == 287)
	{
		if( lpObj->CsJoinSide != 1)
		{
			gNotice.GCNoticeSend(iIndex,1,0,0,0,0,0,gMessage.GetMessage(465));
			return FALSE;
		}
		if(lpObj->GuildStatus != 0x80 && lpObj->GuildStatus != 0x40)
		{
			gNotice.GCNoticeSend(iIndex,1,0,0,0,0,0,gMessage.GetMessage(466));
				return FALSE;
		}
	}

	if( this->m_MercenaryCount > 100 )
	{
		gNotice.GCNoticeSend(iIndex,1,0,0,0,0,0,gMessage.GetMessage(467));
		return FALSE;
	}

	iMonsterIndex = gObjAddMonster(lpObj->Map);

	if(iMonsterIndex >= 0)
	{
		MONSTER_INFO* lpattr = gMonsterManager.GetInfo(iMercenaryTypeIndex);

		if(lpattr == NULL)
		{
			gObjDel(iMonsterIndex);
			return FALSE;
		}

		gObjSetMonster(iMonsterIndex, iMercenaryTypeIndex);

		gObj[iMonsterIndex].Live = 1;
		gObj[iMonsterIndex].Life = (float)lpattr->Life;
		gObj[iMonsterIndex].MaxLife = (float)lpattr->Life;
		gObj[iMonsterIndex].PosNum = -1;
		gObj[iMonsterIndex].X = cX;
		gObj[iMonsterIndex].Y = cY;
		gObj[iMonsterIndex].MTX = cX;
		gObj[iMonsterIndex].MTY = cY;
		gObj[iMonsterIndex].TX = cX;
		gObj[iMonsterIndex].TY = cY;
		gObj[iMonsterIndex].OldX = cX;
		gObj[iMonsterIndex].OldY = cY;
		gObj[iMonsterIndex].StartX = cX;
		gObj[iMonsterIndex].StartY = cY;
		gObj[iMonsterIndex].Map = lpObj->Map;
		gObj[iMonsterIndex].MoveRange = 0;
		gObj[iMonsterIndex].Level = lpattr->Level;
		gObj[iMonsterIndex].Type = OBJECT_MONSTER;
		gObj[iMonsterIndex].MaxRegenTime = 1000;
		gObj[iMonsterIndex].Dir = 1;
		gObj[iMonsterIndex].RegenTime = GetTickCount();
		gObj[iMonsterIndex].Attribute = 0;
		gObj[iMonsterIndex].DieRegen = 0;
		gObj[iMonsterIndex].CsNpcType = 2;
		gObj[iMonsterIndex].CsJoinSide = 1;

		gNotice.GCNoticeSend(iIndex,1,0,0,0,0,0,gMessage.GetMessage(468));

		this->m_MercenaryCount++;

		if(lpObj->Guild != NULL)
		{
			LogAdd(LOG_BLACK,"[CastleSiege] Mercenary is summoned [%d] - [%d][%d] [%s][%s][%d] - (Guild : %s)",iMonsterIndex, iMercenaryTypeIndex,this->m_MercenaryCount,lpObj->Account,lpObj->Name,lpObj->GuildStatus,lpObj->Guild->Name);
		}
		else
		{
			LogAdd(LOG_BLACK,"[CastleSiege] Mercenary is summoned [%d] - [%d][%d] [%s][%s][%d]",iMonsterIndex, iMercenaryTypeIndex,this->m_MercenaryCount,lpObj->Account,lpObj->Name,lpObj->GuildStatus);
		}
	}
	else
	{
		gNotice.GCNoticeSend(iIndex,1,0,0,0,0,0,gMessage.GetMessage(469));
		return FALSE;
	}

	return TRUE;

	#else

	return 0;

	#endif
}

bool CMercenary::DeleteMercenary(int aIndex) // OK
{
	#if(GAMESERVER_TYPE==1)

	int iIndex = aIndex;

	if ( iIndex < 0 || iIndex > MAX_OBJECT-1 )
	{
		return FALSE;
	}

	this->m_MercenaryCount--;

	LogAdd(LOG_BLACK,"[CastleSiege] Mercenary is broken [%d] - [%d]", iIndex, this->m_MercenaryCount);

	if ( this->m_MercenaryCount < 0 )
	{
		this->m_MercenaryCount = 0;
	}

	return TRUE;

	#else

	return 0;

	#endif
}

bool CMercenary::SearchEnemy(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_TYPE==1)

	int iTargetNumber = -1;
	int iAttackRange = 0;

	lpObj->TargetNumber = -1;

	if(lpObj->Class == 286)
	{
		iAttackRange = 5;
	}
	else if(lpObj->Class == 287)
	{
		iAttackRange = 3;
	}

	int i;
	for(i = 0; i < MAX_VIEWPORT; i++)
	{
		iTargetNumber = lpObj->VpPlayer2[i].index;

		if(iTargetNumber >= 0 && gObj[iTargetNumber].Type == 1 && gObj[iTargetNumber].Live != FALSE && gObj[iTargetNumber].Teleport == FALSE)
		{
			if(gObj[iTargetNumber].CsJoinSide == lpObj->CsJoinSide)
			{
				continue;
			}

			int iTargetDisX = lpObj->X - gObj[iTargetNumber].X;
			int iTargetDisY = lpObj->Y - gObj[iTargetNumber].Y;
			int iTargetDist;
			int cY;
			int cX;
			int SquareDist = (iTargetDisX*iTargetDisX)+(iTargetDisY*iTargetDisY);

			iTargetDist = (long)sqrt((float)SquareDist);
			
			//lpObj->VpPlayer2[i].dis = iTargetDist;

			if(lpObj->Dir == 1 && abs(iTargetDisX) <= 2)
			{
				cY = lpObj->Y - iAttackRange;
				if(cY <= gObj[iTargetNumber].Y)
				{
					if(lpObj->Y >= gObj[iTargetNumber].Y)
					{
						lpObj->TargetNumber = iTargetNumber;
						return 1;
					}
				}
			}

			if(lpObj->Dir == 3 && abs(iTargetDisY) <= 2)
			{
				cX = lpObj->X - iAttackRange;
				if(cX <= gObj[iTargetNumber].X)
				{
					if(lpObj->X >= gObj[iTargetNumber].X)
					{
						lpObj->TargetNumber = iTargetNumber;
						return 1;
					}
				}
			}

		}
	}

	return 0;

	#else

	return 0;

	#endif
}

void CMercenary::MercenaryAct(int aIndex) // OK
{
	#if(GAMESERVER_TYPE==1)

	int iIndex = aIndex;
	if ( ::gObjIsConnected(iIndex) == FALSE )
	{
		return;
	}

	LPOBJ lpObj = &gObj[iIndex];

	if ( lpObj->VPCount2 < 1 )
	{
		return;
	}

	if ( this->SearchEnemy(lpObj) != 0 && lpObj->TargetNumber >= 0)
	{
		lpObj->ActionState.Attack = 1;
		lpObj->NextActionTime = lpObj->PhysiSpeed;
	}
	else
	{
		lpObj->NextActionTime = lpObj->MoveSpeed;
	}

	#endif
}

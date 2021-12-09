// LifeStone.cpp: implementation of the CLifeStone class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LifeStone.h"
#include "CastleSiege.h"
#include "CastleSiegeSync.h"
#include "Guild.h"
#include "Map.h"
#include "Message.h"
#include "Monster.h"
#include "MonsterManager.h"
#include "Notice.h"
#include "Protocol.h"
#include "Util.h"

CLifeStone gLifeStone;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLifeStone::CLifeStone() // OK
{

}

CLifeStone::~CLifeStone() // OK
{

}

bool CLifeStone::CreateLifeStone(int aIndex) // OK
{
	#if(GAMESERVER_TYPE==1)

	int iIndex = aIndex;
	LPOBJ lpObj = &gObj[iIndex];
	int iMonsterIndex = -1;
	BYTE cX = (BYTE)lpObj->X;
	BYTE cY = (BYTE)lpObj->Y;

	if ( gObj[iIndex].Map != MAP_CASTLE_SIEGE || gCastleSiegeSync.GetCastleState() != CASTLESIEGE_STATE_STARTSIEGE )
	{
		gNotice.GCNoticeSend(iIndex,1,0,0,0,0,0,gMessage.GetMessage(448));
		return FALSE;
	}

	if ( lpObj->GuildStatus != 0x80 )
	{
		return FALSE;
	}

	if ( lpObj->CsJoinSide < 2 )
	{
		gNotice.GCNoticeSend(iIndex,1,0,0,0,0,0,gMessage.GetMessage(449));
		return FALSE;
	}

	if ( lpObj->Guild->LifeStone )
	{
		gNotice.GCNoticeSend(iIndex,1,0,0,0,0,0,gMessage.GetMessage(450));
		return FALSE;
	}

	if( cX > 150 && cX < 210 && cY > 175 && cY < 230 )
	{
		gNotice.GCNoticeSend(iIndex,1,0,0,0,0,0,gMessage.GetMessage(451));
		return FALSE;
	}

	iMonsterIndex = gObjAddMonster(lpObj->Map);

	if ( iMonsterIndex >= 0 )
	{
	
		MONSTER_INFO * MAttr = gMonsterManager.GetInfo(278);
	
		if ( MAttr == NULL )
		{
			gObjDel(iMonsterIndex);
			return FALSE;
		}

		gObjSetMonster(iMonsterIndex, 278);
		gObj[iMonsterIndex].Live = TRUE;
		gObj[iMonsterIndex].Life = (float)MAttr->Life;
		gObj[iMonsterIndex].MaxLife = (float)MAttr->Life;
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
		gObj[iMonsterIndex].Level = MAttr->Level;
		gObj[iMonsterIndex].Type = OBJECT_MONSTER;
		gObj[iMonsterIndex].MaxRegenTime = 1000;
		gObj[iMonsterIndex].Dir = 1;
		gObj[iMonsterIndex].RegenTime = GetTickCount();
		gObj[iMonsterIndex].Attribute = 0;
		gObj[iMonsterIndex].DieRegen = 0;
		gObj[iMonsterIndex].CsNpcType = OBJECT_NPC;
		gObj[iMonsterIndex].CsJoinSide = lpObj->CsJoinSide;
		gObj[iMonsterIndex].Guild = lpObj->Guild;
		gObj[iMonsterIndex].CreationState = 0;

		lpObj->Guild->LifeStone = &gObj[iMonsterIndex];
		gObj[iMonsterIndex].CreatedActivationTime = 0;

		gNotice.GCNoticeSend(iIndex,1,0,0,0,0,0,gMessage.GetMessage(452));
		LogAdd(LOG_BLACK,"[CastleSiege] LifeStone is created - [%s] [%s][%s] (Map:%d)(X:%d, Y:%d)",lpObj->Guild->Name,
		lpObj->Account,lpObj->Name,lpObj->Map,cX,cY);
		lpObj->LifeStoneCount++;
	}
	else
	{
		gNotice.GCNoticeSend(iIndex,1,0,0,0,0,0,gMessage.GetMessage(453));
		return FALSE;
	}
	return TRUE;

	#else

	return 0;

	#endif
}

bool CLifeStone::DeleteLifeStone(int aIndex) // OK
{
	#if(GAMESERVER_TYPE==1)

	int iIndex = aIndex;
	if(iIndex < 0 || iIndex > MAX_OBJECT - 1)
	{
		return FALSE;
	}

	LPOBJ lpLifeStone = &gObj[iIndex];

	if(lpLifeStone->Guild)
	{
		LogAdd(LOG_BLACK,"[CastleSiege] LifeStone is broken - [%s]",lpLifeStone->Guild->Name);
		lpLifeStone->Guild->LifeStone = 0;
	}
	return TRUE;

	#else

	return 0;

	#endif
}

bool CLifeStone::SetReSpawnUserXY(int aIndex) // OK
{
	#if(GAMESERVER_TYPE==1)

	int iUserIndex = aIndex;
	if(!gObjIsConnected(iUserIndex))
	{
		return FALSE;
	}

	LPOBJ lpUser = &gObj[iUserIndex];

	if(lpUser->Map != MAP_CASTLE_SIEGE)
	{
		return FALSE;
	}

	if(!lpUser->Guild)
	{
		return FALSE;
	}

	BYTE btCsJoinSide = lpUser->CsJoinSide;

	if(lpUser->Guild->LifeStone == 0)
	{
		return FALSE;
	}

	LPOBJ lpLifeStone = lpUser->Guild->LifeStone;

	if(lpLifeStone->CreatedActivationTime < 60)
	{
		return FALSE;
	}

	lpUser->Map = MAP_CASTLE_SIEGE;
	lpUser->X = lpLifeStone->X;
	lpUser->Y = lpLifeStone->Y;
	return TRUE;

	#else

	return 0;

	#endif
}

void CLifeStone::LifeStoneAct(int aIndex) // OK
{
	#if(GAMESERVER_TYPE==1)

	int iIndex = aIndex;
	if(!gObjIsConnected(iIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[iIndex];

	lpObj->CreatedActivationTime++;
	BYTE btCreationState = lpObj->CreationState;

	if(lpObj->CreatedActivationTime < 60 )
	{
		lpObj->CreationState = (lpObj->CreatedActivationTime / 12);
	}
	else
	{
		lpObj->CreationState = 5;
	}

	if(btCreationState != lpObj->CreationState)
	{
		GCCreationStateSend(iIndex,lpObj->CreationState);
	}

	if(lpObj->CreationState < 5)
	{
		return;
	}

	if(lpObj->VPCount < 1)
	{
		return;
	}

	int tObjNum = -1;

	for(int i = 0; i < MAX_VIEWPORT; i++)
	{
		BOOL iRecoverLife,iRecoverMana,iRecoverBP;

		tObjNum = lpObj->VpPlayer[i].index;

		if(tObjNum >= 0)
		{
			if(gObj[tObjNum].Type == OBJECT_USER)
			{
				if(gObj[tObjNum].Live != FALSE)
				{
					if(gObj[tObjNum].CsJoinSide == lpObj->CsJoinSide)
					{
						if(abs(lpObj->Y - gObj[tObjNum].Y) <= 3)
						{
							if(abs(lpObj->X - gObj[tObjNum].X) <= 3)
							{
								iRecoverLife = FALSE;
								iRecoverMana = FALSE;
								iRecoverBP = FALSE;

								if(gObj[tObjNum].Life < (gObj[tObjNum].MaxLife + gObj[tObjNum].AddLife))
								{
									gObj[tObjNum].Life += ((gObj[tObjNum].MaxLife + gObj[tObjNum].AddLife) / 100 );

									if(gObj[tObjNum].Life > (gObj[tObjNum].MaxLife + gObj[tObjNum].AddLife))
									{
										gObj[tObjNum].Life = (gObj[tObjNum].MaxLife + gObj[tObjNum].AddLife);
									}
									iRecoverLife = TRUE;
								}

								if(gObj[tObjNum].Mana < (gObj[tObjNum].MaxMana + gObj[tObjNum].AddMana) )
								{
									gObj[tObjNum].Mana += ((gObj[tObjNum].MaxMana + gObj[tObjNum].AddMana) / 100 );

									if(gObj[tObjNum].Mana > (gObj[tObjNum].MaxMana + gObj[tObjNum].AddMana))
									{
										gObj[tObjNum].Mana = (gObj[tObjNum].MaxMana + gObj[tObjNum].AddMana);
									}
									iRecoverMana = TRUE;
								}

								if(gObj[tObjNum].BP < (gObj[tObjNum].MaxBP + gObj[tObjNum].AddBP))
								{
									gObj[tObjNum].BP += ((gObj[tObjNum].MaxBP + gObj[tObjNum].AddBP) / 100);

									if(gObj[tObjNum].BP > (gObj[tObjNum].MaxBP + gObj[tObjNum].AddBP))
									{
										gObj[tObjNum].BP = (gObj[tObjNum].MaxBP + gObj[tObjNum].AddBP);
									}
									iRecoverBP = TRUE;
								}

								if(iRecoverLife != FALSE)
								{
									GCLifeSend(tObjNum,0xFF,(int)gObj[tObjNum].Life,gObj[tObjNum].Shield);
								}

								if( (iRecoverMana != FALSE) || (iRecoverBP != FALSE) )
								{
									GCManaSend(tObjNum,0xFF,(int)gObj[tObjNum].Mana,gObj[tObjNum].BP);
								}
							}
						}
					}
				}
			}
		}
		continue;
	}

	#endif
}

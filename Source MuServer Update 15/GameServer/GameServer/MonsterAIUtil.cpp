// MonsterAIUtil.cpp: implementation of the CMonsterAIUtil class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAIUtil.h"
#include "EffectManager.h"
#include "Map.h"
#include "MapPath.h"
#include "SocketManagerModern.h"
#include "Util.h"
#include "Viewport.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAIUtil::CMonsterAIUtil() // OK
{

}

CMonsterAIUtil::~CMonsterAIUtil() // OK
{

}

BOOL CMonsterAIUtil::FindPathToMoveMonster(LPOBJ lpObj,int tx,int ty,int MaxPathCount,bool PreventOverMoving) // OK
{
	if(CMonsterAIUtil::CheckMovingCondition(lpObj) == 0)
	{
		return 0;
	}

	PATH_INFO path;

	if(PreventOverMoving == 0)
	{
		if(gMap[lpObj->Map].PathFinding4(lpObj->X,lpObj->Y,tx,ty,&path) == 0)
		{
			return 0;
		}
	}
	else
	{
		if(gMap[lpObj->Map].PathFinding2(lpObj->X,lpObj->Y,tx,ty,&path) == 0)
		{
			return 0;
		}
	}

	lpObj->TX = lpObj->X;
	lpObj->TY = lpObj->Y;
	lpObj->MTX= lpObj->X;
	lpObj->MTY = lpObj->Y;
	lpObj->OldX = lpObj->X;
	lpObj->OldY = lpObj->Y;

	gMap[lpObj->Map].DelStandAttr(lpObj->X,lpObj->Y);

	lpObj->PathCur = 1;
	lpObj->PathCount = ((path.PathNum>MaxPathCount)?MaxPathCount:path.PathNum);
	lpObj->PathStartEnd = 1;
	lpObj->PathX[0] = lpObj->X;
	lpObj->PathY[0] = lpObj->Y;
	lpObj->PathDir[0] = lpObj->Dir;
	lpObj->LastMoveTime = GetTickCount();

	for(int n=1;n < lpObj->PathCount;n++)
	{
		lpObj->PathX[n] = path.PathX[n];
		lpObj->PathY[n] = path.PathY[n];
		lpObj->PathDir[n] = GetPathPacketDirPos((lpObj->PathX[n]-lpObj->PathX[(n-1)]),(lpObj->PathY[n]-lpObj->PathY[(n-1)]));
		lpObj->TX += RoadPathTable[(lpObj->PathDir[n]*2)+0];
		lpObj->TY += RoadPathTable[(lpObj->PathDir[n]*2)+1];
		lpObj->MTX += RoadPathTable[(lpObj->PathDir[n]*2)+0];
		lpObj->MTY += RoadPathTable[(lpObj->PathDir[n]*2)+1];
	}

	gMap[lpObj->Map].SetStandAttr(lpObj->TX,lpObj->TY);

	CMonsterAIUtil::SendMonsterMoveMsg(lpObj);

	return 1;
}

void CMonsterAIUtil::SendMonsterMoveMsg(LPOBJ lpObj) // OK
{
	PMSG_MOVE_SEND pMsg;

	#if(NEW_PROTOCOL_SYSTEM==0)
	pMsg.header.set(PROTOCOL_CODE1,sizeof(pMsg));
	#endif

	pMsg.index[0] = SET_NUMBERHB(lpObj->Index);

	pMsg.index[1] = SET_NUMBERLB(lpObj->Index);

	pMsg.x = (BYTE)lpObj->MTX;

	pMsg.y = (BYTE)lpObj->MTY;

	pMsg.dir = lpObj->Dir << 4;

	#if(NEW_PROTOCOL_SYSTEM==1)

	uint16_t size = sizeof(pMsg);

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		if(lpObj->VpPlayer2[n].state != VIEWPORT_NONE && lpObj->VpPlayer2[n].type == OBJECT_USER)
		{
			gSocketManagerModern.PacketSend(lpObj->VpPlayer2[n].index,ProtocolHead::BOTH_MOVE,(uint8_t*)&pMsg,size);
		}
	}
	#else
		SendMonsterV2Msg(lpObj,(BYTE*)&pMsg,pMsg.header.size);
	#endif


}

BOOL CMonsterAIUtil::CheckMovingCondition(LPOBJ lpObj) // OK
{
	if(lpObj->Live == 0 || lpObj->RegenOk > 0 || lpObj->Teleport != 0 || lpObj->State != OBJECT_PLAYING)
	{
		return 0;
	}

	if(gEffectManager.CheckStunEffect(lpObj) != 0 || gEffectManager.CheckImmobilizeEffect(lpObj) != 0)
	{
		return 0;
	}

	return 1;
}

BOOL CMonsterAIUtil::GetXYToPatrol(LPOBJ lpObj) // OK
{
	lpObj->NextActionTime = 1000;

	for(int n=0;n < 10;n++)
	{
		int tx = (lpObj->X-lpObj->MoveRange)+(GetLargeRand()%((lpObj->MoveRange*2)+1));
		int ty = (lpObj->X-lpObj->MoveRange)+(GetLargeRand()%((lpObj->MoveRange*2)+1));

		if(gMap[lpObj->Map].CheckAttr(tx,ty,1) == 0 && gMap[lpObj->Map].CheckAttr(tx,ty,2) == 0 && gMap[lpObj->Map].CheckAttr(tx,ty,4) == 0 && gMap[lpObj->Map].CheckAttr(tx,ty,8) == 0)
		{
			lpObj->MTX = tx;
			lpObj->MTY = ty;
			return 1;
		}
	}

	return 0;
}

BOOL CMonsterAIUtil::GetXYToEascape(LPOBJ lpObj) // OK
{
	if(OBJECT_RANGE(lpObj->TargetNumber) == 0)
	{
		return 0;
	}

	int sx = lpObj->X;
	int sy = lpObj->Y;
	int tx = gObj[lpObj->TargetNumber].X;
	int ty = gObj[lpObj->TargetNumber].Y;

	if(lpObj->X < tx)
	{
		sx -= (int)(lpObj->AttackRange/sqrt(2.0));
	}

	if(lpObj->X > tx)
	{
		sx += (int)(lpObj->AttackRange/sqrt(2.0));
	}

	if(lpObj->Y < ty)
	{
		sy -= (int)(lpObj->AttackRange/sqrt(2.0));
	}

	if(lpObj->Y > ty)
	{
		sy += (int)(lpObj->AttackRange/sqrt(2.0));
	}

	int PathValue = GetPathPacketDirPos((lpObj->X-sx),(lpObj->Y-sy))*2;

	if(gMap[lpObj->Map].CheckStandAttr(sx,sy) == 0)
	{
		for(int n=0;n < (MAX_ROAD_PATH_TABLE/2);n++)
		{
			tx = lpObj->X+RoadPathTable[PathValue++];
			ty = lpObj->Y+RoadPathTable[PathValue++];

			if(gMap[lpObj->Map].CheckAttr(tx,ty,1) == 0 && gMap[lpObj->Map].CheckAttr(tx,ty,2) == 0 && gMap[lpObj->Map].CheckAttr(tx,ty,4) == 0 && gMap[lpObj->Map].CheckAttr(tx,ty,8) == 0)
			{
				lpObj->MTX = tx;
				lpObj->MTY = ty;
				return 1;
			}

			PathValue = ((PathValue>=MAX_ROAD_PATH_TABLE)?0:PathValue);
		}
	}
	else
	{
		if(gMap[lpObj->Map].CheckAttr(sx,sy,1) == 0 && gMap[lpObj->Map].CheckAttr(sx,sy,2) == 0 && gMap[lpObj->Map].CheckAttr(sx,sy,4) == 0 && gMap[lpObj->Map].CheckAttr(sx,sy,8) == 0)
		{
			lpObj->MTX = sx;
			lpObj->MTY = sy;
			return 1;
		}
	}

	return 0;
}

BOOL CMonsterAIUtil::GetXYToChase(LPOBJ lpObj) // OK
{
	if(OBJECT_RANGE(lpObj->TargetNumber) == 0)
	{
		return 0;
	}

	int sx = gObj[lpObj->TargetNumber].X;
	int sy = gObj[lpObj->TargetNumber].Y;
	int tx = gObj[lpObj->TargetNumber].X;
	int ty = gObj[lpObj->TargetNumber].Y;

	if(lpObj->X < tx)
	{
		sx -= (int)(lpObj->AttackRange/sqrt(2.0));
	}

	if(lpObj->X > tx)
	{
		sx += (int)(lpObj->AttackRange/sqrt(2.0));
	}

	if(lpObj->Y < ty)
	{
		sy -= (int)(lpObj->AttackRange/sqrt(2.0));
	}

	if(lpObj->Y > ty)
	{
		sy += (int)(lpObj->AttackRange/sqrt(2.0));
	}

	int PathValue = GetPathPacketDirPos((gObj[lpObj->TargetNumber].X-sx),(gObj[lpObj->TargetNumber].Y-sy))*2;

	if(gMap[lpObj->Map].CheckStandAttr(sx,sy) == 0)
	{
		for(int n=0;n < (MAX_ROAD_PATH_TABLE/2);n++)
		{
			tx = gObj[lpObj->TargetNumber].X+RoadPathTable[PathValue++];
			ty = gObj[lpObj->TargetNumber].Y+RoadPathTable[PathValue++];

			if(gMap[lpObj->Map].CheckAttr(tx,ty,1) == 0 && gMap[lpObj->Map].CheckAttr(tx,ty,2) == 0 && gMap[lpObj->Map].CheckAttr(tx,ty,4) == 0 && gMap[lpObj->Map].CheckAttr(tx,ty,8) == 0)
			{
				lpObj->MTX = tx;
				lpObj->MTY = ty;
				return 1;
			}

			PathValue = ((PathValue>=MAX_ROAD_PATH_TABLE)?0:PathValue);
		}
	}
	else
	{
		if(gMap[lpObj->Map].CheckAttr(sx,sy,1) == 0 && gMap[lpObj->Map].CheckAttr(sx,sy,2) == 0 && gMap[lpObj->Map].CheckAttr(sx,sy,4) == 0 && gMap[lpObj->Map].CheckAttr(sx,sy,8) == 0)
		{
			lpObj->MTX = sx;
			lpObj->MTY = sy;
			return 1;
		}
	}

	return 0;
}

BOOL CMonsterAIUtil::FindMonViewportObj(int aIndex,int bIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0 || OBJECT_RANGE(bIndex) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		if(gObj[aIndex].VpPlayer[n].state != VIEWPORT_NONE)
		{
			if(gObj[aIndex].VpPlayer[n].index == bIndex)
			{
				return 1;
			}
		}
	}

	return 0;
}

BOOL CMonsterAIUtil::FindMonViewportObj2(int aIndex,int bIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0 || OBJECT_RANGE(bIndex) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		if(gObj[aIndex].VpPlayer2[n].state != VIEWPORT_NONE)
		{
			if(gObj[aIndex].VpPlayer2[n].index == bIndex)
			{
				return 1;
			}
		}
	}

	return 0;
}

long CMonsterAIUtil::FindMonsterToHeal(LPOBJ lpObj,int rate,int distance) // OK
{
	int TargetIndex = -1;
	int TargetValue = rate;

	for(int n=OBJECT_START_MONSTER;n < MAX_OBJECT_MONSTER;n++)
	{
		if(gObjIsConnected(n) != 0 && gObj[n].Type == OBJECT_MONSTER && gObj[n].Live != 0 && gObj[n].State == OBJECT_PLAYING)
		{
			if(lpObj->Index != n && lpObj->Map == gObj[n].Map && gObjCalcDistance(lpObj,&gObj[n]) < distance)
			{
				if(lpObj->Class != 524 && lpObj->Class != 525 && lpObj->Class != 526 && lpObj->Class != 527 && lpObj->Class != 528)
				{
					if(((gObj[n].Life*100)/(gObj[n].MaxLife+gObj[n].AddLife)) < TargetValue)
					{
						TargetIndex = n;
						TargetValue = (int)((gObj[n].Life*100)/(gObj[n].MaxLife+gObj[n].AddLife));
					}
				}
			}
		}
	}

	return TargetIndex;
}

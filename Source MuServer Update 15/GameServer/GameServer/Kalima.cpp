// Kalima.cpp: implementation of the CKalima class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Kalima.h"
#include "Map.h"
#include "Monster.h"
#include "ObjectManager.h"

CKalima gKalima;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKalima::CKalima() // OK
{
	InitializeCriticalSection(&this->m_critical);
}

CKalima::~CKalima() // OK
{
	DeleteCriticalSection(&this->m_critical);
}

int CKalima::GetKalimaGateLevel(LPOBJ lpObj) // OK
{
	int level = -1;

	if(lpObj->Class == CLASS_MG || lpObj->Class == CLASS_DL || lpObj->Class == CLASS_RF)
	{
		if(lpObj->Level >= 20 && lpObj->Level <= 110)
		{
			level = 0;
		}
		else if(lpObj->Level >= 111 && lpObj->Level <= 160)
		{
			level = 1;
		}
		else if(lpObj->Level >= 161 && lpObj->Level <= 210)
		{
			level = 2;
		}
		else if(lpObj->Level >= 211 && lpObj->Level <= 260)
		{
			level = 3;
		}
		else if(lpObj->Level >= 261 && lpObj->Level <= 310)
		{
			level = 4;
		}
		else if(lpObj->Level >= 311 && lpObj->Level <= 349)
		{
			level = 5;
		}
		else if(lpObj->Level >= 350 && lpObj->Level <= MAX_CHARACTER_LEVEL)
		{
			level = 6;
		}
	}
	else
	{
		if(lpObj->Level >= 40 && lpObj->Level <= 130)
		{
			level = 0;
		}
		else if(lpObj->Level >= 131 && lpObj->Level <= 180)
		{
			level = 1;
		}
		else if(lpObj->Level >= 181 && lpObj->Level <= 230)
		{
			level = 2;
		}
		else if(lpObj->Level >= 231 && lpObj->Level <= 280)
		{
			level = 3;
		}
		else if(lpObj->Level >= 281 && lpObj->Level <= 330)
		{
			level = 4;
		}
		else if(lpObj->Level >= 331 && lpObj->Level <= 349)
		{
			level = 5;
		}
		else if(lpObj->Level >= 350 && lpObj->Level <= MAX_CHARACTER_LEVEL)
		{
			level = 6;
		}
	}

	return level;
}

bool CKalima::CreateKalimaGate(int aIndex,int level,int x,int y) // OK
{
	EnterCriticalSection(&this->m_critical);

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	int KalimaGateLevel = level-1;

	if(KalimaGateLevel < 0 || KalimaGateLevel >= MAX_KALIMA_LEVEL)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	if(this->GetKalimaGateLevel(lpObj) == -1 || this->GetKalimaGateLevel(lpObj) < KalimaGateLevel)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	if(lpObj->Map == MAP_ICARUS)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	if(DS_MAP_RANGE(lpObj->Map) != 0 || BC_MAP_RANGE(lpObj->Map) != 0 || CC_MAP_RANGE(lpObj->Map) != 0 || IT_MAP_RANGE(lpObj->Map) != 0 || DA_MAP_RANGE(lpObj->Map) != 0 || DG_MAP_RANGE(lpObj->Map) != 0 || IG_MAP_RANGE(lpObj->Map) != 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	if(KALIMA_MAP_RANGE(lpObj->Map) != 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	if(lpObj->KalimaGateExist != 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	int px = x;
	int py = y;

	if(gMap[lpObj->Map].CheckAttr(px,py,255) != 0)
	{
		if(gObjGetRandomFreeLocation(lpObj->Map,&px,&py,3,3,50) == 0)
		{
			LeaveCriticalSection(&this->m_critical);
			return 0;
		}
	}

	int index = gObjAddMonster(lpObj->Map);

	if(OBJECT_RANGE(index) == 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	LPOBJ lpMonster = &gObj[index];

	lpMonster->PosNum = -1;
	lpMonster->X = px;
	lpMonster->Y = py;
	lpMonster->TX = px;
	lpMonster->TY = py;
	lpMonster->OldX = px;
	lpMonster->OldY = py;
	lpMonster->StartX = px;
	lpMonster->StartY = py;
	lpMonster->Dir = 1;
	lpMonster->Map = lpObj->Map;

	if(gObjSetMonster(index,(152+KalimaGateLevel)) == 0)
	{
		gObjDel(index);
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	lpMonster->MoveRange = 0;
	lpMonster->DieRegen = 0;
	lpMonster->RegenTime = GetTickCount();
	lpMonster->MaxRegenTime = 0;
	lpMonster->Attribute = 51+KalimaGateLevel;
	lpMonster->SummonIndex = aIndex;
	lpMonster->KalimaGateEnterCount = 0;

	lpObj->KalimaGateExist = 1;
	lpObj->KalimaGateIndex = index;

	LeaveCriticalSection(&this->m_critical);
	return 1;
}

bool CKalima::CreateNextKalimaGate(int aIndex,int map,int x,int y) // OK
{
	EnterCriticalSection(&this->m_critical);

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(map < MAP_KALIMA1 || map > MAP_KALIMA6)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	if(lpObj->KalimaGateExist != 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	int px = x;
	int py = y;

	if(gMap[lpObj->Map].CheckAttr(px,py,255) != 0)
	{
		if(gObjGetRandomFreeLocation(lpObj->Map,&px,&py,3,3,50) == 0)
		{
			LeaveCriticalSection(&this->m_critical);
			return 0;
		}
	}

	int index = gObjAddMonster(lpObj->Map);

	if(OBJECT_RANGE(index) == 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	LPOBJ lpMonster = &gObj[index];

	lpMonster->PosNum = -1;
	lpMonster->X = px;
	lpMonster->Y = py;
	lpMonster->TX = px;
	lpMonster->TY = py;
	lpMonster->OldX = px;
	lpMonster->OldY = py;
	lpMonster->StartX = px;
	lpMonster->StartY = py;
	lpMonster->Dir = 1;
	lpMonster->Map = lpObj->Map;

	if(gObjSetMonster(index,(129+lpObj->Map)) == 0)
	{
		gObjDel(index);
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	lpMonster->MoveRange = 0;
	lpMonster->DieRegen = 0;
	lpMonster->RegenTime = GetTickCount();
	lpMonster->MaxRegenTime = 0;
	lpMonster->Attribute = 58;
	lpMonster->SummonIndex = aIndex;
	lpMonster->KalimaGateEnterCount = 0;

	lpObj->KalimaGateExist = 1;
	lpObj->KalimaGateIndex = index;

	LeaveCriticalSection(&this->m_critical);
	return 1;
}

bool CKalima::DeleteKalimaGate(int aIndex) // OK
{
	EnterCriticalSection(&this->m_critical);

	if(OBJECT_RANGE(aIndex) == 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	int index = gObj[aIndex].KalimaGateIndex;

	if(OBJECT_RANGE(index) == 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	if(gObj[index].Live == 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	if(KALIMA_ATTRIBUTE_RANGE(gObj[index].Attribute) == 0)
	{
		LeaveCriticalSection(&this->m_critical);
		return 0;
	}

	GCUserDieSend(&gObj[index],index,0,0);

	gObjDel(index);

	gObj[aIndex].KalimaGateExist = 0;
	gObj[aIndex].KalimaGateIndex = -1;

	LeaveCriticalSection(&this->m_critical);
	return 1;
}

void CKalima::KalimaGateAct(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Live == 0)
	{
		return;
	}

	if(gObjIsConnectedGP(lpObj->SummonIndex) == 0)
	{
		GCUserDieSend(lpObj,aIndex,0,0);
		gObjDel(aIndex);
		return;
	}

	LPOBJ lpTarget = &gObj[lpObj->SummonIndex];

	if(lpTarget->DieRegen != 0)
	{
		this->DeleteKalimaGate(lpTarget->Index);
		return;
	}

	if(lpObj->KalimaGateEnterCount >= MAX_KALIMA_ENTER)
	{
		this->DeleteKalimaGate(lpTarget->Index);
		return;
	}

	if((GetTickCount()-lpObj->RegenTime) >= MAX_KALIMA_STAND)
	{
		this->DeleteKalimaGate(lpTarget->Index);
		return;
	}

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		int index = lpObj->VpPlayer[n].index;

		if(OBJECT_RANGE(index) == 0)
		{
			continue;
		}

		if(gObj[index].Type != OBJECT_USER)
		{
			continue;
		}

		if(gObj[index].Live == 0)
		{
			continue;
		}

		if(gObj[index].X < (lpObj->X-2) || gObj[index].X > (lpObj->X+2) || gObj[index].Y < (lpObj->Y-2) || gObj[index].Y > (lpObj->Y+2))
		{
			continue;
		}

		if(index != lpTarget->Index && (OBJECT_RANGE(gObj[index].PartyNumber) == 0 || gObj[index].PartyNumber != lpTarget->PartyNumber))
		{
			continue;
		}

		int KalimaGateLevel = lpObj->Attribute-51;

		if(KalimaGateLevel < 0 || KalimaGateLevel >= MAX_KALIMA_LEVEL)
		{
			continue;
		}

		if(this->GetKalimaGateLevel(&gObj[index]) == -1 || this->GetKalimaGateLevel(&gObj[index]) < KalimaGateLevel)
		{
			continue;
		}

		switch(KalimaGateLevel)
		{
			case 0:
				gObjMoveGate(index,88);
				break;
			case 1:
				gObjMoveGate(index,89);
				break;
			case 2:
				gObjMoveGate(index,90);
				break;
			case 3:
				gObjMoveGate(index,91);
				break;
			case 4:
				gObjMoveGate(index,92);
				break;
			case 5:
				gObjMoveGate(index,93);
				break;
			case 6:
				gObjMoveGate(index,116);
				break;
		}

		if((lpObj->KalimaGateEnterCount++) >= MAX_KALIMA_ENTER)
		{
			break;
		}
	}
}

void CKalima::KalimaNextGateAct(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Live == 0)
	{
		return;
	}

	if(gObjIsConnectedGP(lpObj->SummonIndex) == 0)
	{
		GCUserDieSend(lpObj,aIndex,0,0);
		gObjDel(aIndex);
		return;
	}

	LPOBJ lpTarget = &gObj[lpObj->SummonIndex];

	if(lpTarget->DieRegen != 0)
	{
		this->DeleteKalimaGate(lpTarget->Index);
		return;
	}

	if(lpObj->KalimaGateEnterCount >= MAX_KALIMA_ENTER)
	{
		this->DeleteKalimaGate(lpTarget->Index);
		return;
	}

	if((GetTickCount()-lpObj->RegenTime) >= MAX_KALIMA_NEXT_STAND)
	{
		this->DeleteKalimaGate(lpTarget->Index);
		return;
	}

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		int index = lpObj->VpPlayer[n].index;

		if(OBJECT_RANGE(index) == 0)
		{
			continue;
		}

		if(gObj[index].Type != OBJECT_USER)
		{
			continue;
		}

		if(gObj[index].Live == 0)
		{
			continue;
		}

		if(gObj[index].X < (lpObj->X-2) || gObj[index].X > (lpObj->X+2) || gObj[index].Y < (lpObj->Y-2) || gObj[index].Y > (lpObj->Y+2))
		{
			continue;
		}

		if(index != lpTarget->Index && (OBJECT_RANGE(gObj[index].PartyNumber) == 0 || gObj[index].PartyNumber != lpTarget->PartyNumber))
		{
			continue;
		}

		int KalimaGateLevel = lpObj->Class-152;

		if(KalimaGateLevel < 0 || KalimaGateLevel >= MAX_KALIMA_LEVEL)
		{
			continue;
		}

		switch(KalimaGateLevel)
		{
			case 0:
				gObjMoveGate(index,88);
				break;
			case 1:
				gObjMoveGate(index,89);
				break;
			case 2:
				gObjMoveGate(index,90);
				break;
			case 3:
				gObjMoveGate(index,91);
				break;
			case 4:
				gObjMoveGate(index,92);
				break;
			case 5:
				gObjMoveGate(index,93);
				break;
			case 6:
				gObjMoveGate(index,116);
				break;
		}

		if((lpObj->KalimaGateEnterCount++) >= MAX_KALIMA_ENTER)
		{
			break;
		}
	}
}

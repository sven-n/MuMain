// RaklionUtil.cpp: implementation of the CRaklionUtil class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RaklionUtil.h"
#include "Map.h"
#include "Notice.h"
#include "Util.h"

CRaklionUtil gRaklionUtil;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRaklionUtil::CRaklionUtil() // OK
{

}

CRaklionUtil::~CRaklionUtil() // OK
{

}

void CRaklionUtil::NotifyRaklionState(int aIndex,int state,int DetailState) // OK
{
	PMSG_RAKLION_STATE_SEND pMsg;

	pMsg.header.set(0xD1,0x11,sizeof(pMsg));

	pMsg.state = state;

	pMsg.DetailState = DetailState;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CRaklionUtil::NotifyRaklionChangeState(int state,int DetailState) // OK
{
	PMSG_RAKLION_STATE_CHANGE_SEND pMsg;

	pMsg.header.set(0xD1,0x12,sizeof(pMsg));

	pMsg.state = state;

	pMsg.DetailState = DetailState;

	this->SendDataRaklionMapUser((BYTE*)&pMsg,pMsg.header.size);
}

void CRaklionUtil::NotifyRaklionResult(int result) // OK
{
	PMSG_RAKLION_SUCCESS_SEND pMsg;

	pMsg.header.set(0xD1,0x13,sizeof(pMsg));

	pMsg.result = result;

	this->SendDataRaklionBossMapUser((BYTE*)&pMsg,pMsg.header.size);
}

void CRaklionUtil::NotifyRaklionAreaAttack(int aIndex,int bIndex,int type) // OK
{
	PMSG_RAKLION_AREA_ATTACK_SEND pMsg;

	pMsg.header.set(0xD1,0x14,sizeof(pMsg));

	pMsg.MonsterClass[0] = SET_NUMBERHB(gObj[aIndex].Class);

	pMsg.MonsterClass[1] = SET_NUMBERLB(gObj[aIndex].Class);

	pMsg.type = type;

	DataSend(bIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CRaklionUtil::NotifyRaklionObjectCount(int MonsterCount,int UserCount) // OK
{
	PMSG_RAKLION_OBJECT_COUNT_SEND pMsg;

	pMsg.header.set(0xD1,0x15,sizeof(pMsg));

	pMsg.MonsterCount = MonsterCount;

	pMsg.UserCount = UserCount;

	this->SendDataRaklionBossMapUser((BYTE*)&pMsg,pMsg.header.size);
}

void CRaklionUtil::SendDataRaklionMapUser(BYTE* lpMsg,int size) // OK
{
	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && (gObj[n].Map == MAP_RAKLION1 || gObj[n].Map == MAP_RAKLION2))
		{
			DataSend(n,lpMsg,size);
		}
	}
}

void CRaklionUtil::SendDataRaklionBossMapUser(BYTE* lpMsg,int size) // OK
{
	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && gObj[n].Map == MAP_RAKLION2)
		{
			DataSend(n,lpMsg,size);
		}
	}
}

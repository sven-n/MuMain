// KanturuUtil.cpp: implementation of the CKanturuUtil class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KanturuUtil.h"
#include "Map.h"
#include "Notice.h"
#include "Util.h"

CKanturuUtil gKanturuUtil;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKanturuUtil::CKanturuUtil() // OK
{

}

CKanturuUtil::~CKanturuUtil() // OK
{

}

void CKanturuUtil::NotifyKanturuEnter(int aIndex,int result) // OK
{
	PMSG_KANTURU_ENTER_SEND pMsg;

	pMsg.header.set(0xD1,0x01,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CKanturuUtil::NotifyKanturuState(int aIndex,int state,int DetailState) // OK
{
	PMSG_KANTURU_STATE_SEND pMsg;

	pMsg.header.set(0xD1,0x02,sizeof(pMsg));

	pMsg.state = state;

	pMsg.DetailState = DetailState;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CKanturuUtil::NotifyKanturuChangeState(int state,int DetailState) // OK
{
	PMSG_KANTURU_STATE_CHANGE_SEND pMsg;

	pMsg.header.set(0xD1,0x03,sizeof(pMsg));

	pMsg.state = state;

	pMsg.DetailState = DetailState;

	this->SendDataKanturuBossMapUser((BYTE*)&pMsg,pMsg.header.size);
}

void CKanturuUtil::NotifyKanturuResult(int result) // OK
{
	PMSG_KANTURU_SUCCESS_SEND pMsg;

	pMsg.header.set(0xD1,0x04,sizeof(pMsg));

	pMsg.result = result;

	this->SendDataKanturuBossMapUser((BYTE*)&pMsg,pMsg.header.size);
}

void CKanturuUtil::NotifyKanturuBattleTime(int time) // OK
{
	PMSG_KANTURU_BATTLE_TIME_SEND pMsg;

	pMsg.header.set(0xD1,0x05,sizeof(pMsg));

	pMsg.time = time;

	this->SendDataKanturuBossMapUser((BYTE*)&pMsg,pMsg.header.size);
}

void CKanturuUtil::NotifyKanturuAreaAttack(int aIndex,int bIndex,int type) // OK
{
	PMSG_KANTURU_AREA_ATTACK_SEND pMsg;

	pMsg.header.set(0xD1,0x06,sizeof(pMsg));

	pMsg.MonsterClass[0] = SET_NUMBERHB(gObj[aIndex].Class);

	pMsg.MonsterClass[1] = SET_NUMBERLB(gObj[aIndex].Class);

	pMsg.type = type;

	DataSend(bIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CKanturuUtil::NotifyKanturuObjectCount(int MonsterCount,int UserCount) // OK
{
	PMSG_KANTURU_OBJECT_COUNT_SEND pMsg;

	pMsg.header.set(0xD1,0x07,sizeof(pMsg));

	pMsg.MonsterCount = MonsterCount;

	pMsg.UserCount = UserCount;

	this->SendDataKanturuBossMapUser((BYTE*)&pMsg,pMsg.header.size);
}

void CKanturuUtil::SendMsgKanturuMapUser(char* message,...) // OK
{
	char buff[256];

	va_list arg;
	va_start(arg,message);
	vsprintf_s(buff,message,arg);
	va_end(arg);

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && (gObj[n].Map == MAP_KANTURU1 || gObj[n].Map == MAP_KANTURU2 || gObj[n].Map == MAP_KANTURU3))
		{
			gNotice.GCNoticeSend(n,0,0,0,0,0,0,buff);
		}
	}
}

void CKanturuUtil::SendMsgKanturuBossMapUser(char* message,...) // OK
{
	char buff[256];

	va_list arg;
	va_start(arg,message);
	vsprintf_s(buff,message,arg);
	va_end(arg);

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && gObj[n].Map == MAP_KANTURU3)
		{
			gNotice.GCNoticeSend(n,0,0,0,0,0,0,buff);
		}
	}
}

void CKanturuUtil::SendDataKanturuMapUser(BYTE* lpMsg,int size) // OK
{
	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && (gObj[n].Map == MAP_KANTURU1 || gObj[n].Map == MAP_KANTURU2 || gObj[n].Map == MAP_KANTURU3))
		{
			DataSend(n,lpMsg,size);
		}
	}
}

void CKanturuUtil::SendDataKanturuBossMapUser(BYTE* lpMsg,int size) // OK
{
	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && gObj[n].Map == MAP_KANTURU3)
		{
			DataSend(n,lpMsg,size);
		}
	}
}

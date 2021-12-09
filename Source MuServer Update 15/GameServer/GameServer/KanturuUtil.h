// KanturuUtil.h: interface for the CRaklionUtil class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_KANTURU_ENTER_INFO_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:00
	BYTE state;
	BYTE DetailState;
	BYTE result;
	BYTE count;
	DWORD time;
	#pragma pack()
};

struct PMSG_KANTURU_ENTER_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:01
	BYTE result;
	#pragma pack()
};

struct PMSG_KANTURU_STATE_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:02
	BYTE state;
	BYTE DetailState;
	#pragma pack()
};

struct PMSG_KANTURU_STATE_CHANGE_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:03
	BYTE state;
	BYTE DetailState;
	#pragma pack()
};

struct PMSG_KANTURU_SUCCESS_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:04
	BYTE result;
	#pragma pack()
};

struct PMSG_KANTURU_BATTLE_TIME_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:05
	DWORD time;
	#pragma pack()
};

struct PMSG_KANTURU_AREA_ATTACK_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:06
	BYTE MonsterClass[2];
	BYTE type;
	#pragma pack()
};

struct PMSG_KANTURU_OBJECT_COUNT_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:07
	BYTE MonsterCount;
	BYTE UserCount;
	#pragma pack()
};

//**********************************************//
//**********************************************//
//**********************************************//
class CKanturuUtil
{
public:
	CKanturuUtil();
	virtual ~CKanturuUtil();
	void NotifyKanturuEnter(int aIndex,int result);
	void NotifyKanturuState(int aIndex,int state,int DetailState);
	void NotifyKanturuChangeState(int state,int DetailState);
	void NotifyKanturuResult(int result);
	void NotifyKanturuBattleTime(int time);
	void NotifyKanturuAreaAttack(int aIndex,int bIndex,int type);
	void NotifyKanturuObjectCount(int MonsterCount,int UserCount);
	void SendMsgKanturuMapUser(char* message,...);
	void SendMsgKanturuBossMapUser(char* message,...);
	void SendDataKanturuMapUser(BYTE* lpMsg,int size);
	void SendDataKanturuBossMapUser(BYTE* lpMsg,int size);
};

extern CKanturuUtil gKanturuUtil;

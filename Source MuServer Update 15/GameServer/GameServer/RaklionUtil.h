// RaklionUtil.h: interface for the CRaklionUtil class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_RAKLION_STATE_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:11
	BYTE state;
	BYTE DetailState;
	#pragma pack()
};

struct PMSG_RAKLION_STATE_CHANGE_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:12
	BYTE state;
	BYTE DetailState;
	#pragma pack()
};

struct PMSG_RAKLION_SUCCESS_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:13
	BYTE result;
	#pragma pack()
};

struct PMSG_RAKLION_AREA_ATTACK_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:14
	BYTE MonsterClass[2];
	BYTE type;
	#pragma pack()
};

struct PMSG_RAKLION_OBJECT_COUNT_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D1:15
	BYTE MonsterCount;
	BYTE UserCount;
	#pragma pack()
};

//**********************************************//
//**********************************************//
//**********************************************//

class CRaklionUtil
{
public:
	CRaklionUtil();
	virtual ~CRaklionUtil();
	void NotifyRaklionState(int aIndex,int state,int DetailState);
	void NotifyRaklionChangeState(int state,int DetailState);
	void NotifyRaklionResult(int result);
	void NotifyRaklionAreaAttack(int aIndex,int bIndex,int type);
	void NotifyRaklionObjectCount(int MonsterCount,int UserCount);
	void SendDataRaklionMapUser(BYTE* lpMsg,int size);
	void SendDataRaklionBossMapUser(BYTE* lpMsg,int size);
};

extern CRaklionUtil gRaklionUtil;

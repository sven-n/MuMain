// JewelMix.h: interface for the CJewelMix class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

#define MAX_JEWEL_MIX_BUNDLE 10

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_JEWEL_MIX_RECV
{
	PSBMSG_HEAD header; // C1:BC:00
	BYTE type;
	BYTE level;
};

struct PMSG_JEWEL_UNMIX_RECV
{
	PSBMSG_HEAD header; // C1:BC:01
	BYTE type;
	BYTE level;
	BYTE slot;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_JEWEL_MIX_RESULT_SEND
{
	PSBMSG_HEAD header; // C1:BC:[00:01]
	BYTE result;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CJewelMix
{
public:
	CJewelMix();
	virtual ~CJewelMix();
	int GetJewelSimpleIndex(int type);
	int GetJewelBundleIndex(int type);
	void CGJewelMixRecv(PMSG_JEWEL_MIX_RECV* lpMsg,int aIndex);
	void CGJewelUnMixRecv(PMSG_JEWEL_UNMIX_RECV* lpMsg,int aIndex);
	void GCJewelMixResultSend(int aIndex,BYTE result);
	void GCJewelUnMixResultSend(int aIndex,BYTE result);
	bool CommandJewelMix(LPOBJ lpObj,char* arg);
	bool CommandJewelUnMix(LPOBJ lpObj,char* arg);
};

extern CJewelMix gJewelMix;

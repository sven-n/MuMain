// Fruit.h: interface for the CFruit class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_FRUIT_RESULT_SEND
{
	PBMSG_HEAD header; // C1:2C
	BYTE result;
	WORD value;
	BYTE type;
	#if(GAMESERVER_EXTRA==1)
	DWORD ViewValue;
	DWORD ViewPoint;
	DWORD ViewStrength;
	DWORD ViewDexterity;
	DWORD ViewVitality;
	DWORD ViewEnergy;
	DWORD ViewLeadership;
	#endif
};

//**********************************************//
//**********************************************//
//**********************************************//

class CFruit
{
public:
	CFruit();
	virtual ~CFruit();
	void Init();
	int GetMaxFruitPoint(LPOBJ lpObj);
	bool UseFruitAddPoint(LPOBJ lpObj,int type);
	bool UseFruitSubPoint(LPOBJ lpObj,int type);
	bool UseFruitResetPoint(LPOBJ lpObj,int type,int amount);
	void GCFruitResultSend(LPOBJ lpObj,int result,int value,int type);
private:
	int m_NRMaxFruitPointTable[MAX_CHARACTER_LEVEL];
	int m_MGMaxFruitPointTable[MAX_CHARACTER_LEVEL];
	int m_DLMaxFruitPointTable[MAX_CHARACTER_LEVEL];
};

extern CFruit gFruit;

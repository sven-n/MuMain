// CustomStore.h: interface for the CCustomStore class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "PersonalShop.h"
#include "User.h"



class CCustomStore
{
public:
	CCustomStore();
	virtual ~CCustomStore();
	void ReadCustomStoreInfo(char* section,char* path);
	bool CommandCustomStore(LPOBJ lpObj,char* arg);
	bool CommandCustomStoreOffline(LPOBJ lpObj,char* arg);
	bool OpenCustomStore(LPOBJ lpObj,int type);
	bool OnPShopOpen(LPOBJ lpObj);
	void OnPShopClose(LPOBJ lpObj);
	void OnPShopSecondProc(LPOBJ lpObj);
	void OnPShopAlreadyConnected(LPOBJ lpObj);
	void OnPShopItemList(LPOBJ lpObj,LPOBJ lpTarget);
	bool OnPShopBuyItemRecv(PMSG_PSHOP_BUY_ITEM_RECV* lpMsg,int aIndex);
	static void OnPShopBuyItemCallbackRecv(LPOBJ lpObj,LPOBJ lpTarget,DWORD slot,DWORD WCoinC,DWORD WCoinP,DWORD GoblinPoint);
	void CGOffTradeRecv(PMSG_OFFTRADE_RECV* lpMsg, int aIndex);
	void GCOffTradeSend(int aIndex,int bIndex);
	void GCOffActiveSend(int aIndex, int active);
public:
	//int m_CustomStoreSwitch;
	//int m_CustomStoreMapZone;
	//int m_CustomStoreEnable[4];
	//int m_CustomStoreRequireLevel[4];
	//int m_CustomStoreRequireReset[4];
	int m_CustomStoreTime[4];
	//char m_CustomStoreCommandSyntax[32];
	char m_CustomStoreCommandJoBSyntax[32];
	char m_CustomStoreCommandJoSSyntax[32];
	char m_CustomStoreCommandJoCSyntax[32];
	char m_CustomStoreCommandWCCSyntax[32];
	char m_CustomStoreCommandWCPSyntax[32];
	char m_CustomStoreCommandWCGSyntax[32];
	char m_CustomStoreJoBName[36];
	char m_CustomStoreJoSName[36];
	char m_CustomStoreJoCName[36];
	char m_CustomStoreWCCName[36];
	char m_CustomStoreWCPName[36];
	char m_CustomStoreWCGName[36];
	//int m_CustomStoreOfflineMapZone;
};

extern CCustomStore gCustomStore;

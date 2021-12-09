// CustomQuest.h: interface for the CCustomQuest class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "ObjectManager.h"

struct STARTITEM_ITEM_INFO
{
	int Class;
	int Slot;
	int ItemIndex;
	int ItemLevel;
	int ItemDurability;
	int ItemOption1;
	int ItemOption2;
	int ItemOption3;
	int ItemNewOption;
	int AncOption;
	int JOH;
	int OpEx;
	int Socket1;
	int Socket2;
	int Socket3;
	int Socket4;
	int Socket5;
	int Duration;
};

struct STARTITEM_BUFF_INFO
{
	int Class;
	int EffectID;
	int Power1;
	int Power2;
	int Time;
};

struct STARTITEM_COIN_INFO
{
	int Class;
	int Coin1;
	int Coin2;
	int Coin3;
};

struct SDHP_STARTITEM_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:42
	WORD index;
    char name[11];
};

class CCustomStartItem
{
public:
	CCustomStartItem();
	virtual ~CCustomStartItem();
	void Init();
	void Load(char* path);
	void SetStartItem(LPOBJ lpObj);
	void InsertItem(LPOBJ lpObj);
	void InsertBuff(LPOBJ lpObj);
	void InsertCoin(LPOBJ lpObj);
	void DGStartItemSaveSend(LPOBJ lpObj);
private:
	std::vector<STARTITEM_ITEM_INFO>			m_CustomStartItemItemInfo;
	std::vector<STARTITEM_BUFF_INFO>			m_CustomStartItemBuffInfo;
	std::vector<STARTITEM_COIN_INFO>			m_CustomStartItemCoinInfo;
};

extern CCustomStartItem gCustomStartItem;
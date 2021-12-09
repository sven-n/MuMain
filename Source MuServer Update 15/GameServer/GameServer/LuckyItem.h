// LuckyItem.h: interface for the CLuckyItem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_LUCKY_ITEM_RECV
{
	PSWMSG_HEAD header; // C2:22:00
	WORD index;
	char account[11];
	BYTE count;
};

struct SDHP_LUCKY_ITEM1
{
	BYTE slot;
	DWORD serial;
	DWORD DurabilitySmall;
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_LUCKY_ITEM_SEND
{
	PSWMSG_HEAD header; // C2:22:00
	WORD index;
	char account[11];
	BYTE count;
};

struct SDHP_LUCKY_ITEM2
{
	BYTE slot;
	DWORD serial;
};

struct SDHP_LUCKY_ITEM_SAVE_SEND
{
	PSWMSG_HEAD header; // C2:22:30
	WORD index;
	char account[11];
	BYTE count;
};

struct SDHP_LUCKY_ITEM_SAVE
{
	DWORD serial;
	DWORD DurabilitySmall;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct LUCKY_ITEM_INFO
{
	int Index;
	int Group;
	int Decay;
	int Option0;
	int Option1;
	int Option2;
	int Option3;
	int Option4;
	int Option5;
	int Option6;
};

class CLuckyItem
{
public:
	CLuckyItem();
	virtual ~CLuckyItem();
	void Load(char* path);
	bool IsLuckyItem(CItem* lpItem);
	bool GetLuckyItemIndex(LPOBJ lpObj,int section,int group,WORD* index);
	bool GetLuckyItemDecay(int index,int* decay);
	bool GetLuckyItemOption0(int index,BYTE* option);
	bool GetLuckyItemOption1(int index,BYTE* option);
	bool GetLuckyItemOption2(int index,BYTE* option);
	bool GetLuckyItemOption3(int index,BYTE* option);
	bool GetLuckyItemOption4(int index,BYTE* option);
	bool GetLuckyItemOption5(int index,BYTE* option);
	bool GetLuckyItemOption6(int index,BYTE* option);
	bool CharacterUseJewelOfExtension(LPOBJ lpObj,int SourceSlot,int TargetSlot);
	bool CharacterUseJewelOfElevation(LPOBJ lpObj,int SourceSlot,int TargetSlot);
	void DGLuckyItemRecv(SDHP_LUCKY_ITEM_RECV* lpMsg);
	void GDLuckyItemSend(int aIndex);
	void GDLuckyItemSaveSend(int aIndex);
private:
	std::map<int,LUCKY_ITEM_INFO> m_LuckyItemInfo;
};

extern CLuckyItem gLuckyItem;

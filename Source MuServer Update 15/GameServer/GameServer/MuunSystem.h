// MuunSystem.h: interface for the CMuunSystem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "ItemManager.h"
#include "Protocol.h"

#define MUUN_INVENTORY_SIZE 62
#define MUUN_INVENTORY_WEAR_SIZE 2

#define MUUN_INVENTORY_RANGE(x) (((x)<0)?0:((x)>=MUUN_INVENTORY_SIZE)?0:1)
#define MUUN_INVENTORY_WEAR_RANGE(x) (((x)<0)?0:((x)>=MUUN_INVENTORY_WEAR_SIZE)?0:1)
#define MUUN_INVENTORY_BASE_RANGE(x) (((x)<MUUN_INVENTORY_WEAR_SIZE)?0:((x)>=MUUN_INVENTORY_SIZE)?0:1)

enum eMuunOption
{
	MUUN_OPTION_LUKI = 0,
	MUUN_OPTION_TONY = 1,
	MUUN_OPTION_NYMPH = 2,
	MUUN_OPTION_SARI = 3,
	MUUN_OPTION_WILLIAM = 4,
	MUUN_OPTION_PAUL = 5,
	MUUN_OPTION_CHIRON = 6,
	MUUN_OPTION_WOOPAROO = 7,
	MUUN_OPTION_TIBETTON = 8,
	MUUN_OPTION_WITCH = 9,
	MUUN_OPTION_SKULL = 10,
	MUUN_OPTION_PUMPY = 11,
	MUUN_OPTION_SAVATH = 12,
	MUUN_OPTION_LYCAN = 13,
	MUUN_OPTION_TORBY = 14,
};

enum eMuunSpecialOption
{
	MUUN_SPECIAL_OPTION_MUL_VALUE = 0,
	MUUN_SPECIAL_OPTION_SUB_COOLDOWN = 1,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_MUUN_ITEM_GET_RECV
{
	PSBMSG_HEAD header; // C3:4E:00
	BYTE index[2];
};

struct PMSG_MUUN_ITEM_USE_RECV
{
	PSBMSG_HEAD header; // C1:4E:08
	BYTE SourceSlot;
	BYTE TargetSlot;
	BYTE type;
};

struct PMSG_MUUN_ITEM_SELL_RECV
{
	PSBMSG_HEAD header; // C3:4E:09
	BYTE slot;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_MUUN_ITEM_GET_SEND
{
	PSBMSG_HEAD header; // C3:4E:00
	BYTE result;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_MUUN_ITEM_LIST_SEND
{
	PSWMSG_HEAD header; // C4:4E:02
	BYTE count;
};

struct PMSG_MUUN_ITEM_LIST
{
	BYTE slot;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_MUUN_ITEM_DUR_SEND
{
	PSBMSG_HEAD header; // C1:4E:03
	BYTE slot;
	BYTE dur;
};

struct PMSG_MUUN_ITEM_DELETE_SEND
{
	PSBMSG_HEAD header; // C1:4E:04
	BYTE slot;
	BYTE flag;
};

struct PMSG_MUUN_ITEM_MODIFY_SEND
{
	PSBMSG_HEAD header; // C1:4E:05
	BYTE flag;
	BYTE slot;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_MUUN_ITEM_CHANGE_SEND
{
	PSBMSG_HEAD header; // C1:4E:06
	BYTE index[2];
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_MUUN_ITEM_STATUS_SEND
{
	PSBMSG_HEAD header; // C1:4E:07
	BYTE slot;
	BYTE status;
};

struct PMSG_MUUN_ITEM_USE_SEND
{
	PSBMSG_HEAD header; // C1:4E:08
	BYTE result;
	BYTE type;
};

struct PMSG_MUUN_ITEM_SELL_SEND
{
	PSBMSG_HEAD header; // C1:4E:09
	BYTE result;
	DWORD money;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_MUUN_INVENTORY_RECV
{
	PSWMSG_HEAD header; // C2:27:00
	WORD index;
	char account[11];
	char name[11];
	BYTE MuunInventory[MUUN_INVENTORY_SIZE][16];
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_MUUN_INVENTORY_SEND
{
	PSBMSG_HEAD header; // C1:27:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_MUUN_INVENTORY_SAVE_SEND
{
	PSWMSG_HEAD header; // C2:27:30
	WORD index;
	char account[11];
	char name[11];
	BYTE MuunInventory[MUUN_INVENTORY_SIZE][16];
};

//**********************************************//
//**********************************************//
//**********************************************//

struct MUUN_SYSTEM_INFO
{
	int Index;
	int Type;
	int Rank;
	int OptionIndex;
	int EvolutionItemIndex;
};

struct MUUN_SYSTEM_OPTION_INFO
{
	int Index;
	int OptionValue[5];
	int MaxOptionValue;
	int SpecialOptionIndex;
	int SpecialOptionValue;
	int MapZone;
	int PlayTime;
	int DayOfWeek;
	int MinHour;
	int MaxHour;
	int MinLevel;
	int MaxLevel;
	int MinMasterLevel;
	int MaxMasterLevel;
};

class CMuunSystem
{
public:
	CMuunSystem();
	virtual ~CMuunSystem();
	void Load(char* path);
	void MainProc();
	bool IsMuunItem(int ItemIndex);
	bool IsMuunItem(CItem* lpItem);
	bool IsMuunUtil(int ItemIndex);
	bool IsMuunUtil(CItem* lpItem);
	bool GetMuunSystemInfo(int index,MUUN_SYSTEM_INFO* lpInfo);
	bool GetMuunSystemOptionInfo(int index,MUUN_SYSTEM_OPTION_INFO* lpInfo);
	void MuunInventoryItemSet(int aIndex,int slot,BYTE type);
	BYTE MuunInventoryRectCheck(int aIndex,int x,int y,int width,int height);
	BYTE MuunInventoryInsertItem(int aIndex,CItem item);
	BYTE MuunInventoryAddItem(int aIndex,CItem item,int slot);
	void MuunInventoryDelItem(int aIndex,int slot);
	bool CheckSpecialOption(LPOBJ lpObj,MUUN_SYSTEM_OPTION_INFO* lpInfo);
	bool CheckItemMoveToMuunInventory(LPOBJ lpObj,CItem* lpItem,int slot);
	void GetOptionValue(LPOBJ lpObj,CItem* lpItem,MUUN_SYSTEM_OPTION_INFO* lpInfo,int* value);
	void GetMuunItemSocketOption(int ItemIndex,BYTE* ItemLevel,BYTE* SocketOption,BYTE* SocketOptionBonus);
	void GetMuunUtilSocketOption(int ItemIndex,BYTE* ItemLevel,BYTE* SocketOption,BYTE* SocketOptionBonus);
	bool CharacterUseMuunLevelUp(LPOBJ lpObj,int SourceSlot,int TargetSlot);
	bool CharacterUseEvolutionStone(LPOBJ lpObj,int SourceSlot,int TargetSlot);
	bool CharacterUseJewelOfLife(LPOBJ lpObj,int SourceSlot,int TargetSlot);
	void MuunSprite(LPOBJ lpObj,int damage);
	void CalcMuunOption(LPOBJ lpObj,bool flag);
	void InsertOption(LPOBJ lpObj,int index,int value,bool flag);
	void CGMuunItemGetRecv(PMSG_MUUN_ITEM_GET_RECV* lpMsg,int aIndex);
	void CGMuunItemUseRecv(PMSG_MUUN_ITEM_USE_RECV* lpMsg,int aIndex);
	void CGMuunItemSellRecv(PMSG_MUUN_ITEM_SELL_RECV* lpMsg,int aIndex);
	void GCMuunItemListSend(int aIndex);
	void GCMuunItemDurSend(int aIndex,BYTE slot,BYTE dur);
	void GCMuunItemDeleteSend(int aIndex,BYTE slot,BYTE flag);
	void GCMuunItemModifySend(int aIndex,BYTE slot);
	void GCMuunItemChangeSend(int aIndex,BYTE slot);
	void GCMuunItemStatusSend(int aIndex,BYTE slot,BYTE status);
	void DGMuunInventoryRecv(SDHP_MUUN_INVENTORY_RECV* lpMsg);
	void GDMuunInventorySend(int aIndex);
	void GDMuunInventorySaveSend(int aIndex);
private:
	std::map<int,MUUN_SYSTEM_INFO> m_MuunSystemInfo;
	std::map<int,MUUN_SYSTEM_OPTION_INFO> m_MuunSystemOptionInfo;
};

extern CMuunSystem gMuunSystem;

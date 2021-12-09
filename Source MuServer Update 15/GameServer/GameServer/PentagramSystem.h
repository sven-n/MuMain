// PentagramSystem.h: interface for the CPentagramSystem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Item.h"
#include "User.h"
#include "Protocol.h"

#define MAX_PENTAGRAM_JEWEL_INFO 250
#define MAX_PENTAGRAM_JEWEL_RANK 5
#define MAX_PENTAGRAM_JEWEL_LEVEL 11
#define MAX_PENTAGRAM_ELEMENTAL_ATTRIBUTE 5

enum ePentagramJewelType
{
	PENTAGRAM_JEWEL_TYPE_INVENTORY = 0,
	PENTAGRAM_JEWEL_TYPE_WAREHOUSE = 1,
	PENTAGRAM_JEWEL_TYPE_TRADE = 2,
	PENTAGRAM_JEWEL_TYPE_PSHOP = 3,
};

enum ePentagramElementalAttribute
{
	ELEMENTAL_ATTRIBUTE_NONE = 0,
	ELEMENTAL_ATTRIBUTE_FIRE = 1,
	ELEMENTAL_ATTRIBUTE_WATER = 2,
	ELEMENTAL_ATTRIBUTE_EARTH = 3,
	ELEMENTAL_ATTRIBUTE_WIND = 4,
	ELEMENTAL_ATTRIBUTE_DARK = 5,
};

enum ePentagramOption
{
	PENTAGRAM_OPTION_MUL_PENTAGRAM_DAMAGE = 1,
	PENTAGRAM_OPTION_MUL_PENTAGRAM_DEFENSE = 2,
	PENTAGRAM_OPTION_ADD_ELEMENTAL_CRITICAL_DAMAGE_RATE = 3,
	PENTAGRAM_OPTION_ADD_ELEMENTAL_DEFENSE_TRANSFER_RATE = 4,
	PENTAGRAM_OPTION_ADD_ELEMENTAL_ATTACK_TRANSFER_RATE = 5,
	PENTAGRAM_OPTION_MUL_ELEMENTAL_DEFENSE_SUCCESS_RATE = 6,
	PENTAGRAM_OPTION_MUL_ELEMENTAL_ATTACK_SUCCESS_RATE = 7,
};

enum ePentagramJewelOption
{
	PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_DAMAGE = 0,
	PENTAGRAM_JEWEL_OF_ANGER_OPTION_MUL_ELEMENTAL_DAMAGE_VS_FIRE = 10,
	PENTAGRAM_JEWEL_OF_ANGER_OPTION_MUL_ELEMENTAL_DAMAGE_VS_WATER = 11,
	PENTAGRAM_JEWEL_OF_ANGER_OPTION_MUL_ELEMENTAL_DAMAGE_VS_EARTH = 12,
	PENTAGRAM_JEWEL_OF_ANGER_OPTION_MUL_ELEMENTAL_DAMAGE_VS_WIND = 13,
	PENTAGRAM_JEWEL_OF_ANGER_OPTION_MUL_ELEMENTAL_DAMAGE_VS_DARK = 14,
	PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_DAMAGE_PVP = 20,
	PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_DAMAGE_PVM = 21,
	PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_DAMAGE_RANGE = 30,
	PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_DAMAGE_MELEE = 31,
	PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_CRITICAL_DAMAGE_RATE_PVP = 40,
	PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_CRITICAL_DAMAGE_RATE_PVM = 41,
	PENTAGRAM_JEWEL_OF_BLESSING_OPTION_ADD_ELEMENTAL_DEFENSE = 50,
	PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DEFENSE_VS_FIRE = 60,
	PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DEFENSE_VS_WATER = 61,
	PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DEFENSE_VS_EARTH = 62,
	PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DEFENSE_VS_WIND = 63,
	PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DEFENSE_VS_DARK = 64,
	PENTAGRAM_JEWEL_OF_BLESSING_OPTION_ADD_ELEMENTAL_DEFENSE_PVP = 70,
	PENTAGRAM_JEWEL_OF_BLESSING_OPTION_ADD_ELEMENTAL_DEFENSE_PVM = 71,
	PENTAGRAM_JEWEL_OF_BLESSING_OPTION_ADD_ELEMENTAL_DEFENSE_RANGE = 80,
	PENTAGRAM_JEWEL_OF_BLESSING_OPTION_ADD_ELEMENTAL_DEFENSE_MELEE = 81,
	PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DAMAGE_PVP = 90,
	PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DAMAGE_PVM = 91,
	PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_ATTACK_SUCCESS_RATE = 100,
	PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_VS_FIRE = 110,
	PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_VS_WATER = 111,
	PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_VS_EARTH = 112,
	PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_VS_WIND = 113,
	PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_VS_DARK = 114,
	PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_ADD_ELEMENTAL_DAMAGE_PVP = 120,
	PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_ADD_ELEMENTAL_DAMAGE_PVM = 121,
	PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_ADD_ELEMENTAL_DAMAGE_RANGE = 130,
	PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_ADD_ELEMENTAL_DAMAGE_MELEE = 131,
	PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_PVP = 140,
	PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_PVM = 141,
	PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_MUL_ELEMENTAL_DEFENSE_SUCCESS_RATE = 150,
	PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_MUL_ELEMENTAL_DEFENSE_VS_FIRE = 160,
	PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_MUL_ELEMENTAL_DEFENSE_VS_WATER = 161,
	PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_MUL_ELEMENTAL_DEFENSE_VS_EARTH = 162,
	PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_MUL_ELEMENTAL_DEFENSE_VS_WIND = 163,
	PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_MUL_ELEMENTAL_DEFENSE_VS_DARK = 164,
	PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_ADD_ELEMENTAL_DEFENSE_PVP = 170,
	PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_ADD_ELEMENTAL_DEFENSE_PVM = 171,
	PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_ADD_ELEMENTAL_DEFENSE_RANGE = 180,
	PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_ADD_ELEMENTAL_DEFENSE_MELEE = 181,
	PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_ADD_ELEMENTAL_DAMAGE_REDUCTION_PVP = 190,
	PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_ADD_ELEMENTAL_DAMAGE_REDUCTION_PVM = 191,
	PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_ELEMENTAL_SLOW_RATE = 200,
	PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_STRENGTH = 210,
	PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_DEXTERITY = 211,
	PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_ENERGY = 212,
	PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_VITALITY = 213,
	PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_MAX_HP = 220,
	PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_MAX_MP = 221,
	PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_MAX_BP = 222,
	PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_MAX_SD = 223,
	PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_ELEMENTAL_EXCELLENT_DAMAGE_RATE_PVP = 230,
	PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_ELEMENTAL_EXCELLENT_DAMAGE_RATE_PVM = 231,
	PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_ELEMENTAL_DEBUFF_RATE = 240,
};

enum ePentagramRefineChaosMixNumber
{
	CHAOS_MIX_PENTAGRAM_MITHRIL = 1,
	CHAOS_MIX_PENTAGRAM_ELIXIR = 2,
	CHAOS_MIX_PENTAGRAM_JEWEL = 3,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE1 = 4,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE2 = 5,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE3 = 6,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE4 = 7,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE5 = 8,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE6 = 9,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE7 = 10,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE8 = 11,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE9 = 12,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE10 = 13,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE11 = 14,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE12 = 15,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE13 = 16,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE14 = 17,
	CHAOS_MIX_PENTAGRAM_DECOMPOSITE15 = 18,
};

enum ePentagramUpgradeChaosMixNumber
{
	CHAOS_MIX_PENTAGRAM_JEWEL_UPGRADE_LEVEL = 0,
	CHAOS_MIX_PENTAGRAM_JEWEL_UPGRADE_RANK = 1,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_PENTAGRAM_JEWEL_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:EC:00
	DWORD TargetSlot;
	DWORD SourceSlot;
};

struct PMSG_PENTAGRAM_JEWEL_REMOVE_RECV
{
	PSBMSG_HEAD header; // C1:EC:01
	DWORD SourceSlot;
	BYTE SocketSlot;
};

struct PMSG_PENTAGRAM_JEWEL_REFINE_RECV
{
	PSBMSG_HEAD header; // C1:EC:02
	BYTE type;
};

struct PMSG_PENTAGRAM_JEWEL_UPGRADE_RECV
{
	PSBMSG_HEAD header; // C1:EC:03
	BYTE type;
	BYTE info;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_PENTAGRAM_JEWEL_INSERT_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:EC:00
	BYTE result;
	BYTE Type;
	BYTE Index;
	BYTE Attribute;
	BYTE ItemSection;
	WORD ItemType;
	BYTE ItemLevel;
	BYTE OptionIndexRank1;
	BYTE OptionLevelRank1;
	BYTE OptionIndexRank2;
	BYTE OptionLevelRank2;
	BYTE OptionIndexRank3;
	BYTE OptionLevelRank3;
	BYTE OptionIndexRank4;
	BYTE OptionLevelRank4;
	BYTE OptionIndexRank5;
	BYTE OptionLevelRank5;
	#pragma pack()
};

struct PMSG_PENTAGRAM_JEWEL_REMOVE_SEND
{
	PSBMSG_HEAD header; // C1:EC:00
	BYTE result;
	BYTE Type;
	BYTE Index;
};

struct PMSG_PENTAGRAM_JEWEL_RESULT_SEND
{
	PSBMSG_HEAD header; // C1:EC:04
	BYTE result;
};

struct PMSG_PENTAGRAM_JEWEL_INFO_SEND
{
	PSWMSG_HEAD header; // C1:EE:01
	BYTE result;
	BYTE count;
	BYTE type;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_PENTAGRAM_JEWEL_INFO_RECV
{
	PSWMSG_HEAD header; // C2:23:00
	WORD index;
	char account[11];
	char name[11];
	BYTE type;
	BYTE count;
};

struct SDHP_PENTAGRAM_JEWEL_INFO
{
	BYTE Type;
	BYTE Index;
	BYTE Attribute;
	BYTE ItemSection;
	WORD ItemType;
	BYTE ItemLevel;
	BYTE OptionIndexRank1;
	BYTE OptionLevelRank1;
	BYTE OptionIndexRank2;
	BYTE OptionLevelRank2;
	BYTE OptionIndexRank3;
	BYTE OptionLevelRank3;
	BYTE OptionIndexRank4;
	BYTE OptionLevelRank4;
	BYTE OptionIndexRank5;
	BYTE OptionLevelRank5;
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_PENTAGRAM_JEWEL_INFO_SEND
{
	PSBMSG_HEAD header; // C1:23:00
	WORD index;
	char account[11];
	char name[11];
	BYTE type;
};

struct SDHP_PENTAGRAM_JEWEL_INFO_SAVE_SEND
{
	PSWMSG_HEAD header; // C2:23:30
	WORD index;
	char account[11];
	char name[11];
	BYTE count;
};

struct SDHP_PENTAGRAM_JEWEL_INFO_SAVE
{
	BYTE Type;
	BYTE Index;
	BYTE Attribute;
	BYTE ItemSection;
	WORD ItemType;
	BYTE ItemLevel;
	BYTE OptionIndexRank1;
	BYTE OptionLevelRank1;
	BYTE OptionIndexRank2;
	BYTE OptionLevelRank2;
	BYTE OptionIndexRank3;
	BYTE OptionLevelRank3;
	BYTE OptionIndexRank4;
	BYTE OptionLevelRank4;
	BYTE OptionIndexRank5;
	BYTE OptionLevelRank5;
};

struct SDHP_PENTAGRAM_JEWEL_INSERT_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:23:31
	WORD index;
	char account[11];
	char name[11];
	BYTE Type;
	BYTE Index;
	BYTE Attribute;
	BYTE ItemSection;
	WORD ItemType;
	BYTE ItemLevel;
	BYTE OptionIndexRank1;
	BYTE OptionLevelRank1;
	BYTE OptionIndexRank2;
	BYTE OptionLevelRank2;
	BYTE OptionIndexRank3;
	BYTE OptionLevelRank3;
	BYTE OptionIndexRank4;
	BYTE OptionLevelRank4;
	BYTE OptionIndexRank5;
	BYTE OptionLevelRank5;
};

struct SDHP_PENTAGRAM_JEWEL_DELETE_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:23:32
	WORD index;
	char account[11];
	char name[11];
	BYTE Type;
	BYTE Index;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct PENTAGRAM_JEWEL_INFO
{
	void Clear() // OK
	{
		this->Type = -1;
		this->Index = -1;
		this->Attribute = -1;
		this->ItemSection = -1;
		this->ItemType = 511;
		this->ItemLevel = 0;
		this->OptionIndexRank1 = -1;
		this->OptionLevelRank1 = -1;
		this->OptionIndexRank2 = -1;
		this->OptionLevelRank2 = -1;
		this->OptionIndexRank3 = -1;
		this->OptionLevelRank3 = -1;
		this->OptionIndexRank4 = -1;
		this->OptionLevelRank4 = -1;
		this->OptionIndexRank5 = -1;
		this->OptionLevelRank5 = -1;
	}

	#pragma pack(1)
	BYTE Type;
	BYTE Index;
	BYTE Attribute;
	BYTE ItemSection;
	WORD ItemType;
	BYTE ItemLevel;
	BYTE OptionIndexRank1;
	BYTE OptionLevelRank1;
	BYTE OptionIndexRank2;
	BYTE OptionLevelRank2;
	BYTE OptionIndexRank3;
	BYTE OptionLevelRank3;
	BYTE OptionIndexRank4;
	BYTE OptionLevelRank4;
	BYTE OptionIndexRank5;
	BYTE OptionLevelRank5;
	#pragma pack()
};

struct PENTAGRAM_TYPE_INFO
{
	int Index;
	int OptionIndex1;
	int OptionIndex2;
};

struct PENTAGRAM_OPTION_INFO
{
	int Index;
	int Value;
	int Type[MAX_PENTAGRAM_JEWEL_RANK];
	int Rank[MAX_PENTAGRAM_JEWEL_RANK];
	int Level[MAX_PENTAGRAM_JEWEL_RANK];
};

struct PENTAGRAM_JEWEL_OPTION_INFO
{
	int Index;
	int ItemIndex;
	int RankNumber;
	int RankOption;
	int RankOptionRate;
	int RankOptionValue[MAX_PENTAGRAM_JEWEL_LEVEL];
};

struct PENTAGRAM_JEWEL_REMOVE_INFO
{
	int Index;
	int MixRate[MAX_PENTAGRAM_JEWEL_RANK];
};

struct PENTAGRAM_JEWEL_UPGRADE_RANK_INFO
{
	int Index;
	int MixMoney;
	int MixRate;
};

struct PENTAGRAM_JEWEL_UPGRADE_LEVEL_INFO
{
	int Index;
	int MixMoney;
	int MixRate[MAX_PENTAGRAM_JEWEL_RANK];
};

class CPentagramSystem
{
public:
	CPentagramSystem();
	virtual ~CPentagramSystem();
	void Load(char* path);
	void LoadJewel(char* path);
	void LoadMixRate(char* path);
	bool IsPentagramItem(int ItemIndex);
	bool IsPentagramItem(CItem* lpItem);
	bool IsPentagramJewel(int ItemIndex);
	bool IsPentagramJewel(CItem* lpItem);
	bool IsPentagramMithril(int ItemIndex);
	bool IsPentagramMithril(CItem* lpItem);
	PENTAGRAM_JEWEL_INFO* GetPentagramJewelInfo(LPOBJ lpObj,int index,int type);
	PENTAGRAM_JEWEL_INFO* AddPentagramJewelInfo(LPOBJ lpObj,CItem* lpItem,int type);
	PENTAGRAM_JEWEL_INFO* AddPentagramJewelInfo(LPOBJ lpObj,PENTAGRAM_JEWEL_INFO* lpInfo,int type);
	void DelPentagramJewelInfo(LPOBJ lpObj,int index,int type);
	void DelAllPentagramJewelInfo(LPOBJ lpObj,CItem* lpItem,int type);
	bool CheckExchangePentagramItem(LPOBJ lpObj);
	bool CheckExchangePentagramItem(LPOBJ lpObj,CItem* lpItem);
    bool CPentagramSystem::CheckPentagramSocket(LPOBJ lpObj,CItem* lpItem);
	bool CheckPentagramOption(LPOBJ lpObj,PENTAGRAM_OPTION_INFO* lpInfo);
	void MovePentagramToInventoryFromWarehouse(LPOBJ lpObj,CItem* lpItem);
	void MovePentagramToWarehouseFromInventory(LPOBJ lpObj,CItem* lpItem);
	void ExchangePentagramItem(LPOBJ lpObj,LPOBJ lpTarget);
	void ExchangePentagramItem(LPOBJ lpObj,LPOBJ lpTarget,CItem* lpItem);
	bool GetPentagramTypeInfo(int index,PENTAGRAM_TYPE_INFO* lpInfo);
	bool GetPentagramOptionInfo(int index,PENTAGRAM_OPTION_INFO* lpInfo);
	bool GetPentagramJewelOptionInfo(int ItemIndex,int RankNumber,int RankOption,PENTAGRAM_JEWEL_OPTION_INFO* lpInfo);
	bool GetPentagramJewelRemoveInfo(int index,PENTAGRAM_JEWEL_REMOVE_INFO* lpInfo);
	bool GetPentagramJewelUpgradeRankInfo(int index,PENTAGRAM_JEWEL_UPGRADE_RANK_INFO* lpInfo);
	bool GetPentagramJewelUpgradeLevelInfo(int index,PENTAGRAM_JEWEL_UPGRADE_LEVEL_INFO* lpInfo);
	bool GetPentagramRandomJewelOption(int ItemIndex,int RankNumber,BYTE* option);
	void GetPentagramRelationshipDamage(LPOBJ lpObj,LPOBJ lpTarget,int* damage);
	void GetPentagramRelationshipDefense(LPOBJ lpObj,LPOBJ lpTarget,int* defense);
	void CalcPentagramOption(LPOBJ lpObj,bool flag);
	void CalcPentagramJewelOption(LPOBJ lpObj,bool flag);
	void InsertOption(LPOBJ lpObj,int index,int value,bool flag);
	void InsertJewelOption(LPOBJ lpObj,int index,int value,bool flag);
	void CGPentagramJewelInsertRecv(PMSG_PENTAGRAM_JEWEL_INSERT_RECV* lpMsg,int aIndex);
	void CGPentagramJewelRemoveRecv(PMSG_PENTAGRAM_JEWEL_REMOVE_RECV* lpMsg,int aIndex);
	void CGPentagramJewelRefineRecv(PMSG_PENTAGRAM_JEWEL_REFINE_RECV* lpMsg,int aIndex);
	void CGPentagramJewelUpgradeRecv(PMSG_PENTAGRAM_JEWEL_UPGRADE_RECV* lpMsg,int aIndex);
	void GCPentagramJewelResultSend(int aIndex,int result);
	void GCPentagramJewelInfoSend(int aIndex,int type);
	void GCPentagramJewelTradeInfoSend(int aIndex,int bIndex,CItem* lpItem);
	void GCPentagramJewelPShopInfoSend(int aIndex,int bIndex);
	void DGPentagramJewelInfoRecv(SDHP_PENTAGRAM_JEWEL_INFO_RECV* lpMsg);
	void GDPentagramJewelInfoSend(int aIndex,int type);
	void GDPentagramJewelInfoSaveSend(int aIndex,int type);
	void GDPentagramJewelInsertSaveSend(int aIndex,PENTAGRAM_JEWEL_INFO* lpInfo);
	void GDPentagramJewelDeleteSaveSend(int aIndex,int type,int index);
private:
	std::map<int,PENTAGRAM_TYPE_INFO> m_PentagramTypeInfo;
	std::map<int,PENTAGRAM_OPTION_INFO> m_PentagramOptionInfo;
	std::map<int,PENTAGRAM_JEWEL_OPTION_INFO> m_PentagramJewelOptionInfo;
	std::map<int,PENTAGRAM_JEWEL_REMOVE_INFO> m_PentagramJewelRemoveInfo;
	std::map<int,PENTAGRAM_JEWEL_UPGRADE_RANK_INFO> m_PentagramJewelUpgradeRankInfo;
	std::map<int,PENTAGRAM_JEWEL_UPGRADE_LEVEL_INFO> m_PentagramJewelUpgradeLevelInfo;
};

static const int gPentagramRelationshipDamageTablePvP[6][6] = {{100,100,100,100,100,100},{100,130,116,123,136,143},{100,143,130,116,123,136},{100,136,143,130,116,123},{100,123,136,143,130,116},{100,116,123,136,143,130}};

static const int gPentagramRelationshipDamageTablePvM[6][6] = {{100,100,100,100,100,100},{100,100,80,90,110,120},{100,120,100,80,90,110},{100,110,120,100,80,90},{100,90,110,120,100,80},{100,80,90,110,120,100}};

static const int gPentagramRelationshipDefenseTablePvP[6][6] = {{100,100,100,100,100,100},{100,100,90,95,105,110},{100,110,100,90,95,105},{100,105,110,100,90,95},{100,95,105,110,100,90},{100,90,95,105,110,100}};

static const int gPentagramRelationshipDefenseTablePvM[6][6] = {{100,100,100,100,100,100},{100,100,90,95,105,110},{100,110,100,90,95,105},{100,105,110,100,90,95},{100,95,105,110,100,90},{100,90,95,105,110,100}};

extern CPentagramSystem gPentagramSystem;

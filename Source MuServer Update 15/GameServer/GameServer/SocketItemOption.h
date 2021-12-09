// SocketItemOption.h: interface for the CSocketItemOption class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Item.h"
#include "User.h"

#define MAX_SOCKET_ITEM_OPTION_TYPE 6
#define MAX_SOCKET_ITEM_OPTION 50
#define MAX_SOCKET_ITEM_OPTION_TABLE 5

enum eSocketItemOptionType
{
	SOCKET_ITEM_OPTION_TYPE_NONE = 0,
	SOCKET_ITEM_OPTION_TYPE_FIRE = 1,
	SOCKET_ITEM_OPTION_TYPE_WATER = 2,
	SOCKET_ITEM_OPTION_TYPE_ICE = 3,
	SOCKET_ITEM_OPTION_TYPE_WIND = 4,
	SOCKET_ITEM_OPTION_TYPE_LIGHTNING = 5,
	SOCKET_ITEM_OPTION_TYPE_EARTH = 6,
};

enum eSocketItemOption
{
	SOCKET_ITEM_OPTION_ADD_DAMAGE_BY_LEVEL = 0,
	SOCKET_ITEM_OPTION_ADD_SPEED = 1,
	SOCKET_ITEM_OPTION_ADD_MIN_DAMAGE = 2,
	SOCKET_ITEM_OPTION_ADD_MAX_DAMAGE = 3,
	SOCKET_ITEM_OPTION_ADD_DAMAGE = 4,
	SOCKET_ITEM_OPTION_SUB_BP_CONSUMPTION_RATE = 5,
	SOCKET_ITEM_OPTION_MUL_DEFENSE_SUCCESS_RATE = 10,
	SOCKET_ITEM_OPTION_ADD_DEFENSE = 11,
	SOCKET_ITEM_OPTION_ADD_SHIELD_DAMAGE_REDUCTION = 12,
	SOCKET_ITEM_OPTION_ADD_DAMAGE_REDUCTION = 13,
	SOCKET_ITEM_OPTION_ADD_DAMAGE_REFLECT = 14,
	SOCKET_ITEM_OPTION_ADD_HUNT_HP = 16,
	SOCKET_ITEM_OPTION_ADD_HUNT_MP = 17,
	SOCKET_ITEM_OPTION_ADD_SKILL_DAMAGE = 18,
	SOCKET_ITEM_OPTION_ADD_ATTACK_SUCCESS_RATE = 19,
	SOCKET_ITEM_OPTION_ADD_ITEM_DURABILITY_RATE = 20,
	SOCKET_ITEM_OPTION_ADD_HP_RECOVERY = 21,
	SOCKET_ITEM_OPTION_MUL_MAX_HP = 22,
	SOCKET_ITEM_OPTION_MUL_MAX_MP = 23,
	SOCKET_ITEM_OPTION_ADD_MP_RECOVERY = 24,
	SOCKET_ITEM_OPTION_ADD_MAX_BP = 25,
	SOCKET_ITEM_OPTION_ADD_BP_RECOVERY = 26,
	SOCKET_ITEM_OPTION_ADD_EXCELLENT_DAMAGE = 29,
	SOCKET_ITEM_OPTION_ADD_EXCELLENT_DAMAGE_RATE = 30,
	SOCKET_ITEM_OPTION_ADD_CRITICAL_DAMAGE = 31,
	SOCKET_ITEM_OPTION_ADD_CRITICAL_DAMAGE_RATE = 32,
	SOCKET_ITEM_OPTION_ADD_STRENGTH = 34,
	SOCKET_ITEM_OPTION_ADD_DEXTERITY = 35,
	SOCKET_ITEM_OPTION_ADD_VITALITY = 36,
	SOCKET_ITEM_OPTION_ADD_ENERGY = 37,
	SOCKET_ITEM_OPTION_EMPTY = 254,
	SOCKET_ITEM_OPTION_NONE = 255,
};

enum eSocketItemBonusOption
{
	SOCKET_ITEM_WEAPON_BONUS_OPTION_ADD_DAMAGE = 0,
	SOCKET_ITEM_WEAPON_BONUS_OPTION_ADD_SKILL_DAMAGE = 1,
	SOCKET_ITEM_STAFF_BONUS_OPTION_ADD_DAMAGE = 2,
	SOCKET_ITEM_STAFF_BONUS_OPTION_ADD_SKILL_DAMAGE = 3,
	SOCKET_ITEM_ARMOR_BONUS_OPTION_ADD_DEFENSE = 4,
	SOCKET_ITEM_ARMOR_BONUS_OPTION_ADD_MAX_HP = 5,
	SOCKET_ITEM_BONUS_OPTION_NONE = 255,
};

enum eSocketItemPackageOption
{
	SOCKET_ITEM_PACKAGE_OPTION_ADD_DOUBLE_DAMAGE_RATE = 0,
	SOCKET_ITEM_PACKAGE_OPTION_ADD_IGNORE_DEFENSE_RATE = 1,
};

struct SOCKET_ITEM_OPTION_INFO
{
	int Index;
	int Type;
	int SubIndex;
	char Name[32];
	int Rate;
	int ValueTable[MAX_SOCKET_ITEM_OPTION_TABLE];
};

struct SOCKET_ITEM_BONUS_OPTION_INFO
{
	int Index;
	int MinItemSection;
	int MaxItemSection;
	char Name[32];
	int Value;
	int RequireOptionTypeTable[MAX_SOCKET_ITEM_OPTION_TABLE];
};

struct SOCKET_ITEM_PACKAGE_OPTION_INFO
{
	int Index;
	char Name[32];
	int Value;
	int RequireOptionTypeCountTable[MAX_SOCKET_ITEM_OPTION_TYPE];
};

class CSocketItemOption
{
public:
	CSocketItemOption();
	virtual ~CSocketItemOption();
	void Init();
	void Load(char* path);
	void SetInfo(SOCKET_ITEM_OPTION_INFO info);
	void SetBonusInfo(SOCKET_ITEM_BONUS_OPTION_INFO info);
	void SetPackageInfo(SOCKET_ITEM_PACKAGE_OPTION_INFO info);
	SOCKET_ITEM_OPTION_INFO* GetInfo(int index);
	SOCKET_ITEM_BONUS_OPTION_INFO* GetBonusInfo(int index);
	SOCKET_ITEM_PACKAGE_OPTION_INFO* GetPackageInfo(int index);
	bool IsSocketItem(CItem* lpItem);
	int GetSocketItemOptionCount(CItem* lpItem);
	int GetSocketItemExtraMoney(CItem* lpItem);
	int GetSocketItemSeedOption(int index,int type);
	int GetSocketItemSeedSphereOption(int index,int type);
	int GetSocketItemRandomOption(int type);
	BYTE GetSocketItemOption(CItem* lpItem,int type,int option,int level);
	BYTE GetSocketItemBonusOption(CItem* lpItem);
	bool CheckSocketItemOption(int index,CItem* lpItem);
	bool CheckSocketItemBonusOption(int index,CItem* lpItem);
	bool CheckSocketItemPackageOption(int index,int* ItemOptionTypeCount);
	void CalcSocketItemOption(LPOBJ lpObj,bool flag);
	void CalcSocketItemPackageOption(LPOBJ lpObj,int* ItemOptionTypeCount,bool flag);
	void InsertOption(LPOBJ lpObj,int index,int value,bool flag);
	void InsertBonusOption(LPOBJ lpObj,int index,int value,bool flag);
	void InsertPackageOption(LPOBJ lpObj,int index,int value,bool flag);
private:
	SOCKET_ITEM_OPTION_INFO m_SocketItemOptionInfo[MAX_SOCKET_ITEM_OPTION];
	SOCKET_ITEM_BONUS_OPTION_INFO m_SocketItemBonusOptionInfo[MAX_SOCKET_ITEM_OPTION];
	SOCKET_ITEM_PACKAGE_OPTION_INFO m_SocketItemPackageOptionInfo[MAX_SOCKET_ITEM_OPTION];
};

extern CSocketItemOption gSocketItemOption;

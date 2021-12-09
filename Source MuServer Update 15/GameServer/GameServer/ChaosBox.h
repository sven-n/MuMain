// ChaosBox.h: interface for the CChaosMix class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "ItemManager.h"
#include "Protocol.h"
#include "User.h"

#define MAX_TALISMAN_OF_LUCK 10

enum eChaosMixNumber
{
	CHAOS_MIX_CHAOS_ITEM = 1,
	CHAOS_MIX_DEVIL_SQUARE = 2,
	CHAOS_MIX_PLUS_ITEM_LEVEL1 = 3,
	CHAOS_MIX_PLUS_ITEM_LEVEL2 = 4,
	CHAOS_MIX_DINORANT = 5,
	CHAOS_MIX_FRUIT = 6,
	CHAOS_MIX_WING1 = 7,
	CHAOS_MIX_BLOOD_CASTLE = 8,
	CHAOS_MIX_WING2 = 11,
	CHAOS_MIX_PET1 = 13,
	CHAOS_MIX_PET2 = 14,
	CHAOS_MIX_SIEGE_POTION1 = 15,
	CHAOS_MIX_SIEGE_POTION2 = 16,
	CHAOS_MIX_LIFE_STONE = 17,
	CHAOS_MIX_SENIOR = 18,
	CHAOS_MIX_PLUS_ITEM_LEVEL3 = 22,
	CHAOS_MIX_PLUS_ITEM_LEVEL4 = 23,
	CHAOS_MIX_WING3 = 24,
	CHAOS_MIX_PIECE_OF_HORN = 25,
	CHAOS_MIX_BROKEN_HORN = 26,
	CHAOS_MIX_HORN_OF_FENRIR = 27,
	CHAOS_MIX_HORN_OF_FENRIR_UPGRADE = 28,
	CHAOS_MIX_SHIELD_POTION1 = 30,
	CHAOS_MIX_SHIELD_POTION2 = 31,
	CHAOS_MIX_SHIELD_POTION3 = 32,
	CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_PURITY = 33,
	CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_SMELT = 34,
	CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_RESTORE = 35,
	CHAOS_MIX_ITEM_380 = 36,
	CHAOS_MIX_ILLUSION_TEMPLE = 37,
	CHAOS_MIX_FEATHER_OF_CONDOR = 38,
	CHAOS_MIX_WING4 = 39,
	CHAOS_MIX_CHAOS_CARD = 40,
	CHAOS_MIX_CHERRY_BLOSSOM = 41,
	CHAOS_MIX_SOCKET_ITEM_CREATE_SEED = 42,
	CHAOS_MIX_SOCKET_ITEM_CREATE_SEED_SPHERE = 43,
	CHAOS_MIX_SOCKET_ITEM_MOUNT_SEED_SPHERE = 44,
	CHAOS_MIX_SOCKET_ITEM_UN_MOUNT_SEED_SPHERE = 45,
	CHAOS_MIX_IMPERIAL_GUARDIAN = 46,
	CHAOS_MIX_CHEST = 47,
	CHAOS_MIX_SUMMON_SCROLL = 48,
	CHAOS_MIX_PLUS_ITEM_LEVEL5 = 49,
	CHAOS_MIX_PLUS_ITEM_LEVEL6 = 50,
	CHAOS_MIX_LUCKY_ITEM_CREATE = 51,
	CHAOS_MIX_LUCKY_ITEM_REFINE = 52,
	CHAOS_MIX_MONSTER_WING = 56,
	CHAOS_MIX_SOCKET_WEAPON = 57,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_CHAOS_MIX_RECV
{
	PBMSG_HEAD header; // C1:86
	BYTE type;
	BYTE info;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_CHAOS_MIX_SEND
{
	PBMSG_HEAD header; // C1:86
	BYTE result;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

//**********************************************//
//**********************************************//
//**********************************************//

class CChaosBox
{
public:
	CChaosBox();
	virtual ~CChaosBox();
	void ChaosBoxInit(LPOBJ lpObj);
	void ChaosBoxItemDown(LPOBJ lpObj,int slot);
	void ChaosBoxItemKeep(LPOBJ lpObj,int slot);
	void ChaosBoxItemSave(LPOBJ lpObj);
	bool GetTalismanOfLuckRate(LPOBJ lpObj,int* rate);
	bool GetElementalTalismanOfLuckRate(LPOBJ lpObj,int* rate);
	void ChaosItemMix(LPOBJ lpObj);
	void DevilSquareMix(LPOBJ lpObj);
	void PlusItemLevelMix(LPOBJ lpObj,int type);
	void DinorantMix(LPOBJ lpObj);
	void FruitMix(LPOBJ lpObj);
	void Wing2Mix(LPOBJ lpObj,int type);
	void BloodCastleMix(LPOBJ lpObj);
	void Wing1Mix(LPOBJ lpObj);
	void PetMix(LPOBJ lpObj,int type);
	void SiegePotionMix(LPOBJ lpObj,int type);
	void LifeStoneMix(LPOBJ lpObj);
	void SeniorMix(LPOBJ lpObj);
	void PieceOfHornMix(LPOBJ lpObj);
	void BrokenHornMix(LPOBJ lpObj);
	void HornOfFenrirMix(LPOBJ lpObj);
	void HornOfFenrirUpgradeMix(LPOBJ lpObj);
	void ShieldPotionMix(LPOBJ lpObj,int type);
	void JewelOfHarmonyItemPurityMix(LPOBJ lpObj);
	void JewelOfHarmonyItemSmeltMix(LPOBJ lpObj);
	void JewelOfHarmonyItemRestoreMix(LPOBJ lpObj);
	void Item380Mix(LPOBJ lpObj);
	void IllusionTempleMix(LPOBJ lpObj);
	void FeatherOfCondorMix(LPOBJ lpObj);
	void Wing3Mix(LPOBJ lpObj);
	void ChaosCardMix(LPOBJ lpObj,int type);
	void CherryBlossomMix(LPOBJ lpObj,int type);
	void SocketItemCreateSeedMix(LPOBJ lpObj);
	void SocketItemCreateSeedSphereMix(LPOBJ lpObj);
	void SocketItemMountSeedSphereMix(LPOBJ lpObj,BYTE info);
	void SocketItemUnMountSeedSphereMix(LPOBJ lpObj,BYTE info);
	void ImperialGuardianMix(LPOBJ lpObj);
	void ChestMix(LPOBJ lpObj);
	void SummonScrollMix(LPOBJ lpObj,int type);
	void LuckyItemCreateMix(LPOBJ lpObj);
	void LuckyItemRefineMix(LPOBJ lpObj);
	void MonsterWingMix(LPOBJ lpObj);
	void SocketWeaponMix(LPOBJ lpObj);
	void PentagramMithrilMix(LPOBJ lpObj);
	void PentagramElixirMix(LPOBJ lpObj);
	void PentagramJewelMix(LPOBJ lpObj);
	void PentagramDecompositeMix(LPOBJ lpObj,int type);
	void PentagramJewelUpgradeLevelMix(LPOBJ lpObj,BYTE info);
	void PentagramJewelUpgradeRankMix(LPOBJ lpObj,BYTE info);
	void CGChaosMixRecv(PMSG_CHAOS_MIX_RECV* lpMsg,int aIndex);
	void CGChaosMixCloseRecv(int aIndex);
	void GCChaosBoxSend(LPOBJ lpObj,BYTE type);
	void GCChaosMixSend(int aIndex,BYTE result,CItem* lpItem);
	void CustomMix(LPOBJ lpObj,int type);
public:
	int m_SeniorMixLimitDay;
	int m_SeniorMixLimitMonth;
	int m_SeniorMixLimitYear;
};

extern CChaosBox gChaosBox;

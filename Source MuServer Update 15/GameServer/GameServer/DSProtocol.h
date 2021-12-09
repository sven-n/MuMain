#pragma once

#include "EffectManager.h"
#include "Item.h"
#include "PentagramSystem.h"
#include "Protocol.h"
#include "SkillManager.h"
#include "User.h"

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_DATA_SERVER_INFO_RECV
{
	PBMSG_HEAD header; // C1:00
	BYTE result;
	DWORD ItemCount;
};

struct SDHP_CHARACTER_LIST_RECV
{
	PWMSG_HEAD header; // C2:01
	WORD index;
	char account[11];
	BYTE MoveCnt;
	BYTE ExtClass;
	#if(GAMESERVER_UPDATE>=602)
	BYTE ExtWarehouse;
	#endif
	BYTE count;
};

struct SDHP_CHARACTER_LIST
{
	BYTE slot;
	char name[11];
	WORD level;
	BYTE Class;
	BYTE CtlCode;
	BYTE Inventory[60];
	BYTE GuildStatus;
	DWORD Reset;
	DWORD MasterReset;
};

struct SDHP_CHARACTER_CREATE_RECV
{
	PBMSG_HEAD header; // C1:02
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
	BYTE slot;
	BYTE Class;
	BYTE equipment[24];
	WORD level;
};

struct SDHP_CHARACTER_DELETE_RECV
{
	PBMSG_HEAD header; // C1:03
	WORD index;
	char account[11];
	BYTE result;
};

struct SDHP_CHARACTER_INFO_RECV
{
	PWMSG_HEAD header; // C2:04
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
	BYTE Class;
	WORD Level;
	DWORD LevelUpPoint;
	DWORD Experience;
	DWORD Money;
	DWORD Strength;
	DWORD Dexterity;
	DWORD Vitality;
	DWORD Energy;
	DWORD Leadership;
	DWORD Life;
	DWORD MaxLife;
	DWORD Mana;
	DWORD MaxMana;
	DWORD BP;
	DWORD MaxBP;
	DWORD Shield;
	DWORD MaxShield;
	BYTE Inventory[INVENTORY_SIZE][16];
	BYTE Skill[MAX_SKILL_LIST][3];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	DWORD PKCount;
	DWORD PKLevel;
	DWORD PKTime;
	BYTE CtlCode;
	BYTE Quest[50];
	WORD FruitAddPoint;
	WORD FruitSubPoint;
	BYTE Effect[MAX_EFFECT_LIST][13];
	#if(GAMESERVER_UPDATE>=602)
	BYTE ExtInventory;
	//BYTE ExtWarehouse;
	#endif
	DWORD Reset;
	DWORD MasterReset;
	#if(GAMESERVER_UPDATE>=801)
	DWORD UseGuildMatching;
	DWORD UseGuildMatchingJoin;
	#endif
	DWORD Kills;
	DWORD Deads;
	BYTE StartItem;
};

struct SDHP_CREATE_ITEM_RECV
{
	PBMSG_HEAD header; // C1:07
	WORD index;
	char account[11];
	BYTE X;
	BYTE Y;
	BYTE Map;
	DWORD Serial;
	WORD ItemIndex;
	BYTE Level;
	BYTE Dur;
	BYTE Option1;
	BYTE Option2;
	BYTE Option3;
	BYTE NewOption;
	WORD LootIndex;
	BYTE SetOption;
	BYTE JewelOfHarmonyOption;
	BYTE ItemOptionEx;
	BYTE SocketOption[MAX_SOCKET_OPTION];
	BYTE SocketOptionBonus;
	DWORD Duration;
};

struct SDHP_OPTION_DATA_RECV
{
	PBMSG_HEAD header; // C1:08
	WORD index;
	char account[11];
	char name[11];
	BYTE SkillKey[20];
	BYTE GameOption;
	BYTE QKey;
	BYTE WKey;
	BYTE EKey;
	BYTE ChatWindow;
	BYTE RKey;
	DWORD QWERLevel;
	#if(GAMESERVER_UPDATE>=701)
	BYTE ChangeSkin;
	#endif
};

struct SDHP_PET_ITEM_INFO_RECV
{
	PWMSG_HEAD header; // C2:09
	WORD index;
	char account[11];
	BYTE type;
	BYTE count;
};

struct SDHP_PET_ITEM_INFO1
{
	BYTE slot;
	DWORD serial;
	BYTE level;
	DWORD experience;
};

struct SDHP_CHARACTER_NAME_CHECK_RECV
{
	PBMSG_HEAD header; // C1:0A
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
};

struct SDHP_CHARACTER_NAME_CHANGE_RECV
{
	PBMSG_HEAD header; // C1:0B
	WORD index;
	char account[11];
	char OldName[11];
	char NewName[11];
	BYTE result;
};

struct SDHP_CRYWOLF_SYNC_RECV
{
	PBMSG_HEAD header; // C1:1E
	WORD MapServerGroup;
	DWORD CrywolfState;
	DWORD OccupationState;
};

struct SDHP_CRYWOLF_INFO_RECV
{
	PBMSG_HEAD header; // C1:1F
	WORD MapServerGroup;
	DWORD CrywolfState;
	DWORD OccupationState;
};

struct SDHP_GLOBAL_POST_RECV
{
	PBMSG_HEAD header; // C1:20
	WORD MapServerGroup;
	BYTE type;
	char name[11];
	char message[60];
};

struct SDHP_GLOBAL_NOTICE_RECV
{
	PBMSG_HEAD header; // C1:21
	WORD MapServerGroup;
	BYTE type;
	BYTE count;
	BYTE opacity;
	WORD delay;
	DWORD color;
	BYTE speed;
	char message[128];
};

struct SDHP_SNS_DATA_RECV
{
	PWMSG_HEAD header; // C1:24
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
	BYTE data[256];
};

struct SDHP_GLOBAL_WHISPER_RECV
{
	PBMSG_HEAD header; // C1:72
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
	char TargetName[11];
	char message[60];
};

struct SDHP_GLOBAL_WHISPER_ECHO_RECV
{
	PBMSG_HEAD header; // C1:73
	WORD index;
	char account[11];
	char name[11];
	char SourceName[11];
	char message[60];
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_MARRY_INFO_SAVE_SEND
{
    PBMSG_HEAD header; // C1:39
    WORD index;
    char name[11];
    char marryname[11];
    char mode[11];
    UINT countyou;
    UINT counthim;
    UINT marriedon;
};


struct SDHP_DATA_SERVER_INFO_SEND
{
	PBMSG_HEAD header; // C1:00
	BYTE type;
	WORD ServerPort;
	char ServerName[50];
	WORD ServerCode;
};

struct SDHP_CHARACTER_LIST_SEND
{
	PBMSG_HEAD header; // C1:01
	WORD index;
	char account[11];
};

struct SDHP_CHARACTER_CREATE_SEND
{
	PBMSG_HEAD header; // C1:02
	WORD index;
	char account[11];
	char name[11];
	BYTE Class;
};

struct SDHP_CHARACTER_DELETE_SEND
{
	PBMSG_HEAD header; // C1:03
	WORD index;
	char account[11];
	char name[11];
	BYTE guild;
	char GuildName[9];
};

struct SDHP_CHARACTER_INFO_SEND
{
	PBMSG_HEAD header; // C1:04
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_CREATE_ITEM_SEND
{
	PBMSG_HEAD header; // C1:07
	WORD index;
	char account[11];
	BYTE X;
	BYTE Y;
	BYTE Map;
	WORD ItemIndex;
	BYTE Level;
	BYTE Dur;
	BYTE Option1;
	BYTE Option2;
	BYTE Option3;
	BYTE NewOption;
	WORD LootIndex;
	BYTE SetOption;
	BYTE JewelOfHarmonyOption;
	BYTE ItemOptionEx;
	BYTE SocketOption[MAX_SOCKET_OPTION];
	BYTE SocketOptionBonus;
	DWORD Duration;
};

struct SDHP_OPTION_DATA_SEND
{
	PBMSG_HEAD header; // C1:08
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_PET_ITEM_INFO_SEND
{
	PWMSG_HEAD header; // C2:09
	WORD index;
	char account[11];
	BYTE type;
	BYTE count;
};

struct SDHP_PET_ITEM_INFO2
{
	BYTE slot;
	DWORD serial;
};

struct SDHP_CHARACTER_NAME_CHECK_SEND
{
	PBMSG_HEAD header; // C1:0A
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_CHARACTER_NAME_CHANGE_SEND
{
	PBMSG_HEAD header; // C1:0B
	WORD index;
	char account[11];
	char OldName[11];
	char NewName[11];
};

struct SDHP_CRYWOLF_SYNC_SEND
{
	PBMSG_HEAD header; // C1:1E
	WORD MapServerGroup;
	DWORD CrywolfState;
	DWORD OccupationState;
};

struct SDHP_CRYWOLF_INFO_SEND
{
	PBMSG_HEAD header; // C1:1F
	WORD MapServerGroup;
};

struct SDHP_GLOBAL_POST_SEND
{
	PBMSG_HEAD header; // C1:20
	WORD MapServerGroup;
	BYTE type;
	char name[11];
	char message[60];
};

struct SDHP_GLOBAL_NOTICE_SEND
{
	PBMSG_HEAD header; // C1:21
	WORD MapServerGroup;
	BYTE type;
	BYTE count;
	BYTE opacity;
	WORD delay;
	DWORD color;
	BYTE speed;
	char message[128];
};

struct SDHP_SNS_DATA_SEND
{
	PBMSG_HEAD header; // C1:24
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_CHARACTER_INFO_SAVE_SEND
{
	PWMSG_HEAD header; // C2:30
	WORD index;
	char account[11];
	char name[11];
	WORD Level;
	BYTE Class;
	DWORD LevelUpPoint;
	DWORD Experience;
	DWORD Money;
	DWORD Strength;
	DWORD Dexterity;
	DWORD Vitality;
	DWORD Energy;
	DWORD Leadership;
	DWORD Life;
	DWORD MaxLife;
	DWORD Mana;
	DWORD MaxMana;
	DWORD BP;
	DWORD MaxBP;
	DWORD Shield;
	DWORD MaxShield;
	BYTE Inventory[INVENTORY_SIZE][16];
	BYTE Skill[MAX_SKILL_LIST][3];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	DWORD PKCount;
	DWORD PKLevel;
	DWORD PKTime;
	BYTE Quest[50];
	WORD FruitAddPoint;
	WORD FruitSubPoint;
	BYTE Effect[MAX_EFFECT_LIST][13];
	#if(GAMESERVER_UPDATE>=602)
	BYTE ExtInventory;
	//BYTE ExtWarehouse;
	#endif
	DWORD Kills;
	DWORD Deads;
};

struct SDHP_INVENTORY_ITEM_SAVE_SEND
{
	PWMSG_HEAD header; // C2:31
	WORD index;
	char account[11];
	char name[11];
	BYTE Inventory[INVENTORY_SIZE][16];
};

struct SDHP_OPTION_DATA_SAVE_SEND
{
	PBMSG_HEAD header; // C1:33
	WORD index;
	char account[11];
	char name[11];
	BYTE SkillKey[20];
	BYTE GameOption;
	BYTE QKey;
	BYTE WKey;
	BYTE EKey;
	BYTE ChatWindow;
	BYTE RKey;
	DWORD QWERLevel;
	#if(GAMESERVER_UPDATE>=701)
	BYTE ChangeSkin;
	#endif
};

struct SDHP_PET_ITEM_INFO_SAVE_SEND
{
	PWMSG_HEAD header; // C2:34
	WORD index;
	char account[11];
	BYTE count;
};

struct SDHP_PET_ITEM_INFO_SAVE
{
	DWORD serial;
	BYTE level;
	DWORD experience;
};

struct SDHP_RESET_INFO_SAVE_SEND
{
	PBMSG_HEAD header; // C1:39
	WORD index;
	char account[11];
	char name[11];
	UINT Reset;
	UINT ResetDay;
	UINT ResetWek;
	UINT ResetMon;
};

struct SDHP_MASTER_RESET_INFO_SAVE_SEND
{
	PBMSG_HEAD header; // C1:3A
	WORD index;
	char account[11];
	char name[11];
	UINT Reset;
	UINT MasterReset;
	UINT MasterResetDay;
	UINT MasterResetWek;
	UINT MasterResetMon;
};

struct SDHP_RANKING_DUEL_SAVE_SEND
{
	PBMSG_HEAD header; // C1:3C
	WORD index;
	char account[11];
	char name[11];
	DWORD WinScore;
	DWORD LoseScore;
};

struct SDHP_RANKING_BLOOD_CASTLE_SAVE_SEND
{
	PBMSG_HEAD header; // C1:3D
	WORD index;
	char account[11];
	char name[11];
	DWORD score;
};

struct SDHP_RANKING_CHAOS_CASTLE_SAVE_SEND
{
	PBMSG_HEAD header; // C1:3E
	WORD index;
	char account[11];
	char name[11];
	DWORD score;
};

struct SDHP_RANKING_DEVIL_SQUARE_SAVE_SEND
{
	PBMSG_HEAD header; // C1:3F
	WORD index;
	char account[11];
	char name[11];
	DWORD score;
};

struct SDHP_RANKING_ILLUSION_TEMPLE_SAVE_SEND
{
	PBMSG_HEAD header; // C1:40
	WORD index;
	char account[11];
	char name[11];
	DWORD score;
};

struct SDHP_CREATION_CARD_SAVE_SEND
{
	PBMSG_HEAD header; // C1:42
	WORD index;
	char account[11];
	BYTE ExtClass;
};

struct SDHP_CRYWOLF_INFO_SAVE_SEND
{
	PBMSG_HEAD header; // C1:49
	WORD MapServerGroup;
	DWORD CrywolfState;
	DWORD OccupationState;
};

struct SDHP_SNS_DATA_SAVE_SEND
{
	PWMSG_HEAD header; // C1:4E
	WORD index;
	char account[11];
	char name[11];
	BYTE data[256];
};

struct SDHP_CUSTOM_MONSTER_REWARD_SAVE_SEND
{
	PBMSG_HEAD header; // C1:52
	WORD index;
	char account[11];
	char name[11];
	DWORD MonsterClass;
	DWORD MapNumber;
	DWORD RewardValue1;
	DWORD RewardValue2;
};

struct SDHP_RANKING_CUSTOM_ARENA_SAVE_SEND
{
	PBMSG_HEAD header; // C1:55
	WORD index;
	char account[11];
	char name[11];
	DWORD number;
	DWORD score;
	DWORD rank;
};

struct SDHP_CONNECT_CHARACTER_SEND
{
	PBMSG_HEAD header; // C1:70
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_DISCONNECT_CHARACTER_SEND
{
	PBMSG_HEAD header; // C1:71
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_GLOBAL_WHISPER_SEND
{
	PBMSG_HEAD header; // C1:72
	WORD index;
	char account[11];
	char name[11];
	char TargetName[11];
	char message[60];
};

struct SDHP_SETCOIN_SEND
{
    PBMSG_HEAD header; // C1:F3
    WORD index;
    char account[11];
	char name[11];
	DWORD value1;
	DWORD value2;
	DWORD value3;
};

struct SDHP_KILL_SYSTEM_SEND
{
    PBMSG_HEAD header; // C1:F3
    char killer[11];
	char dead[11];
};

struct SDHP_RANKING_TVT_EVENT_SAVE_SEND
{
	PBMSG_HEAD header; // C1:55
	WORD index;
	char account[11];
	char name[11];
	DWORD killcount;
	DWORD deathcount;
};


//**********************************************//
//**********************************************//
//**********************************************//

void DataServerProtocolCore(BYTE head,BYTE* lpMsg,int size);
void DGServerInfoRecv(SDHP_DATA_SERVER_INFO_RECV* lpMsg);
void DGCharacterListRecv(SDHP_CHARACTER_LIST_RECV* lpMsg);
void DGCharacterCreateRecv(SDHP_CHARACTER_CREATE_RECV* lpMsg);
void DGCharacterDeleteRecv(SDHP_CHARACTER_DELETE_RECV* lpMsg);
void DGCharacterInfoRecv(SDHP_CHARACTER_INFO_RECV* lpMsg);
void DGCreateItemRecv(SDHP_CREATE_ITEM_RECV* lpMsg);
void DGPetItemInfoRecv(SDHP_PET_ITEM_INFO_RECV* lpMsg);
void DGOptionDataRecv(SDHP_OPTION_DATA_RECV* lpMsg);
void DGCharacterNameCheckRecv(SDHP_CHARACTER_NAME_CHECK_RECV* lpMsg);
void DGCharacterNameChangeRecv(SDHP_CHARACTER_NAME_CHANGE_RECV* lpMsg);
void DGCrywolfSyncRecv(SDHP_CRYWOLF_SYNC_RECV* lpMsg);
void DGCrywolfInfoRecv(SDHP_CRYWOLF_INFO_RECV* lpMsg);
void DGGlobalPostRecv(SDHP_GLOBAL_POST_RECV* lpMsg);
void DGGlobalNoticeRecv(SDHP_GLOBAL_NOTICE_RECV* lpMsg);
void DGSNSDataRecv(SDHP_SNS_DATA_RECV* lpMsg);
void DGGlobalWhisperRecv(SDHP_GLOBAL_WHISPER_RECV* lpMsg);
void DGGlobalWhisperEchoRecv(SDHP_GLOBAL_WHISPER_ECHO_RECV* lpMsg);
void GDServerInfoSend();
void GDCharacterListSend(int aIndex);
void GDCharacterCreateSend(int aIndex,char* name,BYTE Class);
void GDCharacterDeleteSend(int aIndex,char* name);
void GDCharacterInfoSend(int aIndex,char* name);
void GDCreateItemSend(int aIndex,BYTE map,BYTE x,BYTE y,int index,BYTE level,BYTE dur,BYTE Option1,BYTE Option2,BYTE Option3,int LootIndex,BYTE NewOption,BYTE SetOption,BYTE JewelOfHarmonyOption,BYTE ItemOptionEx,BYTE SocketOption[MAX_SOCKET_OPTION],BYTE SocketOptionBonus,DWORD duration);
void GDOptionDataSend(int aIndex);
void GDPetItemInfoSend(int aIndex,int type);
void GDCharacterNameCheckSend(int aIndex,char* name);
void GDCharacterNameChangeSend(int aIndex,char* OldName,char* NewName);
void GDCrywolfSyncSend(int MapServerGroup,int CrywolfState,int OccupationState);
void GDCrywolfInfoSend(int MapServerGroup);
void GDGlobalPostSend(int MapServerGroup,BYTE type,char* name,char* message);
void GDGlobalNoticeSend(int MapServerGroup,BYTE type,BYTE count,BYTE opacity,WORD delay,DWORD color,BYTE speed,char* message);
void GDSNSDataSend(int aIndex);
void GDCharacterInfoSaveSend(int aIndex);
void GDInventoryItemSaveSend(int aIndex);
void GDOptionDataSaveSend(int aIndex,BYTE* SkillKey,BYTE GameOption,BYTE QKey,BYTE WKey,BYTE EKey,BYTE ChatWindow,BYTE RKey,DWORD QWERLevel);
void GDPetItemInfoSaveSend(int aIndex,int type);
void GDResetInfoSaveSend(int aIndex,int ResetDay,int ResetWek,int ResetMon);
void GDMasterResetInfoSaveSend(int aIndex,int MasterResetDay,int MasterResetWek,int MasterResetMon);
void GDRankingDuelSaveSend(int aIndex,DWORD WinScore,DWORD LoseScore);
void GDRankingBloodCastleSaveSend(int aIndex,DWORD Score);
void GDRankingChaosCastleSaveSend(int aIndex,DWORD Score);
void GDRankingDevilSquareSaveSend(int aIndex,DWORD Score);
void GDRankingIllusionTempleSaveSend(int aIndex,DWORD Score);
void GDCreationCardSaveSend(int aIndex,BYTE ExtClass);
void GDCrywolfInfoSaveSend(int MapServerGroup,int CrywolfState,int OccupationState);
void GDSNSDataSaveSend(int aIndex,BYTE* data);
void GDCustomMonsterRewardSaveSend(int aIndex,int MonsterClass,int MapNumber,int RewardValue1,int RewardValue2);
void GDRankingCustomArenaSaveSend(int aIndex,DWORD number,DWORD score,DWORD rank);
void GDConnectCharacterSend(int aIndex);
void GDDisconnectCharacterSend(int aIndex);
void GDGlobalWhisperSend(int aIndex,char* TargetName,char* message);
void GDMarryInfoSaveSend(int aIndex,char* marryname,char* mode);
void GDSetCoinSend(int aIndex, int value1, int value2, int value3, char * LogName);
void GDKillSystemSend(int aIndex,int bIndex); // OK
void GDRankingTvTEventSaveSend(int aIndex, DWORD KillCount, DWORD DeathCount);


//**************************************************************************//
// RAW FUNCTIONS ***********************************************************//
//**************************************************************************//

struct CSP_REQ_CASTLEDATA
{
	PSBMSG_HEAD h;	// C1:80:00
	WORD wMapSvrNum;	// 4
	int iCastleEventCycle;	// 8
};

struct CSP_REQ_OWNERGUILDMASTER
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
};

struct CSP_REQ_NPCBUY
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
	int iNpcNumber;	// C
	int iNpcIndex;	// 10
	int iNpcDfLevel;	// 14
	int iNpcRgLevel;	// 18
	int iNpcMaxHp;	// 1C
	int iNpcHp;	// 20
	BYTE btNpcX;	// 24
	BYTE btNpcY;	// 25
	BYTE btNpcDIR;	// 26
	int iBuyCost;	// 28
};

struct CSP_REQ_NPCREPAIR
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
	int iNpcNumber;	// C
	int iNpcIndex;	// 10
	int iRepairCost;	// 14
};

struct CSP_REQ_NPCUPGRADE
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
	int iNpcNumber;	// C
	int iNpcIndex;	// 10
	int iNpcUpType;	// 14
	int iNpcUpValue;	// 18
	int iNpcUpIndex;	// 1C
};

struct CSP_REQ_TAXINFO
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 3
	int iIndex;	// 8
};

struct CSP_REQ_TAXRATECHANGE
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
	int iTaxKind;	// C
	int iTaxRate;	// 10
};

struct CSP_REQ_MONEYCHANGE
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
	int iMoneyChanged;	// C
};

struct CSP_REQ_SDEDCHANGE
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
	WORD wStartYear;	// C
	BYTE btStartMonth;	// E
	BYTE btStartDay;	// F
	WORD wEndYear;	// 10
	BYTE btEndMonth;	// 12
	BYTE btEndDay;	// 13
};

struct CSP_REQ_GUILDREGINFO
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
	char szGuildName[8];	// C
};

struct CSP_REQ_SIEGEENDCHANGE
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	BOOL bIsSiegeEnded;	// 8
};

struct CSP_REQ_CASTLEOWNERCHANGE
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	BOOL bIsCastleOccupied;	// 8
	char szOwnerGuildName[8];	// C
};

struct CSP_REQ_REGATTACKGUILD
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
	char szEnemyGuildName[8];	// C
};

struct CSP_REQ_CASTLESIEGEEND
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
};

struct CSP_REQ_MAPSVRMULTICAST
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	char szMsgText[128];	// 6
};

struct CSP_REQ_GUILDREGMARK
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
	char szGuildName[8];	// C
	int iItemPos;	// 14
};

struct CSP_REQ_GUILDRESETMARK
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
	char szGuildName[8];	// C
};

struct CSP_REQ_GUILDSETGIVEUP
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
	char szGuildName[8];	// C
	BOOL bIsGiveUp;	// 14
};

struct CSP_REQ_NPCREMOVE
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iNpcNumber;	// 8
	int iNpcIndex;	// C
};

struct CSP_REQ_CASTLESTATESYNC
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iCastleState;	// 8
	int iTaxRateChaos;	// C
	int iTaxRateStore;	// 10
	int iTaxHuntZone;	// 14
	char szOwnerGuildName[8];	// 18
};

struct CSP_REQ_CASTLETRIBUTEMONEY
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iCastleTributeMoney;	// 8
};

struct CSP_REQ_RESETCASTLETAXINFO
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
};

struct CSP_REQ_RESETSIEGEGUILDINFO
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
};

struct CSP_REQ_RESETREGSIEGEINFO
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 2
};

struct CSP_REQ_CSINITDATA
{
	PBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iCastleEventCycle;	// 8
};

struct CSP_REQ_NPCDATA
{
	PBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
};

struct CSP_REQ_ALLGUILDREGINFO
{
	PBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iIndex;	// 8
};

struct CSP_REQ_CALCREGGUILDLIST
{
	PBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
};

struct CSP_REQ_CSLOADTOTALGUILDINFO
{
	PBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
};

struct CSP_ANS_CASTLEDATA
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xa>*/ /*|0x2|*/ WORD wStartYear;
  /*<thisrel this+0xc>*/ /*|0x1|*/ BYTE btStartMonth;
  /*<thisrel this+0xd>*/ /*|0x1|*/ BYTE btStartDay;
  /*<thisrel this+0xe>*/ /*|0x2|*/ WORD wEndYear;
  /*<thisrel this+0x10>*/ /*|0x1|*/ BYTE btEndMonth;
  /*<thisrel this+0x11>*/ /*|0x1|*/ BYTE btEndDay;
  /*<thisrel this+0x12>*/ /*|0x1|*/ BYTE btIsSiegeGuildList;
  /*<thisrel this+0x13>*/ /*|0x1|*/ BYTE btIsSiegeEnded;
  /*<thisrel this+0x14>*/ /*|0x1|*/ BYTE btIsCastleOccupied;
  /*<thisrel this+0x15>*/ /*|0x8|*/ char szCastleOwnGuild[8];
  /*<thisrel this+0x20>*/ /*|0x8|*/ __int64 i64CastleMoney;
  /*<thisrel this+0x28>*/ /*|0x4|*/ int iTaxRateChaos;
  /*<thisrel this+0x2c>*/ /*|0x4|*/ int iTaxRateStore;
  /*<thisrel this+0x30>*/ /*|0x4|*/ int iTaxHuntZone;
  /*<thisrel this+0x34>*/ /*|0x4|*/ int iFirstCreate;
};

struct CSP_ANS_OWNERGUILDMASTER
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iIndex;
  /*<thisrel this+0x10>*/ /*|0x8|*/ char szCastleOwnGuild[8];
  /*<thisrel this+0x18>*/ /*|0xa|*/ char szCastleOwnGuildMaster[10];
};

struct CSP_ANS_NPCBUY
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iIndex;
  /*<thisrel this+0x10>*/ /*|0x4|*/ int iNpcNumber;
  /*<thisrel this+0x14>*/ /*|0x4|*/ int iNpcIndex;
  /*<thisrel this+0x18>*/ /*|0x4|*/ int iBuyCost;
};

struct CSP_ANS_NPCREPAIR
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iIndex;
  /*<thisrel this+0x10>*/ /*|0x4|*/ int iNpcNumber;
  /*<thisrel this+0x14>*/ /*|0x4|*/ int iNpcIndex;
  /*<thisrel this+0x18>*/ /*|0x4|*/ int iNpcMaxHp;
  /*<thisrel this+0x1c>*/ /*|0x4|*/ int iNpcHp;
  /*<thisrel this+0x20>*/ /*|0x4|*/ int iRepairCost;
};

struct CSP_ANS_NPCUPGRADE
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iIndex;
  /*<thisrel this+0x10>*/ /*|0x4|*/ int iNpcNumber;
  /*<thisrel this+0x14>*/ /*|0x4|*/ int iNpcIndex;
  /*<thisrel this+0x18>*/ /*|0x4|*/ int iNpcUpType;
  /*<thisrel this+0x1c>*/ /*|0x4|*/ int iNpcUpValue;
  /*<thisrel this+0x20>*/ /*|0x4|*/ int iNpcUpIndex;
};

struct CSP_ANS_TAXINFO
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iIndex;
  /*<thisrel this+0x10>*/ /*|0x8|*/ __int64 i64CastleMoney;
  /*<thisrel this+0x18>*/ /*|0x4|*/ int iTaxRateChaos;
  /*<thisrel this+0x1c>*/ /*|0x4|*/ int iTaxRateStore;
  /*<thisrel this+0x20>*/ /*|0x4|*/ int iTaxHuntZone;
};

struct CSP_ANS_TAXRATECHANGE
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iIndex;
  /*<thisrel this+0x10>*/ /*|0x4|*/ int iTaxKind;
  /*<thisrel this+0x14>*/ /*|0x4|*/ int iTaxRate;
};

struct CSP_ANS_MONEYCHANGE
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iIndex;
  /*<thisrel this+0x10>*/ /*|0x4|*/ int iMoneyChanged;
  /*<thisrel this+0x18>*/ /*|0x8|*/ __int64 i64CastleMoney;
};

struct CSP_ANS_SDEDCHANGE
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iIndex;
  /*<thisrel this+0x10>*/ /*|0x2|*/ WORD wStartYear;
  /*<thisrel this+0x12>*/ /*|0x1|*/ BYTE btStartMonth;
  /*<thisrel this+0x13>*/ /*|0x1|*/ BYTE btStartDay;
  /*<thisrel this+0x14>*/ /*|0x2|*/ WORD wEndYear;
  /*<thisrel this+0x16>*/ /*|0x1|*/ BYTE btEndMonth;
  /*<thisrel this+0x17>*/ /*|0x1|*/ BYTE btEndDay;
};

struct CSP_ANS_SIEGEENDCHANGE
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int bIsSiegeEnded;
};

struct CSP_ANS_CASTLEOWNERCHANGE
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int bIsCastleOccupied;
  /*<thisrel this+0x10>*/ /*|0x8|*/ char szOwnerGuildName[8];
};

struct CSP_ANS_REGATTACKGUILD
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iIndex;
  /*<thisrel this+0x10>*/ /*|0x8|*/ char szEnemyGuildName[8];
};

struct CSP_ANS_CASTLESIEGEEND
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
};

struct CSP_ANS_MAPSVRMULTICAST
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	char szMsgText[128];	// 6
};

struct CSP_ANS_GUILDRESETMARK
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iIndex;
  /*<thisrel this+0x10>*/ /*|0x8|*/ char szGuildName[8];
  /*<thisrel this+0x18>*/ /*|0x4|*/ int iRegMarkCount;
};

struct CSP_ANS_GUILDSETGIVEUP
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iIndex;
  /*<thisrel this+0x10>*/ /*|0x8|*/ char szGuildName[8];
  /*<thisrel this+0x18>*/ /*|0x4|*/ int bIsGiveUp;
  /*<thisrel this+0x1c>*/ /*|0x4|*/ int iRegMarkCount;
};

struct CSP_ANS_NPCREMOVE
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iNpcNumber;
  /*<thisrel this+0x10>*/ /*|0x4|*/ int iNpcIndex;
};

struct CSP_ANS_CASTLESTATESYNC
{
	PSBMSG_HEAD h;
	WORD wMapSvrNum;	// 4
	int iCastleState;	// 8
	int iTaxRateChaos;	// C
	int iTaxRateStore;	// 10
	int iTaxHuntZone;	// 14
	char szOwnerGuildName[8];	// 18
};

struct CSP_ANS_CASTLETRIBUTEMONEY
{
	PSBMSG_HEAD h;
	int iResult;	// 4
	WORD wMapSvrNum;	// 8
};

struct CSP_ANS_RESETCASTLETAXINFO
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
};

struct CSP_ANS_RESETSIEGEGUILDINFO
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
};

struct CSP_ANS_RESETREGSIEGEINFO
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
};

struct CSP_NPCDATA
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ int iNpcNumber;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iNpcIndex;
  /*<thisrel this+0x8>*/ /*|0x4|*/ int iNpcDfLevel;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iNpcRgLevel;
  /*<thisrel this+0x10>*/ /*|0x4|*/ int iNpcMaxHp;
  /*<thisrel this+0x14>*/ /*|0x4|*/ int iNpcHp;
  /*<thisrel this+0x18>*/ /*|0x1|*/ BYTE btNpcX;
  /*<thisrel this+0x19>*/ /*|0x1|*/ BYTE btNpcY;
  /*<thisrel this+0x1a>*/ /*|0x1|*/ BYTE btNpcDIR;
};

struct CSP_ANS_ALLGUILDREGINFO
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ PWMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iIndex;
  /*<thisrel this+0x10>*/ /*|0x4|*/ int iCount;
};

struct CSP_GUILDREGINFO
{
  /*<thisrel this+0x0>*/ /*|0x8|*/ char szGuildName[8];
  /*<thisrel this+0x8>*/ /*|0x4|*/ int iRegMarkCount;
  /*<thisrel this+0xc>*/ /*|0x1|*/ BYTE bIsGiveUp;
  /*<thisrel this+0xd>*/ /*|0x1|*/ BYTE btRegRank;
};

struct PMSG_ANS_CSREGGUILDLIST
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ PWMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x1|*/ BYTE btResult;
  /*<thisrel this+0x8>*/ /*|0x4|*/ int iCount;
};

struct PMSG_CSREGGUILDLIST
{
  /*<thisrel this+0x0>*/ /*|0x8|*/ char szGuildName[8];
  /*<thisrel this+0x8>*/ /*|0x1|*/ BYTE btRegMarks1;
  /*<thisrel this+0x9>*/ /*|0x1|*/ BYTE btRegMarks2;
  /*<thisrel this+0xa>*/ /*|0x1|*/ BYTE btRegMarks3;
  /*<thisrel this+0xb>*/ /*|0x1|*/ BYTE btRegMarks4;
  /*<thisrel this+0xc>*/ /*|0x1|*/ BYTE btIsGiveUp;
  /*<thisrel this+0xd>*/ /*|0x1|*/ BYTE btSeqNum;
};

struct CSP_ANS_NPCSAVEDATA
{
  /*<thisrel this+0x0>*/ /*|0x3|*/ struct PBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
};

struct CSP_ANS_CALCREGGUILDLIST
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ PWMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iCount;
};

struct CSP_ANS_CSGUILDUNIONINFO
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PWMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iCount;
};

struct CSP_ANS_CSSAVETOTALGUILDINFO
{
  /*<thisrel this+0x0>*/ /*|0x3|*/ PBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
};

struct CSP_ANS_CSLOADTOTALGUILDINFO
{
  /*<thisrel this+0x0>*/ /*|0x4|*/ PWMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
  /*<thisrel this+0xc>*/ /*|0x4|*/ int iCount;
};

struct CSP_ANS_NPCUPDATEDATA
{
  /*<thisrel this+0x0>*/ /*|0x3|*/ PBMSG_HEAD h;
  /*<thisrel this+0x4>*/ /*|0x4|*/ int iResult;
  /*<thisrel this+0x8>*/ /*|0x2|*/ WORD wMapSvrNum;
};

void GS_GDReqCastleTotalInfo(int iMapSvrGroup, int iCastleEventCycle);
void GS_GDReqOwnerGuildMaster(int iMapSvrGroup, int aIndex);
void GS_GDReqCastleNpcBuy(int iMapSvrGroup, int aIndex, int iNpcNumber, int iNpcIndex, int iNpcDfLevel, int iNpcRgLevel, int iNpcMaxHP, int iNpcHP, BYTE btNpcX, BYTE btNpcY, BYTE btNpcDIR,  int iBuyCost);
void GS_GDReqCastleNpcRepair(int iMapSvrGroup, int aIndex, int iNpcNumber, int iNpcIndex, int iRepairCost);
void GS_GDReqCastleNpcUpgrade(int iMapSvrGroup, int aIndex, int iNpcNumber, int iNpcIndex, int iNpcUpType, int iNpcUpValue, int iNpcUpIndex);
void GS_GDReqTaxInfo(int iMapSvrGroup, int aIndex);
void GS_GDReqTaxRateChange(int iMapSvrGroup, int aIndex, int iTaxType, int iTaxRate);
void GS_GDReqCastleMoneyChange(int iMapSvrGroup, int aIndex, int iMoneyChange);
void GS_GDReqSiegeDateChange(int iMapSvrGroup, int aIndex, WORD wStartYear, BYTE btStartMonth, BYTE btStartDay, WORD wEndYear, BYTE btEndMonth, BYTE btEndDay);
void GS_GDReqGuildMarkRegInfo(int iMapSvrGroup, int aIndex);
void GS_GDReqSiegeEndedChange(int iMapSvrGroup, int bIsSiegeEnded);
void GS_GDReqCastleOwnerChange(int iMapSvrGroup, int bIsCastleOccupied, char* lpszGuildName);
void GS_GDReqRegAttackGuild(int iMapSvrGroup, int aIndex);
void GS_GDReqRestartCastleState(int iMapSvrGroup);
void GS_GDReqMapSvrMsgMultiCast(int iMapSvrGroup, char* lpszMsgText);
void GS_GDReqRegGuildMark(int iMapSvrGroup, int aIndex, int iItemPos);
void GS_GDReqGuildMarkReset(int iMapSvrGroup, int aIndex, char* lpszGuildName);
void GS_GDReqGuildSetGiveUp(int iMapSvrGroup, int aIndex, int bIsGiveUp);
void GS_GDReqNpcRemove(int iMapSvrGroup, int iNpcNumber, int iNpcIndex);
void GS_GDReqCastleStateSync(int iMapSvrGroup, int iCastleState, int iTaxRateChaos, int iTaxRateStore, int iTaxHuntZone, char* lpszOwnerGuild);
void GS_GDReqCastleTributeMoney(int iMapSvrGroup, int iCastleTributeMoney);
void GS_GDReqResetCastleTaxInfo(int iMapSvrGroup);
void GS_GDReqResetSiegeGuildInfo(int iMapSvrGroup);
void GS_GDReqResetRegSiegeInfo(int iMapSvrGroup);
void GS_GDReqCastleInitData(int iMapSvrGroup, int iCastleEventCycle);
void GS_GDReqCastleNpcInfo(int iMapSvrGroup, int aIndex);
void GS_GDReqAllGuildMarkRegInfo(int iMapSvrGroup, int aIndex);
void GS_GDReqCalcRegGuildList(int iMapSvrGroup);
void GS_GDReqCsLoadTotalGuildInfo(int iMapSvrGroup);
void GS_DGAnsCastleTotalInfo(LPBYTE lpRecv);
void GS_DGAnsOwnerGuildMaster(LPBYTE lpRecv);
void GS_DGAnsCastleNpcBuy(LPBYTE lpRecv);
void GS_DGAnsCastleNpcRepair(LPBYTE lpRecv);
void GS_DGAnsCastleNpcUpgrade(LPBYTE lpRecv);
void GS_DGAnsTaxInfo(LPBYTE lpRecv);
void GS_DGAnsTaxRateChange(LPBYTE lpRecv);
void GS_DGAnsCastleMoneyChange(LPBYTE lpRecv);
void GS_DGAnsSiegeDateChange(LPBYTE lpRecv);
void GS_DGAnsGuildMarkRegInfo(LPBYTE lpRecv);
void GS_DGAnsSiegeEndedChange(LPBYTE lpRecv);
void GS_DGAnsCastleOwnerChange(LPBYTE lpRecv);
void GS_DGAnsRegAttackGuild(LPBYTE lpRecv);
void GS_DGAnsRestartCastleState(LPBYTE lpRecv);
void GS_DGAnsMapSvrMsgMultiCast(LPBYTE lpRecv);
void GS_DGAnsRegGuildMark(LPBYTE lpRecv);
void GS_DGAnsGuildMarkReset(LPBYTE lpRecv);
void GS_DGAnsGuildSetGiveUp(LPBYTE lpRecv);
void GS_DGAnsNpcRemove(LPBYTE lpRecv);
void GS_DGAnsCastleStateSync(LPBYTE lpRecv);
void GS_DGAnsCastleTributeMoney(LPBYTE lpRecv);
void GS_DGAnsResetCastleTaxInfo(LPBYTE lpRecv);
void GS_DGAnsResetSiegeGuildInfo(LPBYTE lpRecv);
void GS_DGAnsResetRegSiegeInfo(LPBYTE lpRecv);
void GS_DGAnsCastleInitData(LPBYTE lpRecv);
void GS_DGAnsCastleNpcInfo(LPBYTE lpRecv);
void GS_DGAnsAllGuildMarkRegInfo(LPBYTE lpRecv);
void GS_DGAnsFirstCreateNPC(LPBYTE lpRecv);
void GS_DGAnsCalcREgGuildList(LPBYTE lpRecv);
void GS_DGAnsCsGulidUnionInfo(LPBYTE lpRecv);
void GS_DGAnsCsSaveTotalGuildInfo(LPBYTE lpRecv);
void GS_DGAnsCsLoadTotalGuildInfo(LPBYTE lpRecv);
void GS_DGAnsCastleNpcUpdate(LPBYTE lpRecv);

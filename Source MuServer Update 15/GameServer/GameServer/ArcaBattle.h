// ArcaBattle.h: interface for the CArcaBattle class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

#define MAX_ARCA_BATTLE_OBELISK 5
#define MAX_ARCA_BATTLE_OBELISK_AURA 3

enum eArcaBattleState
{
	AB_STATE_BLANK = 0,
	AB_STATE_EMPTY = 1,
	AB_STATE_SIGN1 = 2,
	AB_STATE_SIGN2 = 3,
	AB_STATE_READY = 4,
	AB_STATE_PARTY = 5,
	AB_STATE_START = 6,
	AB_STATE_CLOSE = 7,
	AB_STATE_CLEAN = 8,
	AB_STATE_CLEAR = 9,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_ARCA_BATTLE_ENTER_RECV
{
	PSBMSG_HEAD header; // C1:F8:34
	BYTE seq;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_ARCA_BATTLE_GUILD_MASTER_REGISTER_SEND
{
	PSBMSG_HEAD header; // C1:F8:31
	BYTE result;
};

struct PMSG_ARCA_BATTLE_OBELISK_INFO_SEND
{
	PSBMSG_HEAD header; // C1:F8:37
	BYTE ObeliskAttribute;
	BYTE ObeliskAuraState[MAX_ARCA_BATTLE_OBELISK_AURA];
};

struct PMSG_ARCA_BATTLE_STATE_SEND
{
	PSBMSG_HEAD header; // C1:F8:38
	BYTE state;
};

struct PMSG_ARCA_BATTLE_INFO_SEND
{
	PSBMSG_HEAD header; // C1:F8:39
	WORD GuildGroupNum;
	union
	{
		DWORD GroupNumber;
		DWORD AttrKind;
		DWORD MapNumber;
		DWORD MapX;
		DWORD MapY;
		union
		{
			DWORD AuraX;
			DWORD AuraY;
		}AuraInfo[MAX_ARCA_BATTLE_OBELISK_AURA];
	}ObeliskInfo[MAX_ARCA_BATTLE_OBELISK];
};

struct PMSG_ARCA_BATTLE_OBELISK_STATE_SEND
{
	PSBMSG_HEAD header; // C1:F8:3A
	BYTE ObeliskAttribute;
	BYTE ObeliskState;
	BYTE ObeliskAuraState;
};

struct PMSG_ARCA_BATTLE_SCORE_SEND
{
	PSBMSG_HEAD header; // C1:F8:3B
	BYTE result;
	DWORD ContributePoint;
	DWORD KillPoint;
	DWORD RewardExperience;
	WORD BootyCount;
	union
	{
		WORD ObeliskKind;
		char GuildName[9];
	}ObeliskOccupyInfp[MAX_ARCA_BATTLE_OBELISK];
};

struct PMSG_ARCA_BATTLE_OCCUPY_GUILD_SEND
{
	PSBMSG_HEAD header; // C1:F8:3D
	char GuildName[9];
};

struct PMSG_ARCA_BATTLE_OBELISK_LIFE_SEND
{
	PSBMSG_HEAD header; // C1:F8:3F
	union
	{
		BYTE ObeliskLife;
		BYTE ObeliskAttribute;
	}ObeliskLife[MAX_ARCA_BATTLE_OBELISK];
};

struct PMSG_ARCA_BATTLE_REGISTER_MEMBER_COUNT_SEND
{
	PSBMSG_HEAD header; // C1:F8:42
	BYTE result;
	BYTE MemberCount;
};

struct PMSG_ARCA_BATTLE_REGISTER_MARK_SEND
{
	PSBMSG_HEAD header; // C1:F8:44
	BYTE MarkCount[4];
};

struct PMSG_ARCA_BATTLE_REGISTER_MARK_ERROR_SEND
{
	PSBMSG_HEAD header; // C1:F8:47
	BYTE result;
};

struct _stPMSG_AB_JOIN_USER_COUNT {
	//CArcaBattle::GCArcaBattleUserInfo
  // static data ------------------------------------

  // non-static data --------------------------------
  /*<thisrel this+0x0>*/ /*|0x5|*/ struct PSWMSG_HEAD h; // C1:F8:48
  /*<thisrel this+0x5>*/ /*|0x1|*/ unsigned char Result;
  /*<thisrel this+0x6>*/ /*|0x1|*/ unsigned char btUserCnt;
  /*<thisrel this+0x7>*/ /*|0x4|*/ int iGuildNumber;
  /*<thisrel this+0xb>*/ /*|0xb|*/ char szGuildMasterName[11];
  /*<thisrel this+0x16>*/ /*|0x4|*/ unsigned long dwGuildMasterGateNum;
  /*<thisrel this+0x1a>*/ /*|0x1|*/ unsigned char btGuildMasterPosX;
  /*<thisrel this+0x1b>*/ /*|0x1|*/ unsigned char btGuildMasterPosY;
};

struct _stABCurJoinGuildUserInfo {

  // static data ------------------------------------

  // non-static data --------------------------------
  /*<thisrel this+0x0>*/ /*|0xb|*/ char szUserName[11];
  /*<thisrel this+0xb>*/ /*|0x1|*/ unsigned char btStatus;
  /*<thisrel this+0xc>*/ /*|0x1|*/ unsigned char btPosX;
  /*<thisrel this+0xd>*/ /*|0x1|*/ unsigned char btPosY;
  /*<thisrel this+0xe>*/ /*|0x2|*/ unsigned short wGuildMasterGateNum;

  // base classes -----------------------------------

  // friends ----------------------------------------

  // static functions -------------------------------

  // non-virtual functions --------------------------

  // virtual functions ------------------------------
};

struct _tagPMSG_ARCA_BATTLE_NOTI_MSG {

  // static data ------------------------------------

  // non-static data --------------------------------
  /*<thisrel this+0x0>*/ /*|0x4|*/ struct PSBMSG_HEAD h; // C1:F8:4A
  /*<thisrel this+0x4>*/ /*|0x1|*/ unsigned char btNotiIndex;
  /*<thisrel this+0x8>*/ /*|0x4|*/ int iValue;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_ARCA_BATTLE_GUILD_MASTER_REGISTER_SEND
{
	PSBMSG_HEAD header; // C1:1B:00
};

struct SDHP_ARCA_BATTLE_GUILD_MEMBER_REGISTER_SEND
{
	PSBMSG_HEAD header; // C1:1B:00
};


//**********************************************//
//**********************************************//
//**********************************************//

struct ARCA_BATTLE_START_TIME
{
	int State;
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct ARCA_BATTLE_OBELISK_AURA_INFO
{
	ARCA_BATTLE_OBELISK_AURA_INFO()
	{
		this->Group = -1;
		this->Index = -1;
		this->Map = -1;
		this->X = -1;
		this->Y = -1;
	}

	int Group;
	int Index;
	int Map;
	int X;
	int Y;
};

struct ARCA_BATTLE_OBELISK_INFO
{
	ARCA_BATTLE_OBELISK_INFO()
	{
		this->Group = -1;
		this->MonsterClass = -1;
		this->Map = -1;
		this->X = -1;
		this->Y = -1;
		this->Attr = 0;
		this->Life = 0;
		this->OccupyLife = 0;
		this->CreatedTime = 0;
		this->RespawnTime = 20000;
	}

	int Group;
	int MonsterClass;
	int Map;
	int X;
	int Y;
	int Attr;
	int Life;
	int OccupyLife;
	int CreatedTime;
	int RespawnTime;
	ARCA_BATTLE_OBELISK_AURA_INFO AuraInfo[MAX_ARCA_BATTLE_OBELISK_AURA];
};

struct ARCA_BATTLE_OBELISK_AURA_STATE_INFO
{
	int Group;
	int State;
	int UserIndex;
	int Index;
};

struct ARCA_BATTLE_OBELISK_STATE_INFO
{
	int Group;
	int MonsterIndex;
	int AuraState;
	int AuraReleaseGuildNum;
	int ObeliskState;
	int OccupyGuildNum;
	char OccupyGuildName[9];
	ARCA_BATTLE_OBELISK_AURA_STATE_INFO AuraStateInfo[MAX_ARCA_BATTLE_OBELISK_AURA];
};

struct ARCA_BATTLE_USER_INFO
{
	char Name[11];
	bool	bBootyDrop;
	DWORD	dwBootyCnt;
	DWORD	dwContributePoints;
	DWORD	dwKillPoints;
};

struct ARCA_BATTLE_GUILD_INFO
{
	char GuildName[9];
	int GuildNumber;
	int OccupyObelisk;
	int ObeliskGroup;
};

class CArcaBattle
{
public:
	CArcaBattle();
	virtual ~CArcaBattle();
	void Init();
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK();
	void ProcState_EMPTY();
	void ProcState_SIGN1();
	void ProcState_SIGN2();
	void ProcState_READY();
	void ProcState_PARTY();
	void ProcState_START();
	void ProcState_CLOSE();
	void ProcState_CLEAN();
	void ProcState_CLEAR();
	void SetState(int state);
	void SetState_BLANK();
	void SetState_EMPTY();
	void SetState_SIGN1();
	void SetState_SIGN2();
	void SetState_READY();
	void SetState_PARTY();
	void SetState_START();
	void SetState_CLOSE();
	void SetState_CLEAN();
	void SetState_CLEAR();
	void CheckSync();
	void SetObelisk();
	void SetObeliskAura(int group);
	void ChkAuraUserHover();
	bool RegenObelisk();
	bool IsEnableAttackObelisk(LPOBJ lpObj,int MonsterClass);
	bool IsPkEnable(LPOBJ lpObj,LPOBJ lpTarget);
	void NpcLesnar(LPOBJ lpNpc,LPOBJ lpObj);
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void NoticeSendToAll(int type,char* message,...);
	void DataSendToAll(BYTE* lpMsg,int size);
	void CGArcaBattleGuildMasterRegisterRecv(int aIndex);
	void CGArcaBattleGuildMemberRegisterRecv(int aIndex);
	void CGArcaBattleEnterRecv(int aIndex);
	void CGArcaBattleRegisterMarkRecv(int aIndex);
	void GCArcaBattleStateSend(int state,int aIndex);
	void GCArcaBattleInfoSend(int aIndex,int GuildGroupNum);
	void GCArcaBattleObeliskStateSend();
	void GCArcaBattleOccupyGuildSend(LPOBJ lpObj,int gate);
	void GCArcaBattleObeliskLifeSend();
private:
	int m_State;
	int m_RemainTime;
	int m_TargetTime;
	int m_TickCount;
	ARCA_BATTLE_OBELISK_INFO m_ObeliskInfo[MAX_ARCA_BATTLE_OBELISK];
	ARCA_BATTLE_OBELISK_STATE_INFO m_ObeliskStateInfo[MAX_ARCA_BATTLE_OBELISK];

	ARCA_BATTLE_GUILD_INFO m_GuildInfo[MAX_ARCA_BATTLE_OBELISK];

	std::vector<ARCA_BATTLE_START_TIME> m_ArcaBattleStartTime;
};

extern CArcaBattle gArcaBattle;

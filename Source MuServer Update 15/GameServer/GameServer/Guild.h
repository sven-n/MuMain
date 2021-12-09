// Guild.h: interface for the CGuild class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

enum eGuildStatus
{
	GUILD_STATUS_MEMBER = 0,
	GUILD_STATUS_BATTLE_MASTER = 32,
	GUILD_STATUS_OFFICE_MASTER = 64,
	GUILD_STATUS_MASTER = 128,
	GUILD_STATUS_NONE = 255,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_GUILD_REQUEST_RECV
{
	PBMSG_HEAD header; // C1:50
	BYTE index[2];
};

struct PMSG_GUILD_RESULT_RECV
{
	PBMSG_HEAD header; // C1:51
	BYTE result;
	BYTE index[2];
};

struct PMSG_GUILD_DELETE_RECV
{
	PBMSG_HEAD header; // C1:53
	char name[10];
	char PersonalCode[10];
};

struct PMSG_GUILD_ASSIGN_STATUS_RECV
{
	PBMSG_HEAD header; // C1:E1
	BYTE type;
	BYTE status;
	char name[10];
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_GUILD_REQUEST_SEND
{
	PBMSG_HEAD header; // C1:50
	BYTE index[2];
};

struct PMSG_GUILD_RESULT_SEND
{
	PBMSG_HEAD header; // C1:51
	BYTE result;
};

struct PMSG_GUILD_LIST_SEND
{
	PWMSG_HEAD header; // C2:52
	BYTE result;
	BYTE count;
	DWORD TotalScore;
	BYTE score;
	char RivalGuild[9];
	#if(GAMESERVER_UPDATE>=701)
	BYTE RivalCount;
	BYTE RivalMark[32];
	#endif
};

struct PMSG_GUILD_LIST
{
	char name[10];
	BYTE number;
	BYTE connected;
	BYTE status;
};

struct PMSG_GUILD_DELETE_SEND
{
	PBMSG_HEAD header; // C1:53
	BYTE result;
};

struct PMSG_GUILD_WAR_END_SEND
{
	PBMSG_HEAD header; // C1:63
	BYTE result;
	char GuildName[8];
};

struct PMSG_GUILD_WAR_SCORE_SEND
{
	PBMSG_HEAD header; // C1:64
	BYTE score[2];
	BYTE type;
};

struct PMSG_GUILD_ASSIGN_STATUS_SEND
{
	PBMSG_HEAD header; // C1:E1
	BYTE type;
	BYTE result;
	char name[10];
};

//**********************************************//
//**********************************************//
//**********************************************//

struct GUILD_INFO_STRUCT
{
	int GetTimeStamp() // OK
	{
		return this->TimeStamp;
	}

	BOOL CheckTimeStamp(int time) // OK
	{
		return ((time==this->TimeStamp)?1:0);
	}

	void SetTimeStamp() // OK
	{
		this->TimeStamp++;
	}

	void SetGuildUnion(int GuildNumber) // OK
	{
		this->GuildUnion = GuildNumber;
		this->SetTimeStamp();
	}

	void SetGuildRival(int GuildNumber) // OK
	{
		this->GuildRival = GuildNumber;
		this->SetTimeStamp();
	}

	int Number;
	char Name[9];
	BYTE Mark[32];
	BYTE Count;
	BYTE TotalCount;
	char Names[MAX_GUILD_USER][11];
	short Index[MAX_GUILD_USER];
	BYTE Use[MAX_GUILD_USER];
	char Server[MAX_GUILD_USER];
	char TargetGuildName[9];
	short TargetIndex[MAX_GUILD_USER];
	GUILD_INFO_STRUCT* TargetGuildNode;
	BYTE WarDeclareState;
	BYTE WarState;
	BYTE WarType;
	BYTE BattleGroundIndex;
	BYTE BattleTeamCode;
	BYTE PlayScore;
	int TotalScore;
	char Notice[60];
	int GuildStatus[MAX_GUILD_USER];
	BYTE GuildType;
	int GuildUnion;
	int GuildRival;
	int TimeStamp;
	char GuildRivalName[9];
	#if(GAMESERVER_TYPE==1)
	OBJECTSTRUCT* LifeStone;
	int LifeStoneCount;
	#endif

	GUILD_INFO_STRUCT* back;
	GUILD_INFO_STRUCT* next;
};

class CGuild
{
public:
	CGuild();
	virtual ~CGuild();
	int GetMaxGuildUser(LPOBJ lpObj);
	int GetGuildNameCount(LPOBJ lpObj,char* name);
	int GetGuildBattleMasterCount(LPOBJ lpObj);
	int GetGuildOfficeMasterCount(LPOBJ lpObj);
	void CGGuildRequestRecv(PMSG_GUILD_REQUEST_RECV* lpMsg,int aIndex);
	void CGGuildResultRecv(PMSG_GUILD_RESULT_RECV* lpMsg,int aIndex);
	void CGGuildListRecv(int aIndex);
	void CGGuildDeleteRecv(PMSG_GUILD_DELETE_RECV* lpMsg,int aIndex);
	void CGGuildAssignStatusRecv(PMSG_GUILD_ASSIGN_STATUS_RECV* lpMsg,int aIndex);
	void GCGuildResultSend(int aIndex,int result);
	void GCGuildWarEndSend(int aIndex,int result,char* GuildName);
	void GCGuildWarScoreSend(int aIndex);
	void GCGuildAssignStatusSend(int aIndex,int type,int result,char* name);
};

extern CGuild gGuild;

//**************************************************************************//
// RAW FUNCTIONS ***********************************************************//
//**************************************************************************//

struct PMSG_GUILDMASTERANSWER
{
	PBMSG_HEAD h; //0x54
	BYTE Result;	// 3
};

struct PMSG_GUILDMASTERMANAGER_RUN
{
	PBMSG_HEAD h;	// C1:55
};

struct PMSG_GUILDINFOSAVE
{
	PBMSG_HEAD h; // 55
	BYTE btGuildType;	// 3
	char GuildName[8];	// 4
	BYTE Mark[32];	// C
};

struct PMSG_GUILDCREATED_RESULT
{
	PBMSG_HEAD h;	// C1:56
	BYTE Result;	// 3
	BYTE btGuildType;	// 4
};

struct PMSG_GUILDWARSEND_RESULT
{
	PBMSG_HEAD h; //61
	BYTE Result;	// 3
};

struct PMSG_GUILDWARSEND
{
	PBMSG_HEAD h;	// C1:61
	char GuildName[8];	// 3
	BYTE Type;	// B
};

struct PMSG_GUILDWARREQUEST_RESULT
{
	PBMSG_HEAD h; //60
	BYTE Result;
};

struct PMSG_REQ_GUILDVIEWPORT
{
	PBMSG_HEAD h; //6
	int GuildNumber;	// 4
};

struct PMSG_ANS_GUILDVIEWPORT
{
	PBMSG_HEAD h;	// C1:66
	int GuildNumber;	// 4
	BYTE btGuildType;	// 8
	BYTE UnionName[8];	// 9
	char GuildName[8];	// 11
	BYTE Mark[32];	// 19
};

struct PMSG_GUILDWAR_DECLARE
{
	PBMSG_HEAD h; // 62
	char GuildName[8];
	BYTE Type;
	BYTE TeamCode;
};

struct PMSG_GUILD_ASSIGN_TYPE_REQ
{
	PBMSG_HEAD h; //E2
	BYTE btGuildType;	// 3
};

struct PMSG_GUILD_ASSIGN_TYPE_RESULT
{
	PBMSG_HEAD h;	// C1:E2
	BYTE btGuildType;	// 3
	BYTE btResult;	// 4
};

struct PMSG_RELATIONSHIP_JOIN_BREAKOFF_REQ
{
	PBMSG_HEAD h;	// C1:E5
	BYTE btRelationShipType;	// 3
	BYTE btRequestType;	// 4
	BYTE btTargetUserIndexH;	// 5
	BYTE btTargetUserIndexL;	// 6
};

struct PMSG_RELATIONSHIP_JOIN_BREAKOFF_ANS
{
	PBMSG_HEAD h;	// C1:E6
	BYTE btRelationShipType;	// 3
	BYTE btRequestType;	// 4
	BYTE btResult;	// 5
	BYTE btTargetUserIndexH;	// 6
	BYTE btTargetUserIndexL;	// 7
};

struct PMSG_UNIONLIST_REQ
{
	PBMSG_HEAD h;
};

struct PMSG_KICKOUT_UNIONMEMBER_REQ
{
	PSBMSG_HEAD h;
	char szTargetGuildName[8];	// 4
};

struct PMSG_UNIONLIST_COUNT
{
	PWMSG_HEAD h;
	BYTE btCount;	// 4
	BYTE btResult;	// 5
	BYTE btRivalMemberNum;	// 6
	BYTE btUnionMemberNum;	// 7
};

struct PMSG_UNIONLIST
{
	BYTE btMemberNum;	// 0
	BYTE Mark[32];	// 1
	char szGuildName[8];	// 21
};

struct PMSG_KICKOUT_UNIONMEMBER_RESULT
{
	PSBMSG_HEAD h;
	BYTE btResult;	// 4
	BYTE btRequestType;	// 5
	BYTE btRelationShipType;	// 6
};

struct PMSG_GUILD_ASSIGN_STATUS_RESULT
{
	PBMSG_HEAD h;	// C1:E1
	BYTE btType;	// 3
	BYTE btResult;	// 4
	char szTagetName[10];	// 5
};

struct PMSG_GUILDDEL_VIEWPORT_NOW
{
	PBMSG_HEAD h;
	BYTE NumberH;
	BYTE NumberL;
};

struct PMSG_SIMPLE_GUILDVIEWPORT_COUNT
{
	PWMSG_HEAD h;	// C2:65
	BYTE Count;	// 4
};

struct PMSG_SIMPLE_GUILDVIEWPORT
{
	int GuildNumber;	// 0
	BYTE btGuildStatus;	// 4
	BYTE btGuildType;	// 5
	BYTE btGuildRelationShip;	// 6
	BYTE NumberH;	// 7
	BYTE NumberL;	// 8
	BYTE btOwnerStatus; //9 season4 add-on
};

void GCGuildWarDeclare(int aIndex, LPSTR _guildname);
void CGGuildMasterAnswerRecv(PMSG_GUILDMASTERANSWER * lpMsg, int aIndex);
void GCGuildMasterManagerRun(int aIndex);
void CGGuildMasterInfoSave(int aIndex, PMSG_GUILDINFOSAVE * lpMsg);
void CGGuildMasterCreateCancel(int aIndex);
void GCGuildWarRequestResult(LPSTR GuildName, int aIndex, int type);
void GCGuildWarRequestSend(LPSTR GuildName, int aIndex, int type);
void GCGuildWarRequestSendRecv(PMSG_GUILDWARSEND_RESULT * lpMsg, int aIndex);
void GCGuildViewportInfo(PMSG_REQ_GUILDVIEWPORT * aRecv, int aIndex);
void CGGuildAssignType(PMSG_GUILD_ASSIGN_TYPE_REQ * aRecv, int aIndex);
void CGRelationShipReqJoinBreakOff(PMSG_RELATIONSHIP_JOIN_BREAKOFF_REQ * aRecv, int aIndex);
void CGRelationShipAnsJoinBreakOff(PMSG_RELATIONSHIP_JOIN_BREAKOFF_ANS * aRecv, int aIndex);
void CGUnionList(PMSG_UNIONLIST_REQ * aRecv, int aIndex);
void CGRelationShipReqKickOutUnionMember(PMSG_KICKOUT_UNIONMEMBER_REQ* aRecv, int aIndex);
void GCResultSend(int aIndex, BYTE headcode, BYTE result);
void GCGuildViewportDelNow(int aIndex, BOOL isGuildMaster);

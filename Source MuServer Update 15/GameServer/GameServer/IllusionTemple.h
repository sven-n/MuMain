// IllusionTemple.h: interface for the CIllusionTemple class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MapItem.h"
#include "Protocol.h"
#include "User.h"

#define MAX_IT_LEVEL 6
#define MAX_IT_USER 10
#define MAX_IT_TEAM 2
#define MAX_IT_RANK 10
#define MAX_IT_MONSTER 100

#define GET_IT_LEVEL(x) (((x)<MAP_ILLUSION_TEMPLE1)?-1:((x)>MAP_ILLUSION_TEMPLE6)?-1:((x)-MAP_ILLUSION_TEMPLE1))

#define IT_LEVEL_RANGE(x) (((x)<0)?0:((x)>=MAX_IT_LEVEL)?0:1)

enum eIllusionTempleState
{
	IT_STATE_BLANK = 0,
	IT_STATE_EMPTY = 1,
	IT_STATE_STAND = 2,
	IT_STATE_START = 3,
	IT_STATE_CLEAN = 4,
};

enum eIllusionTempleTeam
{
	IT_TEAM_NONE = -1,
	IT_TEAM_Y = 0,
	IT_TEAM_B = 1,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_ILLUSION_TEMPLE_ENTER_RECV
{
	PSBMSG_HEAD header; // C1:BF:00
	BYTE level;
	BYTE slot;
};

struct PMSG_ILLUSION_TEMPLE_SKILL_RECV
{
	PSBMSG_HEAD header; // C1:BF:02
	BYTE skill[2];
	BYTE index[2];
	BYTE dir;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_ILLUSION_TEMPLE_ENTER_SEND
{
	PSBMSG_HEAD header; // C1:BF:00
	BYTE result;
};

struct PMSG_ILLUSION_TEMPLE_BATTLE_INFO_SEND
{
	PSBMSG_HEAD header; // C1:BF:01
	WORD time;
	WORD EventItemOwner;
	BYTE EventItemOwnerX;
	BYTE EventItemOwnerY;
	BYTE score[2];
	BYTE team;
	BYTE count;
};

struct PMSG_ILLUSION_TEMPLE_BATTLE_INFOS8_SEND
{
	PSBMSG_HEAD header;
	WORD wRemainSec;
	BYTE byAlliedPoint;
	BYTE btIllusionPoint;
	BYTE btMyTeam;
};

struct PMSG_ILLUSION_TEMPLE_BATTLE_INFO
{
	WORD index;
	BYTE map;
	BYTE x;
	BYTE y;
};

struct PMSG_ILLUSION_TEMPLE_ADD_SKILL_SEND
{
	PSBMSG_HEAD header; // C1:BF:02
	BYTE result;
	BYTE skill[2];
	WORD index;
	WORD target;
};

struct PMSG_ILLUSION_TEMPLE_USER_COUNT_SEND
{
	PSBMSG_HEAD header; // C1:BF:03
	BYTE count[MAX_IT_LEVEL];
};

struct PMSG_ILLUSION_TEMPLE_REWARD_SCORE_SEND
{
	PSBMSG_HEAD header; // C1:BF:04
	BYTE score[2];
	BYTE count;
};

struct PMSG_ILLUSION_TEMPLE_REWARD_SCORES8_SEND
{
	PSBMSG_HEAD header;
	BYTE btAlliedPoint;
	BYTE btIllusionPoint;
	BYTE btUserCount;
	BYTE byWinTeamIndex;
};

struct PMSG_ILLUSION_TEMPLE_REWARD_SCORE
{
	char name[10];
	BYTE map;
	BYTE team;
	BYTE Class;
	DWORD Experience;
};

struct PMSG_ILLUSION_TEMPLE_REWARD_SCORES8
{
	char name[10];
	BYTE map;
	BYTE team;
	BYTE Class;
	BYTE btUserKillCount;
};

struct PMSG_ILLUSION_TEMPLE_KILL_POINT_SEND
{
	PSBMSG_HEAD header; // C1:BF:06
	BYTE KillPoint;
};

struct PMSG_ILLUSION_TEMPLE_DEL_SKILL_SEND
{
	PSBMSG_HEAD header; // C1:BF:07
	BYTE skill[2];
	WORD index;
};

struct PMSG_ILLUSION_TEMPLE_EVENT_ITEM_OWNER_SEND
{
	PSBMSG_HEAD header; // C1:BF:08
	WORD index;
	char name[10];
#if (GAMESERVER_UPDATE >= 803)
	BYTE byGet;
#endif
};

struct PMSG_ILLUSION_TEMPLE_CHANGE_SCENARIO_SEND
{
	PSBMSG_HEAD header; // C1:BF:09
	BYTE level;
	BYTE state;
};

struct PMSG_ILLUSION_TEMPLE_STONEINFO
{
	PSBMSG_HEAD header;
	BYTE byMapTagIndex;
	WORD wOccupiedStoneType;
	BYTE byOccupiedStoneState;
	int nNpcIndex;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct ILLUSION_TEMPLE_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct ILLUSION_TEMPLE_USER
{
	int Index;
	int Team;
	int Point;
	int Score;
	int RewardExperience;
};

struct ILLUSION_TEMPLE_LEVEL
{
	int Level;
	int State;
	int Map;
	int RemainTime;
	int TargetTime;
	int TickCount;
	int EnterEnabled;
	int MinutesLeft;
	int TimeCount;
	int Score[MAX_IT_TEAM];
	int EventItemSerial;
	int EventItemNumber;
	int EventItemOwner;
	int EventItemLevel;
	int PartyNumber[MAX_IT_TEAM];
	int WinnerTeam;
	int TeamRewardExperience[MAX_IT_TEAM];
	ILLUSION_TEMPLE_USER User[MAX_IT_USER];
	int StoneStatueIndex;
	int MonsterIndex[MAX_IT_MONSTER];
};

class CIllusionTemple
{
public:
	CIllusionTemple();
	virtual ~CIllusionTemple();
	void Init();
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void ProcState_EMPTY(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void ProcState_STAND(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void ProcState_START(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void ProcState_CLEAN(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void SetState(ILLUSION_TEMPLE_LEVEL* lpLevel,int state);
	void SetState_BLANK(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void SetState_EMPTY(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void SetState_STAND(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void SetState_START(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void SetState_CLEAN(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void CheckSync(ILLUSION_TEMPLE_LEVEL* lpLevel);
	int GetState(int level);
	int GetRemainTime(int level);
	int GetEnterEnabled(int level);
	int GetEnteredUserCount(int level);
	bool AddUser(ILLUSION_TEMPLE_LEVEL* lpLevel,int aIndex);
	bool DelUser(ILLUSION_TEMPLE_LEVEL* lpLevel,int aIndex);
	ILLUSION_TEMPLE_USER* GetUser(ILLUSION_TEMPLE_LEVEL* lpLevel,int aIndex);
	void CleanUser(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void ClearUser(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void CheckUser(ILLUSION_TEMPLE_LEVEL* lpLevel);
	int GetUserCount(ILLUSION_TEMPLE_LEVEL* lpLevel,int team);
	int GetUserJoinTeam(LPOBJ lpObj);
	int GetUserAbleLevel(LPOBJ lpObj);
	bool GetUserRespawnLocation(LPOBJ lpObj,int* gate,int* map,int* x,int* y,int* dir,int* level);
	void SearchUserDropEventItem(int aIndex);
	void SearchUserDeleteEventItem(int aIndex);
	void GiveUserRewardExperience(ILLUSION_TEMPLE_LEVEL* lpLevel);
	bool AddMonster(ILLUSION_TEMPLE_LEVEL* lpLevel,int aIndex);
	bool DelMonster(ILLUSION_TEMPLE_LEVEL* lpLevel,int aIndex);
	int* GetMonster(ILLUSION_TEMPLE_LEVEL* lpLevel,int aIndex);
	void CleanMonster(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void ClearMonster(ILLUSION_TEMPLE_LEVEL* lpLevel);
	int GetMonsterCount(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void SetStoneStatue(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void SetMonster(ILLUSION_TEMPLE_LEVEL* lpLevel,int MonsterClass);
	void NpcStoneStatue(LPOBJ lpNpc,LPOBJ lpObj);
	void NpcYellowStorage(LPOBJ lpNpc,LPOBJ lpObj);
	void NpcBlueStorage(LPOBJ lpNpc,LPOBJ lpObj);
	void NpcMirageTheMummy(LPOBJ lpNpc,LPOBJ lpObj);
	bool CheckEventItemSerial(int map,CMapItem* lpItem);
	void SetEventItemSerial(int map,int index,int serial);
	void DropEventItem(int map,int number,int aIndex);
	void GetEventItem(int map,int aIndex,CMapItem* lpItem);
	void DestroyEventItem(int map,CMapItem* lpItem);
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	bool CheckPlayerTarget(LPOBJ lpObj,LPOBJ lpTarget);
	void NoticeSendToAll(ILLUSION_TEMPLE_LEVEL* lpLevel,int type,char* message,...);
	void DataSendToAll(ILLUSION_TEMPLE_LEVEL* lpLevel,BYTE* lpMsg,int size);
	void CGIllusionTempleEnterRecv(PMSG_ILLUSION_TEMPLE_ENTER_RECV* lpMsg,int aIndex);
	void CGIllusionTempleSkillRecv(PMSG_ILLUSION_TEMPLE_SKILL_RECV* lpMsg,int aIndex);
	void CGIllusionTempleButtonClickRecv(int aIndex);
	void GCIllusionTempleBattleInfoSend(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void GCIllusionTempleAddSkillSend(int aIndex,int result,int skill,int bIndex);
	void GCIllusionTempleRewardScoreSend(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void GCIllusionTempleKillPointSend(int aIndex,int point);
	void GCIllusionTempleDelSkillSend(int aIndex,int skill);
	void GCIllusionTempleEventItemOwnerSend(ILLUSION_TEMPLE_LEVEL* lpLevel);
	void GCIllusionTempleEventItemOwnerS8Send(ILLUSION_TEMPLE_LEVEL* lpLevel,int Index, int byGet);
	void GCIllusionTempleChangeScenarioSend(ILLUSION_TEMPLE_LEVEL* lpLevel,int state,int aIndex);
	void GCIllusionTempleStoneInfoSend(ILLUSION_TEMPLE_LEVEL* lpLevel,int type,int state);
	void StartIT();
private:
	ILLUSION_TEMPLE_LEVEL m_IllusionTempleLevel[MAX_IT_LEVEL];
	int m_WarningTime;
	int m_NotifyTime;
	int m_EventTime;
	int m_CloseTime;
	std::vector<ILLUSION_TEMPLE_START_TIME> m_IllusionTempleStartTime;
	int m_IllusionTempleExperienceTable[MAX_IT_LEVEL][2];
	int m_IllusionTempleReward[MAX_IT_LEVEL][3];
};

extern CIllusionTemple gIllusionTemple;

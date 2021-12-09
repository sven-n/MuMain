// ReiDoMu.cpp: Evento rei do MU by louis || www.jogandomu.com.br
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"
#include "Protocol.h"

#define MAX_REI_GUILD 20
#define MAX_REI_CHAR 100

enum eReiState
{
	REI_STATE_BLANK = 0,
	REI_STATE_EMPTY = 1,
	REI_STATE_START = 2,
};

struct REI_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};


//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_RANKING_KING_GUILD_SAVE_SEND
{
	PBMSG_HEAD header; // C1:74
	char name[9];
	DWORD score;
};

struct SDHP_RANKING_KING_PLAYER_SAVE_SEND
{
	PBMSG_HEAD header; // C1:75
	WORD index;
	char account[11];
	char name[11];
	DWORD score;
};


struct REI_GUILD
{
	void Reset() // OK
	{
		this->Number = -1;
		this->Owner = 0;
		this->Time = 0;
	}

	int Number;
	int Owner;
	int Time;
};

struct REI_CHAR
{
	void Reset() // OK
	{
		memset(this->Name,0,sizeof(this->Name));
		this->Index = -1;
		this->Times = 0;
		this->Rank = 0;
	}

	int Index;
	char Name[11];
	int Times;
	int Rank;
};

class CReiDoMU
{
public:
	CReiDoMU();
	virtual ~CReiDoMU();
	void Init();
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK();
	void ProcState_EMPTY();
	void ProcState_START();
	void SetState(int state);
	void SetState_BLANK();
	void SetState_EMPTY();
	void SetState_START();
	void CheckSync();
	void Clear();
    void CommandReiDoMU(LPOBJ lpObj,char* arg);
	void SetGuild(int gIndex);
	void AddGuild(int gIndex);
	bool GetGuild(int gIndex);
	void SetChar(int cIndex);
	void AddChar(int cIndex);
	bool GetChar(int cIndex);
	void CheckChar();
	void SetGuildTop();
	void AddTime();
	void CalcUserRank(); 
	int m_Active;
	void GDRankingKingGuildSaveSend(char* name,DWORD Score);
	void GDRankingKingPlayerSaveSend(int aIndex,DWORD Score);
	void StartKing();
private:
	int m_State;
	int m_RemainTime;
	int m_TargetTime;
	int m_TickCount;
	int m_WarningTime;
	int m_EventTime;
	int MinutesLeft;
	int WinnerNumber;
	int WinnerTime;
	int m_RemainTimeWinner;
	int rank1;
	int rank2;
	int rank3;
	int rank4;
	int rank5;
	int GRewardValue1;
	int GRewardValue2;
	int GRewardValue3;
	int CRewardValue1;
	int CRewardValue2;
	int CRewardValue3;
    char m_GuildOwner[9];
	std::vector<REI_START_TIME> m_ReiDoMUStartTime;
	REI_GUILD Guild[MAX_REI_GUILD];
	REI_CHAR Char[MAX_REI_CHAR];
};

extern CReiDoMU gReiDoMU;
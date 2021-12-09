// ReiDoMU.cpp: implementation of the ReiDoMU class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ReiDoMU.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "SocketManager.h"

CReiDoMU gReiDoMU;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CReiDoMU::CReiDoMU() // OK
{

}

CReiDoMU::~CReiDoMU() // OK
{

}

void CReiDoMU::GDRankingKingGuildSaveRecv(SDHP_RANKING_KING_GUILD_SAVE_RECV* lpMsg) // OK
{
	if(gQueryManager.ExecQuery("SELECT Name FROM RankingKingGuild WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO RankingKingGuild (Name,Score,Score_semanal) VALUES ('%s',%d,%d)",lpMsg->name,lpMsg->score,lpMsg->score);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("UPDATE RankingKingGuild SET Score=Score+%d,Score_semanal=Score_semanal+%d WHERE Name='%s'",lpMsg->score,lpMsg->score,lpMsg->name);
		gQueryManager.Close();
	}
}

void CReiDoMU::GDRankingKingPlayerSaveRecv(SDHP_RANKING_KING_PLAYER_SAVE_RECV* lpMsg) // OK
{
	if(gQueryManager.ExecQuery("SELECT Name FROM RankingKingPlayer WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO RankingKingPlayer (Name,Score,Score_semanal) VALUES ('%s',%d,%d)",lpMsg->name,lpMsg->score,lpMsg->score);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("UPDATE RankingKingPlayer SET Score=Score+%d,Score_semanal=Score_semanal+%d WHERE Name='%s'",lpMsg->score,lpMsg->score,lpMsg->name);
		gQueryManager.Close();
	}
}


// GensSystem.cpp: implementation of the CGensSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GensSystem.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "SocketManager.h"

CGensSystem gGensSystem;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGensSystem::CGensSystem() // OK
{

}

CGensSystem::~CGensSystem() // OK
{

}

void CGensSystem::GDGensSystemInsertRecv(SDHP_GENS_SYSTEM_INSERT_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=501)

	SDHP_GENS_SYSTEM_INSERT_SEND pMsg;

	pMsg.header.set(0x11,0x00,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	pMsg.GensFamily = 0;

	pMsg.GensRank = 0;

	pMsg.GensContribution = 0;

	if(gQueryManager.ExecQuery("INSERT INTO Gens_Rank (Name,Family,Contribution) VALUES ('%s',%d,0)",lpMsg->name,lpMsg->GensFamily) == 0)
	{
		gQueryManager.Close();

		pMsg.result = 1;
	}
	else
	{
		pMsg.GensFamily = lpMsg->GensFamily;

		gQueryManager.Close();

		if(gQueryManager.ExecQuery("EXEC WZ_GetCharacterGensInfo '%s',%d",lpMsg->name,lpMsg->GensFamily) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			pMsg.result = 1;
		}
		else
		{
			pMsg.GensRank = gQueryManager.GetAsInteger("Rank");

			pMsg.GensContribution = gQueryManager.GetAsInteger("Contribution");

			gQueryManager.Close();
		}
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::GDGensSystemDeleteRecv(SDHP_GENS_SYSTEM_DELETE_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=501)

	SDHP_GENS_SYSTEM_DELETE_SEND pMsg;

	pMsg.header.set(0x11,0x01,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	if(gQueryManager.ExecQuery("DELETE FROM Gens_Rank WHERE Name='%s'",lpMsg->name) == 0)
	{
		gQueryManager.Close();

		pMsg.result = 1;
	}
	else
	{
		gQueryManager.Close();

		pMsg.result = 0;
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	this->DGGensSystemCreateSend(lpMsg->GensFamily,(lpMsg->GensRank+1),0xFFFFFFFF);

	#endif
}

void CGensSystem::GDGensSystemMemberRecv(SDHP_GENS_SYSTEM_MEMBER_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=501)

	SDHP_GENS_SYSTEM_MEMBER_SEND pMsg;

	pMsg.header.set(0x11,0x02,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	pMsg.GensFamily = 0;

	pMsg.GensRank = 0;

	pMsg.GensContribution = 0;

	if(gQueryManager.ExecQuery("SELECT Family FROM Gens_Rank WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.result = 1;
	}
	else
	{
		pMsg.GensFamily = gQueryManager.GetAsInteger("Family");

		gQueryManager.Close();

		if(gQueryManager.ExecQuery("EXEC WZ_GetCharacterGensInfo '%s',%d",lpMsg->name,pMsg.GensFamily) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			pMsg.result = 1;
		}
		else
		{
			pMsg.GensRank = gQueryManager.GetAsInteger("Rank");

			pMsg.GensContribution = gQueryManager.GetAsInteger("Contribution");

			gQueryManager.Close();
		}
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::GDGensSystemUpdateRecv(SDHP_GENS_SYSTEM_UPDATE_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=501)

	SDHP_GENS_SYSTEM_UPDATE_SEND pMsg;

	pMsg.header.set(0x11,0x03,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	pMsg.GensFamily = 0;

	pMsg.GensRank = 0;

	pMsg.GensContribution = 0;

	if(gQueryManager.ExecQuery("UPDATE Gens_Rank SET Contribution=%d WHERE Name='%s'",lpMsg->GensContribution,lpMsg->name) == 0)
	{
		gQueryManager.Close();

		pMsg.result = 1;
	}
	else
	{
		pMsg.GensFamily = lpMsg->GensFamily;

		gQueryManager.Close();

		if(gQueryManager.ExecQuery("EXEC WZ_GetCharacterGensInfo '%s',%d",lpMsg->name,lpMsg->GensFamily) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			pMsg.result = 1;
		}
		else
		{
			pMsg.GensRank = gQueryManager.GetAsInteger("Rank");

			pMsg.GensContribution = gQueryManager.GetAsInteger("Contribution");

			gQueryManager.Close();
		}
	}

	if(pMsg.result != 0)
	{
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(pMsg.GensRank == lpMsg->GensRank)
	{
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(pMsg.GensRank != lpMsg->GensRank && pMsg.GensRank < lpMsg->GensRank)
	{
		this->DGGensSystemCreateSend(lpMsg->GensFamily,pMsg.GensRank,lpMsg->GensRank);
		return;
	}

	if(pMsg.GensRank != lpMsg->GensRank && pMsg.GensRank > lpMsg->GensRank)
	{
		this->DGGensSystemCreateSend(lpMsg->GensFamily,lpMsg->GensRank,pMsg.GensRank);
		return;
	}

	#endif
}

void CGensSystem::GDGensSystemRewardRecv(SDHP_GENS_SYSTEM_REWARD_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=501)

	SDHP_GENS_SYSTEM_REWARD_SEND pMsg;

	pMsg.header.set(0x11,0x04,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	pMsg.GensFamily = 0;

	pMsg.GensRank = 0;

	pMsg.GensRewardStatus = 0;

	if(gQueryManager.ExecQuery("SELECT Rank,Status FROM Gens_Reward WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.result = 1;
	}
	else
	{
		pMsg.GensFamily = lpMsg->GensFamily;

		pMsg.GensRank = gQueryManager.GetAsInteger("Rank");

		pMsg.GensRewardStatus = gQueryManager.GetAsInteger("Status");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::GDGensSystemRewardSaveRecv(SDHP_GENS_SYSTEM_REWARD_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=501)

	if(gQueryManager.ExecQuery("SELECT Name FROM Gens_Reward WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO Gens_Reward (Name,Rank,Status) VALUES ('%s',%d,%d)",lpMsg->name,lpMsg->GensRank,lpMsg->GensRewardStatus);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("UPDATE Gens_Reward SET Status=%d WHERE Name='%s'",lpMsg->GensRewardStatus,lpMsg->name);
		gQueryManager.Close();
	}

	#endif
}

void CGensSystem::DGGensSystemCreateSend(DWORD GensFamily,DWORD GensRankStart,DWORD GensRankFinal) // OK
{
	#if(DATASERVER_UPDATE>=501)

	SDHP_GENS_SYSTEM_CREATE_SEND pMsg;

	pMsg.header.set(0x11,0x70,sizeof(pMsg));

	pMsg.GensFamily = GensFamily;

	pMsg.GensRankStart = GensRankStart;

	pMsg.GensRankFinal = GensRankFinal;

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			gSocketManager.DataSend(n,(BYTE*)&pMsg,pMsg.header.size);
		}
	}

	#endif
}

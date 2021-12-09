// GuildManager.cpp: implementation of the CGuildManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GuildManager.h"
#include "ESProtocol.h"
#include "QueryManager.h"
#include "Util.h"

CGuildManager gGuildManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGuildManager::CGuildManager()
{

}

CGuildManager::~CGuildManager()
{

}

void CGuildManager::Init()
{
	this->vGuildList.clear();

	if(gQueryManager.ExecQuery("SELECT * FROM Guild") != 0)
	{
		while(gQueryManager.Fetch() != SQL_NO_DATA)
		{
			GUILD_INFO GuildInfo;

			GuildInfo.Clear();

			gQueryManager.GetAsString("G_Name",GuildInfo.szName,sizeof(GuildInfo.szName));

			gQueryManager.GetAsBinary("G_Mark",GuildInfo.arMark,sizeof(GuildInfo.arMark));

			GuildInfo.dwScore = gQueryManager.GetAsInteger("G_Score");

			gQueryManager.GetAsString("G_Master",GuildInfo.szMaster,sizeof(GuildInfo.szMaster));

			gQueryManager.GetAsString("G_Notice",GuildInfo.szNotice,sizeof(GuildInfo.szNotice));

			GuildInfo.dwNumber = gQueryManager.GetAsInteger("Number");

			GuildInfo.btType = gQueryManager.GetAsInteger("G_Type");

			GuildInfo.dwRivalNumber = gQueryManager.GetAsInteger("G_Rival");

			GuildInfo.dwUnionNumber = gQueryManager.GetAsInteger("G_Union");

			this->vGuildList.push_back(GuildInfo);
		}
	}

	gQueryManager.Close();

	if(gQueryManager.ExecQuery("SELECT * FROM GuildMember") != 0)
	{
		while(gQueryManager.Fetch() != SQL_NO_DATA)
		{
			char GuildName[9] = {0};

			GUILD_MEMBER_INFO GuildMemberInfo;

			GuildMemberInfo.Clear();

			gQueryManager.GetAsString("Name",GuildMemberInfo.szGuildMember,sizeof(GuildMemberInfo.szGuildMember));

			gQueryManager.GetAsString("G_Name",GuildName,sizeof(GuildName));

			GuildMemberInfo.btStatus = gQueryManager.GetAsInteger("G_Status");

			GUILD_INFO* lpGuildInfo = this->GetGuildInfo(GuildName);

			if(lpGuildInfo != 0)
			{
				if(_stricmp(lpGuildInfo->szMaster,GuildMemberInfo.szGuildMember) == 0)
				{
					lpGuildInfo->arGuildMember[0] = GuildMemberInfo;
				}
				else
				{
					for(int n=1;n < MAX_GUILD_MEMBER;n++)
					{
						if(lpGuildInfo->arGuildMember[n].IsEmpty() != 0)
						{
							lpGuildInfo->arGuildMember[n] = GuildMemberInfo;
							break;
						}
					}
				}
			}
		}
	}

	gQueryManager.Close();
}

GUILD_INFO* CGuildManager::GetGuildInfo(char* szName)
{
	for(std::vector<GUILD_INFO>::iterator it=this->vGuildList.begin();it != this->vGuildList.end();it++)
	{
		if(it->szName[0] == szName[0])
		{
			if(_stricmp(it->szName,szName) == 0)
			{
				return it._Ptr;
			}
		}
	}

	return 0;
}

GUILD_INFO* CGuildManager::GetGuildInfo(DWORD dwNumber)
{
	for(std::vector<GUILD_INFO>::iterator it=this->vGuildList.begin();it != this->vGuildList.end();it++)
	{
		if(it->dwNumber == dwNumber)
		{
			return it._Ptr;
		}
	}

	return 0;
}

GUILD_INFO* CGuildManager::GetMemberGuildInfo(char* szGuildMember)
{
	for(std::vector<GUILD_INFO>::iterator it=this->vGuildList.begin();it != this->vGuildList.end();it++)
	{
		for(int n=0;n < MAX_GUILD_MEMBER;n++)
		{
			if(it->arGuildMember[n].szGuildMember[0] == szGuildMember[0])
			{
				if(_stricmp(it->arGuildMember[n].szGuildMember,szGuildMember) == 0)
				{
					return it._Ptr;
				}
			}
		}
	}

	return 0;
}

GUILD_MEMBER_INFO* CGuildManager::GetGuildMemberInfo(char* szGuildMember)
{
	for(std::vector<GUILD_INFO>::iterator it=this->vGuildList.begin();it != this->vGuildList.end();it++)
	{
		for(int n=0;n < MAX_GUILD_MEMBER;n++)
		{
			if(it->arGuildMember[n].szGuildMember[0] == szGuildMember[0])
			{
				if(_stricmp(it->arGuildMember[n].szGuildMember,szGuildMember) == 0)
				{
					return &it->arGuildMember[n];
				}
			}
		}
	}

	return 0;
}

BOOL CGuildManager::CheckGuildOnCS(char* szGuildName)
{
	if(gQueryManager.ExecQuery("WZ_CS_CheckSiegeGuildList '%s'",szGuildName) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		return 0;
	}
	else
	{
		gQueryManager.Close();

		return gQueryManager.GetResult(0);
	}
}

void CGuildManager::ConnectMember(char* szGuildMember,WORD btServer)
{
	GUILD_MEMBER_INFO* lpGuildMemberInfo = this->GetGuildMemberInfo(szGuildMember);

	if(lpGuildMemberInfo != 0){lpGuildMemberInfo->btServer = btServer;}
}

void CGuildManager::DisconnectMember(char* szGuildMember)
{
	GUILD_MEMBER_INFO* lpGuildMemberInfo = this->GetGuildMemberInfo(szGuildMember);

	if(lpGuildMemberInfo != 0){lpGuildMemberInfo->btServer = 0xFFFF;}
}

BYTE CGuildManager::AddGuild(int index,char* szGuildName,char* szMasterName,BYTE* lpMark,BYTE btType)
{
	GUILD_INFO* lpGuildInfo = this->GetGuildInfo(szGuildName);

	if(lpGuildInfo != 0)
	{
		return 3;
	}

	if(CheckTextSyntax(szGuildName,strlen(szGuildName)) == 0)
	{
		return 4;
	}

	if(gQueryManager.ExecQuery("WZ_GuildCreate '%s','%s'",szGuildName,szMasterName) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		return 0;
	}
	else
	{
		if(gQueryManager.GetResult(0) == 0)
		{
			gQueryManager.Close();

			gQueryManager.BindParameterAsBinary(1,lpMark,32);

			gQueryManager.ExecQuery("UPDATE Guild SET G_Mark=?,G_Type=%d WHERE G_Name='%s'",btType,szGuildName);

			gQueryManager.Close();

			gQueryManager.ExecQuery("UPDATE GuildMember SET G_Status=%d WHERE Name='%s'",0x80,szMasterName);

			gQueryManager.Close();

			GUILD_INFO GuildInfo;

			GuildInfo.Clear();

			gQueryManager.ExecQuery("SELECT Number FROM Guild WHERE G_Name='%s'",szGuildName);

			gQueryManager.Fetch();

			GuildInfo.dwNumber = gQueryManager.GetAsInteger("Number");

			gQueryManager.Close();

			memcpy(GuildInfo.szName,szGuildName,sizeof(GuildInfo.szName));

			memcpy(GuildInfo.szMaster,szMasterName,sizeof(GuildInfo.szMaster));

			GuildInfo.btType = btType;

			memcpy(GuildInfo.arMark,lpMark,sizeof(GuildInfo.arMark));

			memcpy(GuildInfo.arGuildMember[0].szGuildMember,szMasterName,sizeof(GuildInfo.arGuildMember[0].szGuildMember));

			GuildInfo.arGuildMember[0].btStatus = 0x80;

			GuildInfo.arGuildMember[0].btServer = 0xFFFF;

			this->vGuildList.push_back(GuildInfo);

			return 1;
		}
		else
		{
			gQueryManager.Close();

			return 2;
		}
	}
}

BYTE CGuildManager::DelGuild(int index,char* szGuildName)
{
	GUILD_INFO* lpGuildInfo = this->GetGuildInfo(szGuildName);

	if(lpGuildInfo == 0)
	{
		return 3;
	}

	if(this->CheckGuildOnCS(szGuildName) != 0)
	{
		return 2;
	}

	this->DelGuildRelationship(index,lpGuildInfo->dwNumber,1);

	this->DelGuildRelationship(index,lpGuildInfo->dwNumber,2);

	lpGuildInfo->Clear();

	gQueryManager.ExecQuery("DELETE FROM Guild WHERE G_Name='%s'",szGuildName);

	gQueryManager.Close();

	gQueryManager.ExecQuery("DELETE FROM GuildMember WHERE G_Name='%s'",szGuildName);

	gQueryManager.Close();

	gQueryManager.ExecQuery("DELETE FROM WarehouseGuild WHERE Guild='%s'",szGuildName);

	gQueryManager.Close();

	return 1;
}

BYTE CGuildManager::AddGuildMember(int index,char* szGuildName,char* szGuildMember,BYTE btStatus,WORD btServer)
{
	GUILD_INFO* lpGuildInfo = this->GetGuildInfo(szGuildName);

	if(lpGuildInfo == 0)
	{
		return 0;
	}

	if(this->GetGuildMemberInfo(szGuildMember) != 0)
	{
		return 3;
	}

	if(gQueryManager.ExecQuery("INSERT INTO GuildMember (Name,G_Name,G_Level,G_Status) VALUES ('%s','%s',%d,%d)",szGuildMember,szGuildName,1,btStatus) == 0)
	{
		gQueryManager.Close();

		return 4;
	}
	else
	{
		gQueryManager.Close();

		GUILD_MEMBER_INFO GuildMemberInfo;

		GuildMemberInfo.Clear();

		memcpy(GuildMemberInfo.szGuildMember,szGuildMember,sizeof(GuildMemberInfo.szGuildMember));

		GuildMemberInfo.btStatus = btStatus;

		GuildMemberInfo.btServer = btServer;

		for(int n=1;n < MAX_GUILD_MEMBER;n++)
		{
			if(lpGuildInfo->arGuildMember[n].IsEmpty() != 0)
			{
				lpGuildInfo->arGuildMember[n] = GuildMemberInfo;
				return 1;
			}
		}

		return 4;
	}
}

BYTE CGuildManager::DelGuildMember(int index,char* szGuildMember)
{
	GUILD_MEMBER_INFO* lpGuildMemberInfo = this->GetGuildMemberInfo(szGuildMember);

	if(lpGuildMemberInfo == 0){return 3;}

	gQueryManager.ExecQuery("DELETE FROM GuildMember WHERE Name='%s'",szGuildMember);

	gQueryManager.Close();

	lpGuildMemberInfo->Clear();

	return 1;
}

BYTE CGuildManager::AddGuildRelationship(int index,DWORD dwSourceGuild,DWORD dwTargetGuild,BYTE btRelationType)
{
	GUILD_INFO* lpSourceGuild = this->GetGuildInfo(dwSourceGuild);

	GUILD_INFO* lpTargetGuild = this->GetGuildInfo(dwTargetGuild);

	if(lpSourceGuild == 0)
	{
		return 0;
	}

	if(lpTargetGuild == 0)
	{
		return 0;
	}

	if(this->CheckGuildOnCS(lpSourceGuild->szName) != 0)
	{
		return 16;
	}

	if(this->CheckGuildOnCS(lpTargetGuild->szName) != 0)
	{
		return 16;
	}

	DWORD UnionGuildNumber[MAX_GUILD_UNION];

	DWORD RivalGuildNumber[MAX_GUILD_RIVAL];

	DWORD UnionGuildAmount = 0;

	DWORD RivalGuildAmount = 0;

	switch(btRelationType)
	{
		case 1:
			if(lpSourceGuild->dwUnionNumber > 0)
			{
				return 0;
			}

			if(lpSourceGuild->dwRivalNumber > 0)
			{
				return 0;
			}

			if((UnionGuildAmount=this->GetUnionList(lpTargetGuild->dwNumber,UnionGuildNumber)) >= MAX_GUILD_UNION)
			{
				return 0;
			}

			if(gQueryManager.ExecQuery("UPDATE Guild SET G_Union=%d WHERE Number=%d",lpTargetGuild->dwNumber,lpTargetGuild->dwNumber) == 0)
			{
				gQueryManager.Close();
				return 0;
			}
			else
			{
				gQueryManager.Close();
				lpTargetGuild->dwUnionNumber = lpTargetGuild->dwNumber;
			}

			if(gQueryManager.ExecQuery("UPDATE Guild SET G_Union=%d WHERE Number=%d",lpTargetGuild->dwNumber,lpSourceGuild->dwNumber) == 0)
			{
				gQueryManager.Close();
				return 0;
			}
			else
			{
				gQueryManager.Close();
				lpSourceGuild->dwUnionNumber = lpTargetGuild->dwNumber;
			}

			DGRelationShipListRecv(index,lpTargetGuild->dwNumber,1);

			UnionGuildAmount = this->GetUnionList(lpTargetGuild->dwNumber,UnionGuildNumber);

			DGRelationShipNotificationRecv(index,0,UnionGuildAmount,(int*)UnionGuildNumber);

			return 1;
		case 2:
			if(lpSourceGuild->dwRivalNumber > 0)
			{
				return 0;
			}

			if(lpTargetGuild->dwRivalNumber > 0)
			{
				return 0;
			}

			if(lpSourceGuild->dwUnionNumber > 0 && lpSourceGuild->dwUnionNumber != lpSourceGuild->dwNumber)
			{
				return 0;
			}

			if(lpTargetGuild->dwUnionNumber > 0 && lpTargetGuild->dwUnionNumber != lpTargetGuild->dwNumber)
			{
				return 0;
			}

			if(gQueryManager.ExecQuery("UPDATE Guild SET G_Rival=%d WHERE Number=%d",lpSourceGuild->dwNumber,lpTargetGuild->dwNumber) == 0)
			{
				gQueryManager.Close();
				return 0;
			}
			else
			{
				gQueryManager.Close();
				lpTargetGuild->dwRivalNumber = lpSourceGuild->dwNumber;
			}

			if(gQueryManager.ExecQuery("UPDATE Guild SET G_Rival=%d WHERE Number=%d",lpTargetGuild->dwNumber,lpSourceGuild->dwNumber) == 0)
			{
				gQueryManager.Close();
				return 0;
			}
			else
			{
				gQueryManager.Close();
				lpSourceGuild->dwRivalNumber = lpTargetGuild->dwNumber;
			}

			DGRelationShipListRecv(index,lpSourceGuild->dwNumber,2);

			DGRelationShipListRecv(index,lpTargetGuild->dwNumber,2);

			RivalGuildNumber[RivalGuildAmount++] = lpSourceGuild->dwNumber;

			RivalGuildNumber[RivalGuildAmount++] = lpTargetGuild->dwNumber;

			DGRelationShipNotificationRecv(index,0,RivalGuildAmount,(int*)RivalGuildNumber);

			return 1;
	}

	return 0;
}

BYTE CGuildManager::DelGuildRelationship(int index,DWORD dwSourceGuild,BYTE btRelationType)
{
	GUILD_INFO* lpGuildInfo = this->GetGuildInfo(dwSourceGuild);

	if(lpGuildInfo == 0)
	{
		return 0;
	}

	GUILD_INFO* lpUnionInfo = 0;

	GUILD_INFO* lpRivalInfo = 0;

	DWORD UnionGuildNumber[MAX_GUILD_UNION];

	DWORD RivalGuildNumber[MAX_GUILD_RIVAL];

	DWORD UnionGuildAmount = 0;

	DWORD RivalGuildAmount = 0;

	switch(btRelationType)
	{
		case 1:
			if(this->CheckGuildOnCS(lpGuildInfo->szName) != 0)
			{
				return 16;
			}

			if(lpGuildInfo->dwNumber == lpGuildInfo->dwUnionNumber)
			{
				return 0;
			}

			if((lpUnionInfo=this->GetGuildInfo(lpGuildInfo->dwUnionNumber)) == 0)
			{
				return 0;
			}

			if((UnionGuildAmount=this->GetUnionList(lpUnionInfo->dwNumber,UnionGuildNumber)) == 0)
			{
				return 0;
			}

			if(UnionGuildAmount == 2)
			{
				if(gQueryManager.ExecQuery("UPDATE Guild SET G_Union=0 WHERE Number IN (%d,%d)",UnionGuildNumber[0],UnionGuildNumber[1]) == 0)
				{
					gQueryManager.Close();
					return 0;
				}
				else
				{
					gQueryManager.Close();
					lpGuildInfo->dwUnionNumber = 0;
					lpUnionInfo->dwUnionNumber = 0;
					DGRelationShipListRecv(index,lpUnionInfo->dwNumber,1);
					DGRelationShipNotificationRecv(index,16,UnionGuildAmount,(int*)UnionGuildNumber);
					return 1;
				}
			}
			else
			{
				if(gQueryManager.ExecQuery("UPDATE Guild SET G_Union=0 WHERE Number=%d",lpGuildInfo->dwNumber) == 0)
				{
					gQueryManager.Close();
					return 0;
				}
				else
				{
					gQueryManager.Close();
					lpGuildInfo->dwUnionNumber = 0;
					DGRelationShipListRecv(index,lpUnionInfo->dwNumber,1);
					DGRelationShipNotificationRecv(index,0,UnionGuildAmount,(int*)UnionGuildNumber);
					return 1;
				}
			}

			break;
		case 2:
			if(this->CheckGuildOnCS(lpGuildInfo->szName) != 0)
			{
				return 16;
			}

			if((lpRivalInfo=this->GetGuildInfo(lpGuildInfo->dwRivalNumber)) == 0)
			{
				return 0;
			}

			if(gQueryManager.ExecQuery("UPDATE Guild SET G_Rival=0 WHERE Number IN (%d,%d)",lpGuildInfo->dwNumber,lpRivalInfo->dwNumber) == 0)
			{
				gQueryManager.Close();
				return 0;
			}
			else
			{
				gQueryManager.Close();
				lpGuildInfo->dwRivalNumber = 0;
				lpRivalInfo->dwRivalNumber = 0;
				DGRelationShipListRecv(index,lpGuildInfo->dwNumber,2);
				DGRelationShipListRecv(index,lpRivalInfo->dwNumber,2);
				RivalGuildNumber[RivalGuildAmount++] = lpGuildInfo->dwNumber;
				RivalGuildNumber[RivalGuildAmount++] = lpRivalInfo->dwNumber;
				DGRelationShipNotificationRecv(index,0,RivalGuildAmount,(int*)RivalGuildNumber);
				return 1;
			}

			break;
	}

	return 0;
}

BYTE CGuildManager::SetGuildRelationship(int index,char* szGuildMember,char* szGuildMaster)
{
	GUILD_INFO* lpSourceGuild = this->GetGuildInfo(szGuildMember);

	GUILD_INFO* lpTargetGuild = this->GetGuildInfo(szGuildMaster);

	if(lpSourceGuild == 0)
	{
		return 0;
	}

	if(lpTargetGuild == 0)
	{
		return 0;
	}

	if(lpSourceGuild->dwUnionNumber != lpTargetGuild->dwNumber)
	{
		return 0;
	}

	if(lpTargetGuild->dwUnionNumber != lpTargetGuild->dwNumber)
	{
		return 0;
	}

	if(this->CheckGuildOnCS(lpSourceGuild->szName) != 0)
	{
		return 16;
	}

	if(this->CheckGuildOnCS(lpTargetGuild->szName) != 0)
	{
		return 16;
	}

	DWORD UnionGuildNumber[MAX_GUILD_UNION];

	DWORD UnionGuildAmount = this->GetUnionList(lpTargetGuild->dwNumber,UnionGuildNumber);

	if(UnionGuildAmount == 2)
	{
		if(gQueryManager.ExecQuery("UPDATE Guild SET G_Union=0 WHERE Number IN (%d,%d)",lpSourceGuild->dwNumber,lpTargetGuild->dwNumber) == 0)
		{
			gQueryManager.Close();

			return 0;
		}
		else
		{
			gQueryManager.Close();

			lpSourceGuild->dwUnionNumber = 0;

			lpTargetGuild->dwUnionNumber = 0;

			DGRelationShipListRecv(index,lpTargetGuild->dwNumber,1);

			DGRelationShipNotificationRecv(index,16,UnionGuildAmount,(int*)UnionGuildNumber);

			return 1;
		}
	}
	else
	{
		if(gQueryManager.ExecQuery("UPDATE Guild SET G_Union=0 WHERE Number=%d",lpSourceGuild->dwNumber) == 0)
		{
			gQueryManager.Close();

			return 0;
		}
		else
		{
			gQueryManager.Close();

			lpSourceGuild->dwUnionNumber = 0;

			DGRelationShipListRecv(index,lpTargetGuild->dwNumber,1);

			DGRelationShipNotificationRecv(index,0,UnionGuildAmount,(int*)UnionGuildNumber);

			return 1;
		}
	}
}

BYTE CGuildManager::SetGuildScore(char* szGuildName,DWORD dwScore)
{
	GUILD_INFO* lpGuildInfo = this->GetGuildInfo(szGuildName);

	if(lpGuildInfo == 0){return 0;}

	if(gQueryManager.ExecQuery("UPDATE Guild SET G_Score=%d WHERE G_Name='%s'",dwScore,szGuildName) == 0)
	{
		gQueryManager.Close();

		return 0;
	}
	else
	{
		gQueryManager.Close();

		lpGuildInfo->dwScore = dwScore;

		return 1;
	}
}

BYTE CGuildManager::SetGuildNotice(char* szGuildName,char* szNotice)
{
	GUILD_INFO* lpGuildInfo = this->GetGuildInfo(szGuildName);

	if(lpGuildInfo == 0){return 0;}

	gQueryManager.BindParameterAsString(1,szNotice,sizeof(lpGuildInfo->szNotice));

	if(gQueryManager.ExecQuery("UPDATE Guild SET G_Notice=? WHERE G_Name='%s'",szGuildName) == 0)
	{
		gQueryManager.Close();

		return 0;
	}
	else
	{
		gQueryManager.Close();

		memcpy(lpGuildInfo->szNotice,szNotice,sizeof(lpGuildInfo->szNotice));

		return 1;
	}
}

BYTE CGuildManager::SetGuildType(char* szGuildName,BYTE btType)
{
	GUILD_INFO* lpGuildInfo = this->GetGuildInfo(szGuildName);

	if(lpGuildInfo == 0){return 0;}

	if(gQueryManager.ExecQuery("UPDATE Guild SET G_Type=%d WHERE G_Name='%s'",btType,szGuildName) == 0)
	{
		gQueryManager.Close();

		return 0;
	}
	else
	{
		gQueryManager.Close();

		lpGuildInfo->btType = btType;

		return 1;
	}
}

BYTE CGuildManager::SetGuildMemberStatus(char* szGuildMember,BYTE btStatus)
{
	GUILD_MEMBER_INFO* lpGuildMemberInfo = this->GetGuildMemberInfo(szGuildMember);

	if(lpGuildMemberInfo == 0){return 0;}

	if(gQueryManager.ExecQuery("UPDATE GuildMember SET G_Status=%d WHERE Name='%s'",btStatus,szGuildMember) == 0)
	{
		gQueryManager.Close();

		return 0;
	}
	else
	{
		gQueryManager.Close();

		lpGuildMemberInfo->btStatus = btStatus;

		return 1;
	}
}

long CGuildManager::GetUnionList(DWORD dwUnionNumber,DWORD* lpUnionList)
{
	GUILD_INFO* lpGuildInfo = this->GetGuildInfo(dwUnionNumber);

	if(lpGuildInfo == 0)
	{
		return 0;
	}

	if(lpGuildInfo->dwUnionNumber == 0)
	{
		return 0;
	}

	long count = 0;

	lpUnionList[count++] = lpGuildInfo->dwNumber;

	for(std::vector<GUILD_INFO>::iterator it=this->vGuildList.begin();it != this->vGuildList.end();it++)
	{
		if(it->dwNumber != lpGuildInfo->dwNumber && it->dwUnionNumber == lpGuildInfo->dwNumber)
		{
			lpUnionList[count++] = it->dwNumber;
			if(count >= MAX_GUILD_UNION){break;}
		}
	}

	return count;
}

long CGuildManager::GetRivalList(DWORD dwRivalNumber,DWORD* lpRivalList)
{
	GUILD_INFO* lpGuildInfo = this->GetGuildInfo(dwRivalNumber);

	if(lpGuildInfo == 0)
	{
		return 0;
	}

	if(lpGuildInfo->dwRivalNumber == 0)
	{
		return 0;
	}

	long count = 0;

	lpRivalList[count++] = lpGuildInfo->dwRivalNumber;

	return count;
}
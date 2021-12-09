// Guild.cpp: implementation of the CGuild class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Guild.h"
#include "BattleSoccer.h"
#include "BattleSoccerManager.h"
#include "CastleSiege.h"
#include "CastleSiegeSync.h"
#include "DefaultClassInfo.h"
#include "ESProtocol.h"
#include "GensSystem.h"
#include "GuildClass.h"
#include "GuildMatching.h"
#include "Map.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "ServerInfo.h"
#include "Union.h"
#include "Util.h"

CGuild gGuild;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGuild::CGuild() // OK
{

}

CGuild::~CGuild() // OK
{

}

int CGuild::GetMaxGuildUser(LPOBJ lpObj) // OK
{
	int MaxGuildUser = 0;

	if(lpObj->Class == CLASS_DL)
	{
		MaxGuildUser = ((lpObj->Reset==0)?(((lpObj->Level+lpObj->MasterLevel)/10)+(lpObj->Leadership/10)):gServerInfo.m_GuildInsertMaxUser2);

		MaxGuildUser = ((MaxGuildUser>gServerInfo.m_GuildInsertMaxUser2)?gServerInfo.m_GuildInsertMaxUser2:MaxGuildUser);
	}
	else
	{
		MaxGuildUser = ((lpObj->Reset==0)?((lpObj->Level+lpObj->MasterLevel)/10):gServerInfo.m_GuildInsertMaxUser1);

		MaxGuildUser = ((MaxGuildUser>gServerInfo.m_GuildInsertMaxUser1)?gServerInfo.m_GuildInsertMaxUser1:MaxGuildUser);
	}

	return ((MaxGuildUser>MAX_GUILD_USER)?MAX_GUILD_USER:MaxGuildUser);
}

int CGuild::GetGuildNameCount(LPOBJ lpObj,char* name) // OK
{
	int count = 0;

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpObj->Guild->Use[n] != 0 && strcmp(lpObj->Guild->Names[n],name) == 0)
		{
			count++;
		}
	}

	return count;
}

int CGuild::GetGuildBattleMasterCount(LPOBJ lpObj) // OK
{
	int count = 0;

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpObj->Guild->Use[n] != 0 && lpObj->Guild->GuildStatus[n] == GUILD_STATUS_BATTLE_MASTER)
		{
			count++;
		}
	}

	return count;
}

int CGuild::GetGuildOfficeMasterCount(LPOBJ lpObj) // OK
{
	int count = 0;

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpObj->Guild->Use[n] != 0 && lpObj->Guild->GuildStatus[n] == GUILD_STATUS_OFFICE_MASTER)
		{
			count++;
		}
	}

	return count;
}

void CGuild::CGGuildRequestRecv(PMSG_GUILD_REQUEST_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(gObjIsConnectedGP(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if((lpTarget->Option & 1) == 0)
	{
		this->GCGuildResultSend(aIndex,0);
		return;
	}

	if(lpTarget->GuildNumber == 0)
	{
		this->GCGuildResultSend(aIndex,4);
		return;
	}

	if(strcmp(lpTarget->Guild->Names[0],lpTarget->Name) != 0)
	{
		this->GCGuildResultSend(aIndex,4);
		return;
	}

	if(lpObj->GuildNumber > 0)
	{
		this->GCGuildResultSend(aIndex,5);
		return;
	}

	if(lpObj->Interface.use != 0 || lpTarget->Interface.use != 0)
	{
		this->GCGuildResultSend(aIndex,6);
		return;
	}

	if(lpTarget->Guild->WarState != 0)
	{
		this->GCGuildResultSend(aIndex,6);
		return;
	}

	#if(GAMESERVER_TYPE==1)

	if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
	{
		this->GCGuildResultSend(aIndex,6);
		return;
	}

	#endif

	if(lpObj->Level < 6)
	{
		this->GCGuildResultSend(aIndex,7);
		return;
	}

	if(lpTarget->Guild->TotalCount >= this->GetMaxGuildUser(lpTarget))
	{
		this->GCGuildResultSend(aIndex,2);
		return;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && gObj[aIndex].GensFamily == GENS_FAMILY_NONE)
	{
		this->GCGuildResultSend(aIndex,163);
		return;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && gObj[bIndex].GensFamily == GENS_FAMILY_NONE)
	{
		this->GCGuildResultSend(aIndex,161);
		return;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && gObj[aIndex].GensFamily != gObj[bIndex].GensFamily)
	{
		this->GCGuildResultSend(aIndex,162);
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_GUILD;
	lpObj->Interface.state = 0;
	lpObj->InterfaceTime = GetTickCount();
	lpObj->TargetNumber = bIndex;

	lpTarget->Interface.use = 1;
	lpTarget->Interface.type = INTERFACE_GUILD;
	lpTarget->Interface.state = 0;
	lpTarget->InterfaceTime = GetTickCount();
	lpTarget->TargetNumber = aIndex;

	PMSG_GUILD_REQUEST_SEND pMsg;

	pMsg.header.set(0x50,sizeof(pMsg));

	pMsg.index[0] = SET_NUMBERHB(aIndex);
	pMsg.index[1] = SET_NUMBERLB(aIndex);

	DataSend(bIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::CGGuildResultRecv(PMSG_GUILD_RESULT_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(gObjIsConnectedGP(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_GUILD)
	{
		return;
	}

	if(lpTarget->Interface.use == 0 || lpTarget->Interface.type != INTERFACE_GUILD)
	{
		return;
	}

	if(lpMsg->result == 0)
	{
		this->GCGuildResultSend(bIndex,0);
		goto CLEAR_JUMP;
	}

	if((lpObj->Option & 1) == 0)
	{
		this->GCGuildResultSend(bIndex,0);
		goto CLEAR_JUMP;
	}

	if(lpObj->GuildNumber == 0)
	{
		this->GCGuildResultSend(bIndex,4);
		goto CLEAR_JUMP;
	}

	if(strcmp(lpObj->Guild->Names[0],lpObj->Name) != 0)
	{
		this->GCGuildResultSend(bIndex,4);
		goto CLEAR_JUMP;
	}

	if(lpTarget->GuildNumber > 0)
	{
		this->GCGuildResultSend(bIndex,5);
		goto CLEAR_JUMP;
	}

	if(lpObj->Guild->WarState != 0)
	{
		this->GCGuildResultSend(bIndex,6);
		goto CLEAR_JUMP;
	}

	#if(GAMESERVER_TYPE==1)

	if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
	{
		this->GCGuildResultSend(bIndex,6);
		goto CLEAR_JUMP;
	}

	#endif

	if(lpTarget->Level < 6)
	{
		this->GCGuildResultSend(bIndex,7);
		goto CLEAR_JUMP;
	}

	if(lpObj->Guild->TotalCount >= this->GetMaxGuildUser(lpObj))
	{
		this->GCGuildResultSend(bIndex,2);
		goto CLEAR_JUMP;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && gObj[aIndex].GensFamily == GENS_FAMILY_NONE)
	{
		this->GCGuildResultSend(aIndex,163);
		goto CLEAR_JUMP;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && gObj[bIndex].GensFamily == GENS_FAMILY_NONE)
	{
		this->GCGuildResultSend(aIndex,161);
		goto CLEAR_JUMP;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && gObj[aIndex].GensFamily != gObj[bIndex].GensFamily)
	{
		this->GCGuildResultSend(aIndex,162);
		goto CLEAR_JUMP;
	}

	gGuildMatching.GDGuildMatchingJoinCancelSend(bIndex,1);

	GDGuildMemberAdd(bIndex,lpObj->Guild->Name,lpTarget->Name);

	CLEAR_JUMP:

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;
	lpObj->InterfaceTime = 0;
	lpObj->TargetNumber = -1;

	lpTarget->Interface.use = 0;
	lpTarget->Interface.type = INTERFACE_NONE;
	lpTarget->Interface.state = 0;
	lpTarget->InterfaceTime = 0;
	lpTarget->TargetNumber = -1;
}

void CGuild::CGGuildListRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0 || lpObj->GuildNumber == 0)
	{
		return;
	}

	BYTE send[2048];

	PMSG_GUILD_LIST_SEND pMsg;

	pMsg.header.set(0x52,0);

	int size = sizeof(pMsg);

	pMsg.result = 1;

	pMsg.count = 0;

	pMsg.TotalScore = lpObj->Guild->TotalScore;

	pMsg.score = lpObj->Guild->PlayScore;

	GUILD_INFO_STRUCT* lpRivalGuild = gGuildClass.SearchGuild_Number(lpObj->Guild->GuildRival);

	if(lpRivalGuild == 0)
	{
		memset(pMsg.RivalGuild,0,sizeof(pMsg.RivalGuild));

		#if(GAMESERVER_UPDATE>=701)

		pMsg.RivalCount = 0;

		memset(pMsg.RivalMark,0,sizeof(pMsg.RivalMark));

		#endif
	}
	else
	{
		memcpy(pMsg.RivalGuild,lpRivalGuild->Name,sizeof(pMsg.RivalGuild));

		#if(GAMESERVER_UPDATE>=701)

		pMsg.RivalCount = lpRivalGuild->Count;

		memcpy(pMsg.RivalMark,lpRivalGuild->Mark,sizeof(pMsg.RivalMark));

		#endif
	}

	PMSG_GUILD_LIST info;

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpObj->Guild->Use[n] == 0)
		{
			continue;
		}

		memcpy(info.name,lpObj->Guild->Names[n],sizeof(info.name));

		info.number = lpObj->Guild->Server[n];

		info.connected = (lpObj->Guild->Server[n] & 0x7F) | ((lpObj->Guild->Server[n]>=0)?0x80:0);

		info.status = lpObj->Guild->GuildStatus[n];

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	if(pMsg.count > 0)
	{
		pMsg.header.size[0] = SET_NUMBERHB(size);
		pMsg.header.size[1] = SET_NUMBERLB(size);

		memcpy(send,&pMsg,sizeof(pMsg));

		DataSend(aIndex,send,size);
	}
}

void CGuild::CGGuildDeleteRecv(PMSG_GUILD_DELETE_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_GUILD_DELETE_SEND pMsg;

	pMsg.header.set(0x53,sizeof(pMsg));

	pMsg.result = 0;

	if(gServerInfo.m_GuildDeleteSwitch == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->GuildNumber == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	#if(GAMESERVER_TYPE==1)

	if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	#endif

	char name[11] = {0};

	char PersonalCode[14] = {0};

	memcpy(name,lpMsg->name,sizeof(lpMsg->name));

	memcpy(PersonalCode,lpMsg->PersonalCode,sizeof(lpMsg->PersonalCode));

	if(strcmp(lpObj->Guild->Names[0],lpObj->Name) == 0)
	{
		if(strcmp(lpObj->Name,name) != 0)
		{
			if(gObjCheckPersonalCode(aIndex,PersonalCode) == 0)
			{
				DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
				return;
			}
			else
			{
				GDGuildMemberDel(aIndex,lpObj->Guild->Name,name);
				return;
			}
		}

		if(gObjCheckPersonalCode(aIndex,PersonalCode) == 0)
		{
			DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		if(gServerInfo.m_GuildOwnerDestroyLimit != 0 && gCastleSiegeSync.CheckCastleOwnerMember(aIndex) != 0)
		{
			DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		gObjGuildWarMasterClose(lpObj);

		gGuildMatching.GDGuildMatchingCancelSend(aIndex,1);

		GDGuildDestroySend(aIndex,lpObj->Guild->Name,name);

		LogAdd(LOG_BLACK,"[%s][%s] Guild Request - Dismiss All : %s",lpObj->Account,lpObj->Name,lpObj->Guild->Name);
	}
	else
	{
		if(strcmp(lpObj->Name,name) != 0)
		{
			DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		if(gObjCheckPersonalCode(aIndex,PersonalCode) == 0)
		{
			DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		GDGuildMemberDel(aIndex,lpObj->Guild->Name,name);

		LogAdd(LOG_BLACK,"[%s][%s] Guild Request - Leave : %s",lpObj->Account,lpObj->Name,lpObj->Guild->Name);
	}
}

void CGuild::CGGuildAssignStatusRecv(PMSG_GUILD_ASSIGN_STATUS_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpMsg->type != 1 && lpMsg->type != 2 && lpMsg->type != 3)
	{
		return;
	}

	char name[11] = {0};

	memcpy(name,lpMsg->name,sizeof(lpMsg->name));

	if(lpObj->GuildNumber == 0)
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,16,name);
		return;
	}

	if(strcmp(lpObj->Name,name) == 0)
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,17,name);
		return;
	}

	if(this->GetGuildNameCount(lpObj,name) == 0)
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,17,name);
		return;
	}

	if(strcmp(lpObj->Guild->Names[0],lpObj->Name) != 0)
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,17,name);
		return;
	}

	if(lpMsg->status != GUILD_STATUS_MEMBER && lpMsg->status != GUILD_STATUS_BATTLE_MASTER && lpMsg->status != GUILD_STATUS_OFFICE_MASTER)
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,18,name);
		return;
	}

	if(lpMsg->status == GUILD_STATUS_BATTLE_MASTER && this->GetGuildBattleMasterCount(lpObj) >= (((lpObj->Level+lpObj->MasterLevel)/200)+1))
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,18,name);
		return;
	}

	if(lpMsg->status == GUILD_STATUS_OFFICE_MASTER && this->GetGuildOfficeMasterCount(lpObj) > 0)
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,18,name);
		return;
	}

	GDGuildReqAssignStatus(aIndex,lpMsg->type,name,lpMsg->status);
}

void CGuild::GCGuildResultSend(int aIndex,int result) // OK
{
	PMSG_GUILD_RESULT_SEND pMsg;

	pMsg.header.set(0x51,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildWarEndSend(int aIndex,int result,char* GuildName) // OK
{
	PMSG_GUILD_WAR_END_SEND pMsg;

	pMsg.header.set(0x63,sizeof(pMsg));

	pMsg.result = result;

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildWarScoreSend(int aIndex) // OK
{
	if(gObj[aIndex].GuildNumber == 0)
	{
		return;
	}

	PMSG_GUILD_WAR_SCORE_SEND pMsg;

	pMsg.header.set(0x64,sizeof(pMsg));

	pMsg.score[0] = ((gObj[aIndex].Guild==0)?0:gObj[aIndex].Guild->PlayScore);

	pMsg.score[1] = ((gObj[aIndex].Guild->TargetGuildNode==0)?0:gObj[aIndex].Guild->TargetGuildNode->PlayScore);

	pMsg.type = 0;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildAssignStatusSend(int aIndex,int type,int result,char* name) // OK
{
	PMSG_GUILD_ASSIGN_STATUS_SEND pMsg;

	pMsg.header.set(0xE1,sizeof(pMsg));

	pMsg.type = type;

	pMsg.result = result;

	memcpy(pMsg.name,name,sizeof(pMsg.name));
}

//**************************************************************************//
// RAW FUNCTIONS ***********************************************************//
//**************************************************************************//

void GCGuildWarDeclare(int aIndex, LPSTR _guildname) //check gs-cs
{
#if(GAMESERVER_TYPE==0)
	PMSG_GUILDWAR_DECLARE pMsg;
	pMsg.h.set(0x62, sizeof(pMsg));
	memcpy(pMsg.GuildName, _guildname, sizeof(pMsg.GuildName));

	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
#endif
}

BOOL gObjGuildMasterCapacityTest(LPOBJ lpObj)
{
	if (lpObj->Level < gServerInfo.m_GuildCreateMinLevel)
	{
		return 0;
	}
	return 1;
}

void CGGuildMasterAnswerRecv(PMSG_GUILDMASTERANSWER * lpMsg, int aIndex) // 0x54
{
	if ( lpMsg->Result == 1 )
	{
		if ( !gObjGuildMasterCapacityTest(&gObj[aIndex]))
		{
			if ( gObj[aIndex].Interface.use && gObj[aIndex].Interface.type == 5 )
			{
				gObj[aIndex].Interface.use = 0;
			}

			return;
		}

		GCGuildMasterManagerRun(aIndex);
		return;
	}

	if ( gObj[aIndex].Interface.use && gObj[aIndex].Interface.type == 5 )
	{
		gObj[aIndex].Interface.use = 0;
	}
}

void GCGuildMasterManagerRun(int aIndex) 
{
	PMSG_GUILDMASTERMANAGER_RUN pMsg;

	pMsg.h.set(0x55, sizeof(pMsg));

	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
}

void CGGuildMasterInfoSave(int aIndex, PMSG_GUILDINFOSAVE * lpMsg) //0x55
{
	char GuildName[9];
	GuildName[8] ='\0';
	memcpy(GuildName, lpMsg->GuildName, 8);
	int len = strlen(GuildName);

	if ( len <= 2 || len > 8)
	{
		PMSG_GUILDCREATED_RESULT pMsg;

		pMsg.h.set(0x56, sizeof(pMsg));
		pMsg.Result = 2;
		
		DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);

		if ( gObj[aIndex].Interface.use && gObj[aIndex].Interface.type == 5 )
		{
			gObj[aIndex].Interface.use = 0;
		}

		return;
	}

	gGuildMatching.GDGuildMatchingJoinCancelSend(aIndex,1);

	GDGuildCreateSend(aIndex, GuildName, gObj[aIndex].Name, lpMsg->Mark, lpMsg->btGuildType);

	if ( gObj[aIndex].Interface.use && gObj[aIndex].Interface.type == 5 )
	{
		gObj[aIndex].Interface.use = 0;
	}
}

void CGGuildMasterCreateCancel(int aIndex)
{
	if ( gObj[aIndex].GuildNumber > 0 )
	{
		return;
	}

	if ( gObj[aIndex].Interface.use && gObj[aIndex].Interface.type == 5 )
	{
		gObj[aIndex].Interface.use = 0;
	}
}

void GCServerMsgStringSendGuild(GUILD_INFO_STRUCT* lpNode, LPSTR szMsg, BYTE type) 
{
	if ( lpNode == NULL )
	{
		return;
	}

	for ( int n = 0; n<MAX_GUILD_USER ; n++)
	{
		if (lpNode->Use[n] > 0 && lpNode->Index[n] >= 0 )
		{
			gNotice.GCNoticeSend(lpNode->Index[n],type,0,0,0,0,0,(char*)szMsg);
		}
	}
}

void GCGuildWarRequestResult(LPSTR GuildName, int aIndex, int type)
{
#if(GAMESERVER_TYPE == 1)
	return;
#endif
	PMSG_GUILDWARREQUEST_RESULT pMsg;

	pMsg.h.set(0x60, sizeof(pMsg));
	pMsg.Result = 3;

	if ( gObj[aIndex].GuildNumber < 1)
	{
		DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
		return;
	}

	GUILD_INFO_STRUCT * lpMyGuild = gObj[aIndex].Guild;

	if ( !lpMyGuild )
	{
		DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
		return;
	}

	if ( lpMyGuild->WarState == 1 || lpMyGuild->WarDeclareState == 1 )
	{
		pMsg.Result = 4;
		DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
		return;
	}

	if ( gServerInfo.m_PKLimitFree == 0 && gObj[aIndex].PKLevel >= 6)
	{
		pMsg.Result = 4;
		DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
		return;
	}

	if ( strcmp(lpMyGuild->Names[0], gObj[aIndex].Name ) )
	{
		pMsg.Result = 5;
		DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
		return;
	}

	char _guildname[9];
	memset(_guildname, 0, sizeof(_guildname));
	memcpy(_guildname, GuildName, 8);

	if ( !strncmp(lpMyGuild->Name, GuildName, 8))
	{
		return;
	}

	GUILD_INFO_STRUCT * lpNode = gGuildClass.SearchGuild(_guildname);

	if ( lpNode )
	{
		if ( lpNode->WarState == 1 || lpNode->WarDeclareState == 1 )
		{
			pMsg.Result = 4;
			DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
			return;
		}

		if ( lpMyGuild->GuildUnion != 0 && lpMyGuild->GuildUnion == lpNode->GuildUnion )
		{
			return;
		}

		int n=0;
		int warmaster=-1;

		while ( true )
		{
			if ( gObj[n].Type == OBJECT_USER )
			{
				if ( gObj[n].Connected > OBJECT_LOGGED )
				{
					if ( gObj[n].Name[0] == lpNode->Names[0][0] )
					{
						if ( !strcmp(gObj[n].Name, lpNode->Names[0]))
						{
							if ( gServerInfo.m_PKLimitFree == 0 && gObj[n].PKLevel >= 6)
							{
								pMsg.Result = 4;
								DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
								return;
							}

							warmaster = n;
							break;
						}
					}
				}
			}

			if ( n < MAX_OBJECT-1 )
			{
				n++;
			}
			else
			{
				break;
			}
		}
		
		if ( warmaster >= 1 )
		{
			if ( CA_MAP_RANGE(gObj[aIndex].Map) || CA_MAP_RANGE(gObj[warmaster].Map) )
			{
				return;
			}

			if ( CC_MAP_RANGE(gObj[aIndex].Map) || CC_MAP_RANGE(gObj[warmaster].Map) )
			{
				return;
			}

			if ( IT_MAP_RANGE(gObj[aIndex].Map) || IT_MAP_RANGE(gObj[warmaster].Map) )
			{
				return;
			}

			if ( (gObj[warmaster].Option&1) != 1 )
			{
				pMsg.Result = 4;
				DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
				return;
			}

			pMsg.Result = 1;
			DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
			GCGuildWarRequestSend(lpMyGuild->Name, warmaster, type);
			lpMyGuild->WarDeclareState = 1;
			lpNode->WarDeclareState = 1;
			lpMyGuild->WarType = type;
			lpNode->WarType = type;

			strcpy_s(lpMyGuild->TargetGuildName, lpNode->Name);
			strcpy_s(lpNode->TargetGuildName, lpMyGuild->Name);
			lpMyGuild->TargetGuildNode = lpNode;
			lpNode->TargetGuildNode = lpMyGuild;
		}
		else
		{
			pMsg.Result = 2;
			DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
		}
	}
	else
	{
		pMsg.Result = 0;
		DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
		return;
	}
}

void GCGuildWarRequestSend(LPSTR GuildName, int aIndex, int type) //check gs-cs
{
#if(GAMESERVER_TYPE==0)
	PMSG_GUILDWARSEND pMsg;
	pMsg.h.set( 0x61, sizeof(pMsg));
	pMsg.Type = type;
	memcpy(pMsg.GuildName, GuildName, 8);

	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
#endif
}

void GCGuildWarRequestSendRecv(PMSG_GUILDWARSEND_RESULT * lpMsg, int aIndex) // 61
{
#if(GAMESERVER_TYPE == 1)
	return;
#endif

	PMSG_GUILDWAR_DECLARE pMsg;
	int count=0;
	int g_call=0;
	
	pMsg.h.set(0x62, sizeof(pMsg));
	pMsg.Type = 0;
	
	GUILD_INFO_STRUCT * lpMyNode = gObj[aIndex].Guild;

	if ( !lpMyNode )
	{
		PMSG_GUILDWARREQUEST_RESULT pResult;

		pResult.h.type = 0xC1;
		pResult.h.head = 0x60;
		pResult.h.size = sizeof(pMsg);
		pResult.Result = 0;

		DataSend(aIndex, (LPBYTE)&pResult, pResult.h.size);
		return;
	}

	if ( lpMyNode->TargetGuildNode && lpMsg->Result )
	{
		int iTarGetIndex = lpMyNode->TargetGuildNode->Index[0];

		if ( CA_MAP_RANGE(gObj[iTarGetIndex].Map) || BC_MAP_RANGE(gObj[iTarGetIndex].Map) || CC_MAP_RANGE(gObj[iTarGetIndex].Map) || DS_MAP_RANGE(gObj[iTarGetIndex].Map) || IT_MAP_RANGE(gObj[iTarGetIndex].Map))
		{
			lpMsg->Result = 0;
		}
	}

	pMsg.Type = lpMyNode->WarType;

	if ( lpMsg->Result == 0 )
	{
		if ( lpMyNode->TargetGuildNode )
		{
			if ( lpMyNode->TargetGuildNode->WarDeclareState == 1 )
			{
				lpMyNode->TargetGuildNode->WarDeclareState = 0;
				lpMyNode->WarDeclareState = 0;

				PMSG_GUILDWARREQUEST_RESULT pResult;

				pResult.h.set(0x60, sizeof(pResult));
				pResult.Result = 6;

				if ( lpMyNode->TargetGuildNode->Index[0] >= 0 )
				{
					DataSend(lpMyNode->TargetGuildNode->Index[0], (LPBYTE)&pResult, pResult.h.size);
				}
			}
		}
	}
	else
	{
		if ( lpMyNode->WarDeclareState == 1 )
		{
			if ( lpMyNode->TargetGuildNode )
			{
				if (lpMyNode->TargetGuildNode->WarDeclareState == 1 )
				{
					if ( lpMyNode->WarType == 1 )
					{
						lpMyNode->BattleGroundIndex = gCheckBlankBattleGround();

						switch ( lpMyNode->BattleGroundIndex )
						{
							case 0xFF:
								lpMyNode->WarDeclareState = 0;
								lpMyNode->WarState = 0;
								lpMyNode->TargetGuildNode->WarDeclareState = 0;
								lpMyNode->TargetGuildNode->WarState = 0;

								PMSG_GUILDWARREQUEST_RESULT pResult;

								pResult.h.set(0x60, sizeof(pResult));
								pResult.Result = 4;

								DataSend(aIndex, (LPBYTE)&pResult, pResult.h.size);
								return;
						}

						lpMyNode->TargetGuildNode->BattleGroundIndex = lpMyNode->BattleGroundIndex;

						if ( gBSGround[0]->m_BallIndex >= 0 )
						{
							gObjMonsterRegen(&gObj[gBSGround[0]->m_BallIndex]);
						}

						BattleSoccerGoalEnd(0);
						lpMyNode->PlayScore = 0;
						lpMyNode->TargetGuildNode->PlayScore = 0;
						gBattleGroundEnable(lpMyNode->BattleGroundIndex, TRUE);
						gSetBattleTeamMaster(lpMyNode->BattleGroundIndex, 0, lpMyNode->Name, lpMyNode);
						gSetBattleTeamMaster(lpMyNode->BattleGroundIndex, 1, lpMyNode->TargetGuildNode->Name, lpMyNode->TargetGuildNode);
					}

					lpMyNode->WarDeclareState = 2;
					lpMyNode->WarState = 1;
					lpMyNode->TargetGuildNode->WarDeclareState = 2;
					lpMyNode->TargetGuildNode->WarState = 1;
					lpMyNode->PlayScore = 0;
					lpMyNode->TargetGuildNode->PlayScore = 0;
					lpMyNode->BattleTeamCode = 0;
					lpMyNode->TargetGuildNode->BattleTeamCode = 1;
					memset(pMsg.GuildName, 0, sizeof(pMsg.GuildName));
					memcpy(pMsg.GuildName, lpMyNode->TargetGuildNode->Name, sizeof(pMsg.GuildName));
					pMsg.TeamCode = lpMyNode->BattleTeamCode;
					count = 0;

					for ( int n=0;n<MAX_GUILD_USER;n++)
					{
						if ( lpMyNode->Use[n] )
						{
							if ( lpMyNode->Index[n] >= 0 )
							{
								g_call = 0;

								if ( n > 0 )
								{
									if ( lpMyNode->WarType == 1 )
									{
										if ( gObj[lpMyNode->Index[0]].PartyNumber  >= 0 )
										{
											if ( gObj[lpMyNode->Index[0]].PartyNumber == gObj[lpMyNode->Index[n]].PartyNumber )
											{
												g_call = 1;
											}
										}
									}
									else 
									{
										g_call = 1;
									}
								}
								else if ( n== 0 )
								{
									g_call = 1;
								}

								if ( g_call )
								{
									DataSend(lpMyNode->Index[n], (LPBYTE)&pMsg, pMsg.h.size);

									gGuild.GCGuildWarScoreSend(lpMyNode->Index[n]);
									int x = 60;

									if ( lpMyNode->WarType == 1 )
									{
										gObj[lpMyNode->Index[n]].IsInBattleGround = true;

										if ( gServerInfo.m_PKLimitFree != 0 || gObj[lpMyNode->Index[n]].PKLevel < 6 )
										{
											gObjTeleport(lpMyNode->Index[n], 6, x++, 153);
											count++;
										}
									}
								}
							}
						}
					}

					memset(pMsg.GuildName, 0, sizeof(pMsg.GuildName));
					memcpy(pMsg.GuildName, lpMyNode->Name, sizeof(pMsg.GuildName));

					pMsg.TeamCode = lpMyNode->TargetGuildNode->BattleTeamCode;
					count = 0;

					for (int n=0;n<MAX_GUILD_USER;n++)
					{
						if ( lpMyNode->TargetGuildNode->Use[n] )
						{
							if ( lpMyNode->TargetGuildNode->Index[n] >= 0 )
							{
								g_call = 0;

								if ( n > 0 )
								{
									if ( lpMyNode->WarType == 1 )
									{
										if ( gObj[lpMyNode->TargetGuildNode->Index[0]].PartyNumber  >= 0 )
										{
											if ( gObj[lpMyNode->TargetGuildNode->Index[0]].PartyNumber == gObj[lpMyNode->TargetGuildNode->Index[n]].PartyNumber )
											{
												g_call = 1;
											}
										}
									}
									else 
									{
										g_call = 1;
									}
								}
								else if ( n== 0 )
								{
									g_call = 1;
								}

								if ( g_call )
								{
									DataSend(lpMyNode->TargetGuildNode->Index[n], (LPBYTE)&pMsg, pMsg.h.size);

									gGuild.GCGuildWarScoreSend(lpMyNode->TargetGuildNode->Index[n]);
									int x = 59;

									if ( lpMyNode->TargetGuildNode->WarType == 1 )
									{

										if ( gServerInfo.m_PKLimitFree != 0 || gObj[lpMyNode->TargetGuildNode->Index[n]].PKLevel < 6 )
										{
											gObj[lpMyNode->TargetGuildNode->Index[n]].IsInBattleGround = true;
											gObjTeleport(lpMyNode->TargetGuildNode->Index[n], 6, x++, 164);
											count++;
										}
									}
								}
							}
						}
					}

					if ( lpMyNode->WarType == 1 )
					{
						gObjAddMsgSendDelay(&gObj[aIndex], 5, aIndex, 10000, 0);
						GCServerMsgStringSendGuild(lpMyNode->TargetGuildNode,gMessage.GetMessage(490), 1);
						GCServerMsgStringSendGuild(lpMyNode,gMessage.GetMessage(490), 1);
					}
				}
			}
		}
	}
}

void GCGuildViewportInfo(PMSG_REQ_GUILDVIEWPORT * aRecv, int aIndex) 
{
	if ( !gObjIsConnectedGP(aIndex))
	{
		return;
	}

	if ( !OBJECT_RANGE(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];
	DWORD dwGuildNumber = aRecv->GuildNumber;
	GUILD_INFO_STRUCT * lpGuildInfo = gGuildClass.SearchGuild_Number(dwGuildNumber);

	if ( lpGuildInfo )
	{
		PMSG_ANS_GUILDVIEWPORT pMsg;

		pMsg.h.type = 0xC1;
		pMsg.h.head = 0x66;
		pMsg.h.size = sizeof(pMsg);
		pMsg.GuildNumber = lpGuildInfo->Number;
		pMsg.btGuildType = lpGuildInfo->GuildType;

		CUnionInfo * pUnionInfo = gUnionManager.SearchUnion(lpGuildInfo->GuildUnion);

		if ( pUnionInfo )
		{
			memcpy(pMsg.UnionName, pUnionInfo->m_szMasterGuild, 8);
		}
		else
		{
			pMsg.UnionName[0] = '\0';
		}

		memcpy(pMsg.GuildName, lpGuildInfo->Name, sizeof(pMsg.GuildName));
		memcpy(pMsg.Mark, lpGuildInfo->Mark, sizeof(pMsg.Mark));

		DataSend(aIndex, (LPBYTE)&pMsg, sizeof(pMsg));
	}
}

void CGGuildAssignType(PMSG_GUILD_ASSIGN_TYPE_REQ * aRecv, int aIndex) 
{
	if ( !gObjIsConnectedGP(aIndex))
	{
		return;
	}

	if ( !OBJECT_RANGE(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];
	PMSG_GUILD_ASSIGN_TYPE_RESULT pMsg={0};
	
	pMsg.h.set(0xE2, sizeof(pMsg));
	pMsg.btGuildType = aRecv->btGuildType;
	
	if ( lpObj->GuildNumber <= 0 || lpObj->Guild == NULL )
	{
		pMsg.btResult = 16;
		DataSend(aIndex, (LPBYTE)&pMsg, sizeof(pMsg));
		return;
	}

	if ( gObj[aIndex].GuildStatus != G_MASTER )
	{
		pMsg.btResult = 17;
		DataSend(aIndex, (LPBYTE)&pMsg, sizeof(pMsg));
		return;
	}
	
	GDGuildReqAssignType(aIndex, aRecv->btGuildType);
}

void CGRelationShipReqJoinBreakOff(PMSG_RELATIONSHIP_JOIN_BREAKOFF_REQ * aRecv, int aIndex) 
{
	if ( gCastleSiegeSync.GetCastleState() >= 5 && gCastleSiegeSync.GetCastleState() <= 7 )
	{
		return;
	}

	if ( IT_MAP_RANGE(gObj[aIndex].Map) != FALSE ) //season 2.5 add-on
	{
		return;
	}

	int iTargetUserIndex = MAKE_NUMBERW(aRecv->btTargetUserIndexH, aRecv->btTargetUserIndexL);

	if ( !OBJECT_RANGE(aIndex) || !OBJECT_RANGE(iTargetUserIndex))
	{
		return;
	}

	if ( gObjIsConnectedGP(iTargetUserIndex) == FALSE )
	{
		gGuild.GCGuildResultSend(aIndex,3);
		return;
	}

	if((gObj[iTargetUserIndex].Option&1) != 1) //season 3.0 add-on
	{
		return;
	}

	if ( !gObj[aIndex].Guild || !gObj[iTargetUserIndex].Guild )
	{
		gGuild.GCGuildResultSend(aIndex,5);
		return;
	}

	if ( gObj[aIndex].GuildStatus != G_MASTER && gObj[iTargetUserIndex].GuildStatus != G_MASTER)
	{
		gGuild.GCGuildResultSend(aIndex,4);
		return;
	}

	if ( gObj[aIndex].Interface.use > 0 )
	{
		gGuild.GCGuildResultSend(aIndex,6);
		return;
	}

	if ( gObj[iTargetUserIndex].Interface.use > 0 )
	{
		gGuild.GCGuildResultSend(aIndex,6);
		return;
	}

	PMSG_RELATIONSHIP_JOIN_BREAKOFF_ANS ErrMsg={0};

	ErrMsg.h.set(0xE6, sizeof(ErrMsg));
	ErrMsg.btResult = 0;
	ErrMsg.btRequestType = aRecv->btRequestType;
	ErrMsg.btRelationShipType = aRecv->btRelationShipType;
	ErrMsg.btTargetUserIndexH = aRecv->btTargetUserIndexH;
	ErrMsg.btTargetUserIndexL = aRecv->btTargetUserIndexL;

	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTargetObj = &gObj[iTargetUserIndex];

	if ( lpObj->GuildStatus != G_MASTER || lpTargetObj->GuildStatus != G_MASTER)
	{
		ErrMsg.btResult = 17;
		DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
		return;
	}

	GUILD_INFO_STRUCT * lpGuildInfo = gObj[aIndex].Guild;
	GUILD_INFO_STRUCT * lpTargetGuildInfo = gObj[iTargetUserIndex].Guild;
	BYTE btRelationShip = gObjGetRelationShip(lpObj, lpTargetObj);

	if ( aRecv->btRequestType == 1 )
	{
		if ( aRecv->btRelationShipType == 1 )
		{
			if ( lpTargetGuildInfo->Count < gServerInfo.m_GuildAllianceMinUser )
			{
				ErrMsg.btResult = 29;
				DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
				return;
			}

			if ( lpTargetGuildInfo->GuildUnion != 0 && lpTargetGuildInfo->GuildUnion == lpTargetGuildInfo->Number )
			{
				if ( gUnionManager.GetGuildRelationShipCount(lpTargetGuildInfo->GuildUnion, 1) >= ((gServerInfo.m_GuildAllianceMaxCount>MAX_UNION_MEMBER)?MAX_UNION_MEMBER:gServerInfo.m_GuildAllianceMaxCount) )
				{
					ErrMsg.btResult = GUILD_ANS_EXCEED_MAX_UNION_MEMBER;
					DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
					return;
				}
			}
		
			char szCastleOwnerGuildName[9]={0};
			memcpy(szCastleOwnerGuildName, gCastleSiegeSync.GetCastleOwnerGuild(), 8);

			if ( !strcmp(lpGuildInfo->Name, szCastleOwnerGuildName))
			{
				ErrMsg.btResult = 23;
				DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
				return;
			}

			if ( lpGuildInfo->GuildUnion )
			{
				ErrMsg.btResult = 23;
				DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
				return;
			}

			if ( btRelationShip == 2 )
			{
				ErrMsg.btResult = 22;
				DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
				return;
			}

			if ( lpTargetGuildInfo->GuildUnion && lpTargetGuildInfo->Number != lpTargetGuildInfo->GuildUnion )
			{
				ErrMsg.btResult = 27;
				DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
				return;
			}
		}
		else if ( aRecv->btRelationShipType == 2 )
		{
			if ( lpGuildInfo->GuildRival || lpTargetGuildInfo->GuildRival )
			{
				ErrMsg.btResult = 24;
				DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
				return;
			}

			if ( btRelationShip == 1 )
			{
				ErrMsg.btResult = 21;
				DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
				return;
			}
		}
	}
	else if ( aRecv->btRequestType == 2 )
	{
		if ( aRecv->btRelationShipType == 1 )
		{
			if ( lpGuildInfo->GuildUnion == 0 )
			{
				ErrMsg.btResult = 25;
				DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
				return;
			}
		}
		else if ( aRecv->btRelationShipType == 2 )
		{
			if ( !lpGuildInfo->GuildRival || !lpTargetGuildInfo->GuildRival )
			{
				ErrMsg.btResult = 25;
				DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
				return;
			}

			if ( lpGuildInfo->GuildRival != lpTargetGuildInfo->Number || lpGuildInfo->Number != lpTargetGuildInfo->GuildRival )
			{
				ErrMsg.btResult = 28;
				DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
				return;
			}
		}
	}		
	
	if ( aRecv->btRequestType == 2 )
	{
		if ( aRecv->btRelationShipType == 1 )
		{
			GDUnionBreakOff(aIndex, lpGuildInfo->GuildUnion);
			return;
		}
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = 14;
	lpObj->Interface.state = 0;
	lpObj->InterfaceTime = GetTickCount();

	lpTargetObj->Interface.use = 1;
	lpTargetObj->Interface.type = 14;
	lpTargetObj->Interface.state = 0;
	lpTargetObj->InterfaceTime = GetTickCount();

	PMSG_RELATIONSHIP_JOIN_BREAKOFF_REQ pMsg={0};

	pMsg.h.set(0xE5, sizeof(pMsg));
	pMsg.btTargetUserIndexL = SET_NUMBERLB(aIndex);
	pMsg.btTargetUserIndexH = SET_NUMBERHB(aIndex);
	pMsg.btRequestType = aRecv->btRequestType;
	pMsg.btRelationShipType = aRecv->btRelationShipType;
	
	DataSend(iTargetUserIndex, (LPBYTE)&pMsg, pMsg.h.size);
}

void CGRelationShipAnsJoinBreakOff(PMSG_RELATIONSHIP_JOIN_BREAKOFF_ANS * aRecv, int aIndex) 
{
	int iTargetUserIndex = MAKE_NUMBERW(aRecv->btTargetUserIndexH, aRecv->btTargetUserIndexL);

	if ( !OBJECT_RANGE(aIndex) || !OBJECT_RANGE(iTargetUserIndex))
	{
		return;
	}

	if ( gObjIsConnectedGP(iTargetUserIndex) == FALSE )
	{
		gGuild.GCGuildResultSend(aIndex,3);
		return;
	}

	if ( aRecv->btResult == 1 )
	{
		if ( aRecv->btRequestType == 1 )
		{
			GDRelationShipReqJoin(iTargetUserIndex, aIndex, aRecv->btRelationShipType);
		}
		else if ( aRecv->btRequestType == 2 )
		{
			GDRelationShipReqBreakOff(aIndex, iTargetUserIndex, aRecv->btRelationShipType);
		}
	}
	else
	{
		PMSG_RELATIONSHIP_JOIN_BREAKOFF_ANS ErrMsg ={0};

		ErrMsg.h.set(0xE6, sizeof(ErrMsg));
		ErrMsg.btResult = 0;
		ErrMsg.btRequestType = aRecv->btRequestType;
		ErrMsg.btRelationShipType = aRecv->btRelationShipType;
		ErrMsg.btTargetUserIndexH = aRecv->btTargetUserIndexH;
		ErrMsg.btTargetUserIndexL = aRecv->btTargetUserIndexL;
		ErrMsg.btResult = 32;

		DataSend(iTargetUserIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
	}

	if ( gObj[aIndex].Interface.use )
	{
		if ( gObj[aIndex].Interface.type == 14 )
		{
			gObj[aIndex].Interface.use = 0;
		}
	}

	if ( gObj[iTargetUserIndex].Interface.use )
	{
		if ( gObj[iTargetUserIndex].Interface.type == 14 )
		{
			gObj[iTargetUserIndex].Interface.use = 0;
		}
	}
}

void CGUnionList(PMSG_UNIONLIST_REQ * aRecv, int aIndex) 
{
	if ( !OBJECT_RANGE(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if ( lpObj == NULL )
	{
		return;
	}

	if ( gObjIsConnectedGP(aIndex) == FALSE )
	{
		gGuild.GCGuildResultSend(aIndex,3);
		return;
	}

	if ( lpObj->Guild == NULL )
	{
		gGuild.GCGuildResultSend(aIndex,3);
		return;
	}

	if ( lpObj->Guild->GuildUnion == 0 )
	{
		gGuild.GCGuildResultSend(aIndex,3);
		return;
	}

	GDUnionListSend(aIndex, lpObj->Guild->GuildUnion);
}

void CGRelationShipReqKickOutUnionMember(PMSG_KICKOUT_UNIONMEMBER_REQ* aRecv, int aIndex) 
{
	if ( !OBJECT_RANGE(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if ( lpObj == NULL )
	{
		return;
	}

	//if ( gObjIsConnectedGP(aIndex) == FALSE )
	//{
	//	gGuild.GCGuildResultSend(aIndex,3);
	//	return;
	//}

	//if ( lpObj->Guild == NULL )
	//{
	//	gGuild.GCGuildResultSend(aIndex,3);
	//	return;
	//}

	PMSG_RELATIONSHIP_JOIN_BREAKOFF_ANS ErrMsg = {0};

	ErrMsg.h.set(0xE6, sizeof(ErrMsg));
	ErrMsg.btResult = 0;
	ErrMsg.btRequestType = 2;
	ErrMsg.btRelationShipType = 1;
	ErrMsg.btTargetUserIndexH = 0;
	ErrMsg.btTargetUserIndexL = 0;

	if ( lpObj->Guild->GuildUnion == 0 )
	{
		ErrMsg.btResult = 25;
		DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
		return;
	}

	if ( lpObj->GuildStatus != GUILD_MASTER || lpObj->Guild->Number != lpObj->Guild->GuildUnion)
	{
		ErrMsg.btResult = 17;
		DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);

		return;
	}

	char szUnionMemberGuildName[9] = {0};
	memcpy(szUnionMemberGuildName, aRecv->szTargetGuildName, 8);

	if ( !strcmp(lpObj->Guild->Name, szUnionMemberGuildName))
	{
		ErrMsg.btResult = 17;
		DataSend(aIndex, (LPBYTE)&ErrMsg, ErrMsg.h.size);
		return;
	}

	GDRelationShipReqKickOutUnionMember(aIndex, szUnionMemberGuildName);
}

void GCResultSend(int aIndex, BYTE headcode, BYTE result)
{
	PMSG_GUILD_RESULT_SEND pResult;

	pResult.header.set(headcode, sizeof(pResult));
	pResult.result = result;
	
	DataSend(aIndex, (LPBYTE)&pResult, pResult.header.size);
}

void GCGuildViewportDelNow(int aIndex, BOOL isGuildMaster)
{
	PMSG_GUILDDEL_VIEWPORT_NOW pMsg;

	LPOBJ lpObj = &gObj[aIndex];

	pMsg.h.set( 0x5D, sizeof(pMsg));
	pMsg.NumberH = SET_NUMBERHB(aIndex) & 0x7F;
	pMsg.NumberL = SET_NUMBERLB(aIndex);

	if ( isGuildMaster != FALSE )
	{
		pMsg.NumberH |= 0x80;
	}

	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
	MsgSendV2(lpObj, (LPBYTE)&pMsg, pMsg.h.size);
}

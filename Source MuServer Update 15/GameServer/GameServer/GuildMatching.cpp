// GuildMatching.cpp: implementation of the CGuildMatching class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GuildMatching.h"
#include "CastleSiege.h"
#include "ESProtocol.h"
#include "GameMain.h"
#include "GensSystem.h"
#include "Guild.h"
#include "GuildClass.h"
#include "ServerInfo.h"
#include "User.h"
#include "Util.h"

CGuildMatching gGuildMatching;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGuildMatching::CGuildMatching() // OK
{

}

CGuildMatching::~CGuildMatching() // OK
{

}

void CGuildMatching::CGGuildMatchingListRecv(PMSG_GUILD_MATCHING_LIST_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->GDGuildMatchingListSend(aIndex,lpMsg->page);

	#endif
}

void CGuildMatching::CGGuildMatchingListSearchRecv(PMSG_GUILD_MATCHING_LIST_SEARCH_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->GDGuildMatchingListSearchSend(aIndex,lpMsg->page,lpMsg->SearchWord);

	#endif
}

void CGuildMatching::CGGuildMatchingInsertRecv(PMSG_GUILD_MATCHING_INSERT_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_GUILD_MATCHING_INSERT_SEND pMsg;

	pMsg.header.set(0xED,0x02,sizeof(pMsg));

	GUILD_INFO_STRUCT* lpGuildInfo = gGuildClass.SearchGuild_Number(lpObj->GuildNumber);

	if(lpGuildInfo == 0)
	{
		pMsg.result = 0xFFFFFFFC;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gGuildClass.GetGuildMemberStatus(lpGuildInfo->Name,lpObj->Name) != G_MASTER)
	{
		pMsg.result = 0xFFFFFFFC;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->PKLevel > 3)
	{
		pMsg.result = 0xFFFFFFFE;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	this->GDGuildMatchingInsertSend(aIndex,lpMsg->text,lpMsg->InterestType,lpMsg->LevelRange,lpMsg->ClassType);

	#endif
}

void CGuildMatching::CGGuildMatchingCancelRecv(PMSG_GUILD_MATCHING_CANCEL_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_GUILD_MATCHING_CANCEL_SEND pMsg;

	pMsg.header.set(0xED,0x03,sizeof(pMsg));

	GUILD_INFO_STRUCT* lpGuildInfo = gGuildClass.SearchGuild_Number(lpObj->GuildNumber);

	if(lpGuildInfo == 0)
	{
		pMsg.result = 0xFFFFFFFE;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gGuildClass.GetGuildMemberStatus(lpGuildInfo->Name,lpObj->Name) != G_MASTER)
	{
		pMsg.result = 0xFFFFFFFE;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	this->GDGuildMatchingCancelSend(aIndex,0);

	#endif
}

void CGuildMatching::CGGuildMatchingJoinInsertRecv(PMSG_GUILD_MATCHING_JOIN_INSERT_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_GUILD_MATCHING_JOIN_INSERT_SEND pMsg;

	pMsg.header.set(0xED,0x04,sizeof(pMsg));

	GUILD_INFO_STRUCT* lpGuildInfo = gGuildClass.SearchGuild_Number(lpMsg->GuildNumber);

	if(lpGuildInfo == 0)
	{
		pMsg.result = 0xFFFFFFFC;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->PKLevel > 3)
	{
		pMsg.result = 0xFFFFFFFE;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->GuildNumber > 0)
	{
		pMsg.result = 0xFFFFFFFD;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	this->GDGuildMatchingJoinInsertSend(aIndex,lpGuildInfo->Name);

	#endif
}

void CGuildMatching::CGGuildMatchingJoinCancelRecv(PMSG_GUILD_MATCHING_JOIN_CANCEL_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->GDGuildMatchingJoinCancelSend(aIndex,0);

	#endif
}

void CGuildMatching::CGGuildMatchingJoinAcceptRecv(PMSG_GUILD_MATCHING_JOIN_ACCEPT_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_GUILD_MATCHING_JOIN_ACCEPT_SEND pMsg;

	pMsg.header.set(0xED,0x06,sizeof(pMsg));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.type = lpMsg->type;

	pMsg.result = 0xFFFFFFF9;

	if(lpMsg->type == 0)
	{
		this->GDGuildMatchingJoinAcceptSend(aIndex,(BYTE)lpMsg->type,lpMsg->name);
		return;
	}

	if((lpObj->Option & 1) == 0)
	{
		gGuild.GCGuildResultSend(aIndex,0);
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->GuildNumber == 0)
	{
		gGuild.GCGuildResultSend(aIndex,4);
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(strcmp(lpObj->Guild->Names[0],lpObj->Name) != 0)
	{
		gGuild.GCGuildResultSend(aIndex,4);
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->Interface.use != 0)
	{
		gGuild.GCGuildResultSend(aIndex,6);
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->Guild->WarState != 0)
	{
		gGuild.GCGuildResultSend(aIndex,6);
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	#if(GAMESERVER_TYPE==1)

	if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
	{
		gGuild.GCGuildResultSend(aIndex,6);
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	#endif

	if(lpObj->Level < 6)
	{
		gGuild.GCGuildResultSend(aIndex,7);
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->Guild->TotalCount >= gGuild.GetMaxGuildUser(lpObj))
	{
		gGuild.GCGuildResultSend(aIndex,2);
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	this->GDGuildMatchingJoinAcceptSend(aIndex,(BYTE)lpMsg->type,lpMsg->name);

	#endif
}

void CGuildMatching::CGGuildMatchingJoinListRecv(PMSG_GUILD_MATCHING_JOIN_LIST_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_GUILD_MATCHING_JOIN_LIST_SEND pMsg;

	pMsg.header.set(0xED,0x07,sizeof(pMsg));

	pMsg.count = 0;

	GUILD_INFO_STRUCT* lpGuildInfo = gGuildClass.SearchGuild_Number(lpObj->GuildNumber);

	if(lpGuildInfo == 0)
	{
		pMsg.result = 0xFFFFFFFE;
		DataSend(aIndex,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	if(gGuildClass.GetGuildMemberStatus(lpGuildInfo->Name,lpObj->Name) != G_MASTER)
	{
		pMsg.result = 0xFFFFFFFE;
		DataSend(aIndex,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	this->GDGuildMatchingJoinListSend(aIndex);

	#endif
}

void CGuildMatching::CGGuildMatchingJoinInfoRecv(PMSG_GUILD_MATCHING_JOIN_INFO_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->GDGuildMatchingJoinInfoSend(aIndex);

	#endif
}

void CGuildMatching::DGGuildMatchingListRecv(SDHP_GUILD_MATCHING_LIST_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGuildMatchingListRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	BYTE send[1024];

	PMSG_GUILD_MATCHING_LIST_SEND pMsg;

	pMsg.header.set(0xED,0x00,0);

	int size = sizeof(pMsg);

	pMsg.CurPage = lpMsg->CurPage;

	pMsg.MaxPage = lpMsg->MaxPage;

	pMsg.count = 0;

	pMsg.result = lpMsg->result;

	PMSG_GUILD_MATCHING_LIST info;

	for(DWORD n=0;n < lpMsg->count;n++)
	{
		SDHP_GUILD_MATCHING_LIST* lpInfo = (SDHP_GUILD_MATCHING_LIST*)(((BYTE*)lpMsg)+sizeof(SDHP_GUILD_MATCHING_LIST_RECV)+(sizeof(SDHP_GUILD_MATCHING_LIST)*n));

		memcpy(info.text,lpInfo->text,sizeof(info.text));

		memcpy(info.name,lpInfo->name,sizeof(info.name));

		memcpy(info.GuildName,lpInfo->GuildName,sizeof(info.GuildName));

		info.GuildMemberCount = lpInfo->GuildMemberCount;

		info.GuildMasterClass = lpInfo->GuildMasterClass;

		info.InterestType = lpInfo->InterestType;

		info.LevelRange = lpInfo->LevelRange;

		info.ClassType = lpInfo->ClassType;

		info.GuildMasterLevel = lpInfo->GuildMasterLevel;

		info.BoardNumber = lpInfo->BoardNumber;

		info.GuildNumber = lpInfo->GuildNumber;

		info.GensType = ((gServerInfo.m_GensSystemGuildLock==0)?lpObj->GensFamily:lpInfo->GensType);

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(lpObj->Index,send,size);

	#endif
}

void CGuildMatching::DGGuildMatchingInsertRecv(SDHP_GUILD_MATCHING_INSERT_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGuildMatchingInsertRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	lpObj->UseGuildMatching = ((lpMsg->result==0)?1:lpObj->UseGuildMatching);

	PMSG_GUILD_MATCHING_INSERT_SEND pMsg;

	pMsg.header.set(0xED,0x02,sizeof(pMsg));

	pMsg.result = lpMsg->result;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::DGGuildMatchingCancelRecv(SDHP_GUILD_MATCHING_CANCEL_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGuildMatchingCancelRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	lpObj->UseGuildMatching = ((lpMsg->result==0)?0:lpObj->UseGuildMatching);

	PMSG_GUILD_MATCHING_CANCEL_SEND pMsg;

	pMsg.header.set(0xED,0x03,sizeof(pMsg));

	pMsg.result = lpMsg->result;

	if(lpMsg->flag == 0){DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);}

	#endif
}

void CGuildMatching::DGGuildMatchingJoinInsertRecv(SDHP_GUILD_MATCHING_JOIN_INSERT_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGuildMatchingJoinInsertRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	lpObj->UseGuildMatchingJoin = ((lpMsg->result==0)?1:lpObj->UseGuildMatching);

	PMSG_GUILD_MATCHING_JOIN_INSERT_SEND pMsg;

	pMsg.header.set(0xED,0x04,sizeof(pMsg));

	pMsg.result = lpMsg->result;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::DGGuildMatchingJoinCancelRecv(SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGuildMatchingJoinCancelRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	lpObj->UseGuildMatchingJoin = ((lpMsg->result==0)?0:lpObj->UseGuildMatching);

	PMSG_GUILD_MATCHING_JOIN_CANCEL_SEND pMsg;

	pMsg.header.set(0xED,0x05,sizeof(pMsg));

	pMsg.result = lpMsg->result;

	if(lpMsg->flag == 0){DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);}

	#endif
}

void CGuildMatching::DGGuildMatchingJoinAcceptRecv(SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGuildMatchingJoinAcceptRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpMsg->result == 1 && lpMsg->type == 1){GDGuildMemberAdd(-1,lpMsg->GuildName,lpMsg->MemberName);}

	PMSG_GUILD_MATCHING_JOIN_ACCEPT_SEND pMsg;

	pMsg.header.set(0xED,0x06,sizeof(pMsg));

	memcpy(pMsg.name,lpMsg->MemberName,sizeof(pMsg.name));

	pMsg.type = lpMsg->type;

	pMsg.result = ((lpMsg->result==1)?0:lpMsg->result);

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::DGGuildMatchingJoinListRecv(SDHP_GUILD_MATCHING_JOIN_LIST_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGuildMatchingJoinListRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	BYTE send[2048];

	PMSG_GUILD_MATCHING_JOIN_LIST_SEND pMsg;

	pMsg.header.set(0xED,0x07,0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	pMsg.result = lpMsg->result;

	PMSG_GUILD_MATCHING_JOIN_LIST info;

	for(DWORD n=0;n < lpMsg->count;n++)
	{
		SDHP_GUILD_MATCHING_JOIN_LIST* lpInfo = (SDHP_GUILD_MATCHING_JOIN_LIST*)(((BYTE*)lpMsg)+sizeof(SDHP_GUILD_MATCHING_JOIN_LIST_RECV)+(sizeof(SDHP_GUILD_MATCHING_JOIN_LIST)*n));

		memcpy(info.name,lpInfo->name,sizeof(info.name));

		info.Class = lpInfo->Class;

		info.Level = lpInfo->Level;

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(lpObj->Index,send,size);

	#endif
}

void CGuildMatching::DGGuildMatchingJoinInfoRecv(SDHP_GUILD_MATCHING_JOIN_INFO_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGuildMatchingJoinInfoRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	PMSG_GUILD_MATCHING_JOIN_INFO_SEND pMsg;

	pMsg.header.set(0xED,0x08,sizeof(pMsg));

	memcpy(pMsg.GuildMasterName,lpMsg->GuildMasterName,sizeof(pMsg.GuildMasterName));

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	pMsg.result = lpMsg->result;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::DGGuildMatchingNotifyRecv(SDHP_GUILD_MATCHING_NOTIFY_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGuildMatchingNotifyRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpMsg->result == 1){GDGuildMemberAdd(lpMsg->index,lpMsg->GuildName,lpMsg->name);}

	this->GDGuildMatchingJoinCancelSend(lpObj->Index,1);

	PMSG_GUILD_MATCHING_NOTIFY_SEND pMsg;

	pMsg.header.set(0xED,0x09,sizeof(pMsg));

	pMsg.result = lpMsg->result;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::DGGuildMatchingNotifyMasterRecv(SDHP_GUILD_MATCHING_NOTIFY_MASTER_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGuildMatchingNotifyMasterRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	PMSG_GUILD_MATCHING_NOTIFY_MASTER_SEND pMsg;

	pMsg.header.set(0xED,0x10,sizeof(pMsg));

	pMsg.result = lpMsg->result;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::GDGuildMatchingListSend(int aIndex,DWORD page) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_GUILD_MATCHING_LIST_SEND pMsg;

	pMsg.header.set(0x28,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.page = page;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::GDGuildMatchingListSearchSend(int aIndex,DWORD page,char* SearchWord) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_GUILD_MATCHING_LIST_SEARCH_SEND pMsg;

	pMsg.header.set(0x28,0x01,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.page = page;

	memcpy(pMsg.SearchWord,SearchWord,sizeof(pMsg.SearchWord));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::GDGuildMatchingInsertSend(int aIndex,char* text,BYTE InterestType,BYTE LevelRange,BYTE ClassType) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	if(gObj[aIndex].UseGuildMatching != 0)
	{
		return;
	}

	SDHP_GUILD_MATCHING_INSERT_SEND pMsg;

	pMsg.header.set(0x28,0x02,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	memcpy(pMsg.text,text,sizeof(pMsg.text));

	memcpy(pMsg.GuildName,gObj[aIndex].GuildName,sizeof(pMsg.GuildName));

	pMsg.GuildNumber = gObj[aIndex].GuildNumber;

	pMsg.GuildMemberCount = gObj[aIndex].Guild->Count;

	pMsg.GuildMasterLevel = ((gObj[aIndex].Level>=MAX_CHARACTER_LEVEL)?(gObj[aIndex].Level+gObj[aIndex].MasterLevel):gObj[aIndex].Level);

	pMsg.GuildMasterClass = gObj[aIndex].Class | ((gObj[aIndex].ChangeUp==1)?8:((gObj[aIndex].ChangeUp==2)?24:0));

	pMsg.InterestType = InterestType;

	pMsg.LevelRange = LevelRange;

	pMsg.ClassType = ClassType;

	pMsg.GensType = ((gServerInfo.m_GensSystemGuildLock==0)?GENS_FAMILY_NONE:gObj[aIndex].GensFamily);

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::GDGuildMatchingCancelSend(int aIndex,BYTE flag) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	if(gObj[aIndex].UseGuildMatching == 0)
	{
		return;
	}

	SDHP_GUILD_MATCHING_CANCEL_SEND pMsg;

	pMsg.header.set(0x28,0x03,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	memcpy(pMsg.GuildName,gObj[aIndex].GuildName,sizeof(pMsg.GuildName));

	pMsg.flag = flag;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::GDGuildMatchingJoinInsertSend(int aIndex,char* GuildName) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	if(gObj[aIndex].UseGuildMatchingJoin != 0)
	{
		return;
	}

	SDHP_GUILD_MATCHING_JOIN_INSERT_SEND pMsg;

	pMsg.header.set(0x28,0x04,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	pMsg.Class = (BYTE)gObj[aIndex].Class;

	pMsg.Level = ((gObj[aIndex].Level>=MAX_CHARACTER_LEVEL)?(gObj[aIndex].Level+gObj[aIndex].MasterLevel):gObj[aIndex].Level);

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::GDGuildMatchingJoinCancelSend(int aIndex,BYTE flag) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	if(gObj[aIndex].UseGuildMatchingJoin == 0)
	{
		return;
	}

	SDHP_GUILD_MATCHING_JOIN_CANCEL_SEND pMsg;

	pMsg.header.set(0x28,0x05,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.flag = flag;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::GDGuildMatchingJoinAcceptSend(int aIndex,BYTE type,char* name) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_GUILD_MATCHING_JOIN_ACCEPT_SEND pMsg;

	pMsg.header.set(0x28,0x06,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.type = type;

	memcpy(pMsg.GuildName,gObj[aIndex].GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.MemberName,name,sizeof(pMsg.MemberName));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::GDGuildMatchingJoinListSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_GUILD_MATCHING_JOIN_LIST_SEND pMsg;

	pMsg.header.set(0x28,0x07,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	memcpy(pMsg.GuildName,gObj[aIndex].GuildName,sizeof(pMsg.GuildName));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::GDGuildMatchingJoinInfoSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_GUILD_MATCHING_JOIN_INFO_SEND pMsg;

	pMsg.header.set(0x28,0x08,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGuildMatching::GDGuildMatchingInsertSaveSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObj[aIndex].GuildNumber == 0)
	{
		return;
	}

	if(gObj[aIndex].UseGuildMatching == 0)
	{
		return;
	}

	SDHP_GUILD_MATCHING_INSERT_SAVE_SEND pMsg;

	pMsg.header.set(0x28,0x30,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	memcpy(pMsg.GuildName,gObj[aIndex].GuildName,sizeof(pMsg.GuildName));

	pMsg.GuildMemberCount = gObj[aIndex].Guild->Count;

	pMsg.GuildMasterLevel = ((gObj[aIndex].Level>=MAX_CHARACTER_LEVEL)?(gObj[aIndex].Level+gObj[aIndex].MasterLevel):gObj[aIndex].Level);

	pMsg.GuildMasterClass = gObj[aIndex].Class | ((gObj[aIndex].ChangeUp==1)?8:((gObj[aIndex].ChangeUp==2)?24:0));

	pMsg.GensType = ((gServerInfo.m_GensSystemGuildLock==0)?GENS_FAMILY_NONE:gObj[aIndex].GensFamily);

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

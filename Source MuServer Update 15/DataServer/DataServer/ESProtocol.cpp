#include "stdafx.h"
#include "ESProtocol.h"
#include "CharacterManager.h"
#include "DataServer.h"
#include "GuildManager.h"
#include "Protect.h"
#include "ServerManager.h"
#include "SocketManager.h"
#include "Util.h"

void ESDataSend(int index,BYTE* lpMsg,int size)
{
	switch(lpMsg[0])
	{
		case 0xC1:
			{
				BYTE send[8192];

				PSBMSG_HEAD pMsg;

				pMsg.set(0xE0,lpMsg[2],(size=(size+1)));

				memcpy(&send[0],&pMsg,sizeof(pMsg));

				memcpy(&send[4],&lpMsg[3],(size-4));

				gSocketManager.DataSend(index,send,size);
			}
			break;
		case 0xC2:
			{
				BYTE send[8192];

				PSWMSG_HEAD pMsg;

				pMsg.set(0xE0,lpMsg[3],(size=(size+1)));

				memcpy(&send[0],&pMsg,sizeof(pMsg));

				memcpy(&send[5],&lpMsg[4],(size-5));

				gSocketManager.DataSend(index,send,size);
			}
			break;
	}
}

void ESDataRecv(int index,BYTE head,BYTE* lpMsg,int size) // OK
{
	switch(lpMsg[0])
	{
		case 0xC1:
			{
				BYTE recv[8192];

				PBMSG_HEAD pMsg;

				pMsg.set(lpMsg[3],(size=(size-1)));

				memcpy(&recv[0],&pMsg,sizeof(pMsg));

				memcpy(&recv[3],&lpMsg[4],(size-3));

				ExDBServerProtocolCore(index,pMsg.head,recv,size);
			}
			break;
		case 0xC2:
			{
				BYTE recv[8192];

				PWMSG_HEAD pMsg;

				pMsg.set(lpMsg[4],(size=(size-1)));

				memcpy(&recv[0],&pMsg,sizeof(pMsg));

				memcpy(&recv[4],&lpMsg[5],(size-4));

				ExDBServerProtocolCore(index,pMsg.head,recv,size);
			}
			break;
	}
}

void ExDBServerProtocolCore(int index,BYTE head,BYTE* lpMsg,int size) // OK
{
	PROTECT_START

	if (AdvancedLog != 0)
	{
		LogAdd(LOG_BLACK,"ESPROTOCOL: Head: %x, 1: %x, 2: %x, 3: %x, 4: %x",head,lpMsg[1],lpMsg[2],lpMsg[3],lpMsg[4]);
	}

	switch(head)
	{
		case 0x02:
			GDCharClose((SDHP_USERCLOSE*)lpMsg,index);
			break;
		case 0x30:
			GDGuildCreateSend((SDHP_GUILDCREATE*)lpMsg,index);
			break;
		case 0x31:
			GDGuildDestroySend((SDHP_GUILDDESTROY*)lpMsg,index);
			break;
		case 0x32:
			GDGuildMemberAdd((SDHP_GUILDMEMBERADD*)lpMsg,index);
			break;
		case 0x33:
			GDGuildMemberDel((SDHP_GUILDMEMBERDEL*)lpMsg,index);
			break;
		case 0x35:
			DGGuildMemberInfoRequest((SDHP_GUILDMEMBER_INFO_REQUEST*)lpMsg,index);
			break;
		case 0x37:
			DGGuildScoreUpdate((SDHP_GUILDSCOREUPDATE*)lpMsg,index);
			break;
		case 0x38:
			GDGuildNoticeSave((SDHP_GUILDNOTICE*)lpMsg,index);
			break;
		case 0x50:
			GDGuildServerGroupChattingSend((EXSDHP_SERVERGROUP_GUILD_CHATTING_SEND*)lpMsg,index);
			break;
		case 0x51:
			GDUnionServerGroupChattingSend((EXSDHP_SERVERGROUP_UNION_CHATTING_SEND*)lpMsg,index);
			break;
		case 0xE1:
			GDGuildReqAssignStatus((EXSDHP_GUILD_ASSIGN_STATUS_REQ*)lpMsg,index);
			break;
		case 0xE2:
			GDGuildReqAssignType((EXSDHP_GUILD_ASSIGN_TYPE_REQ*)lpMsg,index);
			break;
		case 0xE5:
			GDRelationShipReqJoin((EXSDHP_RELATIONSHIP_JOIN_REQ*)lpMsg,index);
			break;
		case 0xE6:
			GDUnionBreakOff((EXSDHP_RELATIONSHIP_BREAKOFF_REQ*)lpMsg,index);
			break;
		case 0xE9:
			GDUnionListSend((EXSDHP_UNION_LIST_REQ*)lpMsg,index);
			break;
		case 0xEB:
			switch(lpMsg[3])
			{
				case 0x01:
					GDRelationShipReqKickOutUnionMember((EXSDHP_KICKOUT_UNIONMEMBER_REQ*)lpMsg,index);
					break;
			}
			break;
	}

	PROTECT_FINAL
}

void GDCharClose(SDHP_USERCLOSE* lpMsg,int index)
{
	SDHP_USERCLOSE pMsg;

	pMsg.h.set(0x02,sizeof(pMsg));

	memcpy(pMsg.CharName,lpMsg->CharName,sizeof(pMsg.CharName));

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	pMsg.Type = 0;

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void GDGuildCreateSend(SDHP_GUILDCREATE* lpMsg,int index)
{
	SDHP_GUILDCREATE_RESULT pMsg;

	pMsg.h.set(0x30,sizeof(pMsg));

	pMsg.Result = 0;

	pMsg.Flag = 1;

	pMsg.GuildNumber = 0;

	pMsg.NumberH = lpMsg->NumberH;

	pMsg.NumberL = lpMsg->NumberL;

	memcpy(pMsg.Master,lpMsg->Master,sizeof(pMsg.Master));

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Mark,lpMsg->Mark,sizeof(pMsg.Mark));

	pMsg.btGuildType = lpMsg->btGuildType;

	if((pMsg.Result=gGuildManager.AddGuild(index,lpMsg->GuildName,lpMsg->Master,lpMsg->Mark,lpMsg->btGuildType)) != 1)
	{
		ESDataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	GUILD_INFO* lpGuildInfo = gGuildManager.GetGuildInfo(lpMsg->GuildName);

	GUILD_MEMBER_INFO* lpGuildMemberInfo = gGuildManager.GetGuildMemberInfo(lpMsg->Master);

	if(lpGuildInfo != 0 && lpGuildMemberInfo != 0)
	{
		pMsg.GuildNumber = lpGuildInfo->dwNumber;

		for(int n=0;n < MAX_SERVER;n++)
		{
			if(gServerManager[n].CheckState() != 0)
			{
				pMsg.Flag = ((n==index)?1:0);
				ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
			}
		}

		gGuildManager.ConnectMember(lpMsg->Master,GetServerCodeByName(lpMsg->Master));

		DGGuildMasterListRecv(index,lpGuildInfo->dwNumber);

		DGRelationShipListRecv(index,lpGuildInfo->dwUnionNumber,1);

		DGRelationShipListRecv(index,lpGuildInfo->dwRivalNumber,2);

		DGGuildMemberInfo(index,lpMsg->GuildName,lpMsg->Master,lpGuildMemberInfo->btStatus,lpMsg->btGuildType,(BYTE)lpGuildMemberInfo->btServer);
	}
}

void GDGuildDestroySend(SDHP_GUILDDESTROY* lpMsg,int index)
{
	SDHP_GUILDDESTROY_RESULT pMsg;

	pMsg.h.set(0x31,sizeof(pMsg));

	pMsg.Result = 0;

	pMsg.Flag = 1;

	pMsg.NumberH = lpMsg->NumberH;

	pMsg.NumberL = lpMsg->NumberL;

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Master,lpMsg->Master,sizeof(pMsg.Master));

	if((pMsg.Result=gGuildManager.DelGuild(index,lpMsg->GuildName)) != 1)
	{
		ESDataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.Flag = ((n==index)?1:0);
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void GDGuildMemberAdd(SDHP_GUILDMEMBERADD* lpMsg,int index)
{
	SDHP_GUILDMEMBERADD_RESULT pMsg;

	pMsg.h.set(0x32,sizeof(pMsg));

	pMsg.Result = 0;

	pMsg.Flag = 1;

	pMsg.NumberH = lpMsg->NumberH;

	pMsg.NumberL = lpMsg->NumberL;

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.MemberID,lpMsg->MemberID,sizeof(pMsg.MemberID));

	pMsg.pServer = GetServerCodeByName(lpMsg->MemberID);

	if((pMsg.Result=gGuildManager.AddGuildMember(index,lpMsg->GuildName,lpMsg->MemberID,0x00,pMsg.pServer)) != 1)
	{
		ESDataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.Flag = ((n==index)?1:0);
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void GDGuildMemberDel(SDHP_GUILDMEMBERDEL* lpMsg,int index)
{
	SDHP_GUILDMEMBERDEL_RESULT pMsg;

	pMsg.h.set(0x33,sizeof(pMsg));

	pMsg.Result = 0;

	pMsg.Flag = 1;

	pMsg.NumberH = lpMsg->NumberH;

	pMsg.NumberL = lpMsg->NumberL;

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.MemberID,lpMsg->MemberID,sizeof(pMsg.MemberID));

	if((pMsg.Result=gGuildManager.DelGuildMember(index,lpMsg->MemberID)) != 1)
	{
		ESDataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.Flag = ((n==index)?1:0);
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void DGGuildMemberInfoRequest(SDHP_GUILDMEMBER_INFO_REQUEST* lpMsg,int index)
{
	GUILD_INFO* lpGuildInfo = gGuildManager.GetMemberGuildInfo(lpMsg->MemberID);

	GUILD_MEMBER_INFO* lpGuildMemberInfo = gGuildManager.GetGuildMemberInfo(lpMsg->MemberID);

	if(lpGuildInfo == 0 || lpGuildMemberInfo == 0)
	{
		return;
	}

	gGuildManager.ConnectMember(lpMsg->MemberID,GetServerCodeByName(lpMsg->MemberID));

	DGGuildMasterListRecv(index,lpGuildInfo->dwNumber);

	DGRelationShipListRecv(index,lpGuildInfo->dwUnionNumber,1);

	DGRelationShipListRecv(index,lpGuildInfo->dwRivalNumber,2);

	DGGuildMemberInfo(index,lpGuildInfo->szName,lpMsg->MemberID,lpGuildMemberInfo->btStatus,lpGuildInfo->btType,(BYTE)lpGuildMemberInfo->btServer);
}

void DGGuildScoreUpdate(SDHP_GUILDSCOREUPDATE* lpMsg,int index)
{
	if(gGuildManager.SetGuildScore(lpMsg->GuildName,lpMsg->Score) != 0)
	{
		SDHP_GUILDSCOREUPDATE pMsg;

		pMsg.h.set(0x37,sizeof(pMsg));

		memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

		pMsg.Score = lpMsg->Score;

		for(int n=0;n < MAX_SERVER;n++)
		{
			if(gServerManager[n].CheckState() != 0)
			{
				ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
			}
		}
	}
}

void GDGuildNoticeSave(SDHP_GUILDNOTICE* lpMsg,int index)
{
	if(gGuildManager.SetGuildNotice(lpMsg->GuildName,lpMsg->szGuildNotice) != 0)
	{
		SDHP_GUILDNOTICE pMsg;

		pMsg.h.set(0x38,sizeof(pMsg));

		memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

		memcpy(pMsg.szGuildNotice,lpMsg->szGuildNotice,sizeof(pMsg.szGuildNotice));

		for(int n=0;n < MAX_SERVER;n++)
		{
			if(gServerManager[n].CheckState() != 0)
			{
				ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
			}
		}
	}
}

void GDGuildServerGroupChattingSend(EXSDHP_SERVERGROUP_GUILD_CHATTING_SEND* lpMsg,int index)
{
	EXSDHP_SERVERGROUP_GUILD_CHATTING_RECV pMsg;

	pMsg.h.set(0x50,sizeof(pMsg));

	pMsg.iGuildNum = lpMsg->iGuildNum;

	memcpy(pMsg.szCharacterName,lpMsg->szCharacterName,sizeof(pMsg.szCharacterName));

	memcpy(pMsg.szChattingMsg,lpMsg->szChattingMsg,sizeof(pMsg.szChattingMsg));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void GDUnionServerGroupChattingSend(EXSDHP_SERVERGROUP_UNION_CHATTING_SEND* lpMsg,int index)
{
	EXSDHP_SERVERGROUP_UNION_CHATTING_RECV pMsg;

	pMsg.h.set(0x51,sizeof(pMsg));

	pMsg.iUnionNum = lpMsg->iUnionNum;

	memcpy(pMsg.szCharacterName,lpMsg->szCharacterName,sizeof(pMsg.szCharacterName));

	memcpy(pMsg.szChattingMsg,lpMsg->szChattingMsg,sizeof(pMsg.szChattingMsg));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void GDGuildReqAssignStatus(EXSDHP_GUILD_ASSIGN_STATUS_REQ* lpMsg,int index)
{
	EXSDHP_GUILD_ASSIGN_STATUS_RESULT pMsg;

	pMsg.h.set(0xE1,sizeof(pMsg));

	pMsg.btFlag = 1;

	pMsg.wUserIndex = lpMsg->wUserIndex;

	pMsg.btType = lpMsg->btType;

	pMsg.btResult = gGuildManager.SetGuildMemberStatus(lpMsg->szTargetName,lpMsg->btGuildStatus);

	pMsg.btGuildStatus = lpMsg->btGuildStatus;

	memcpy(pMsg.szGuildName,lpMsg->szGuildName,sizeof(pMsg.szGuildName));

	memcpy(pMsg.szTargetName,lpMsg->szTargetName,sizeof(pMsg.szTargetName));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.btFlag = ((n==index)?1:0);
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void GDGuildReqAssignType(EXSDHP_GUILD_ASSIGN_TYPE_REQ* lpMsg,int index)
{
	EXSDHP_GUILD_ASSIGN_TYPE_RESULT pMsg;

	pMsg.h.set(0xE2,sizeof(pMsg));

	pMsg.btFlag = 1;

	pMsg.wUserIndex = lpMsg->wUserIndex;

	pMsg.btGuildType = lpMsg->btGuildType;

	pMsg.btResult = gGuildManager.SetGuildType(lpMsg->szGuildName,lpMsg->btGuildType);

	memcpy(pMsg.szGuildName,lpMsg->szGuildName,sizeof(pMsg.szGuildName));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.btFlag = ((n==index)?1:0);
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void GDRelationShipReqJoin(EXSDHP_RELATIONSHIP_JOIN_REQ* lpMsg,int index)
{
	EXSDHP_RELATIONSHIP_JOIN_RESULT pMsg;

	pMsg.h.set(0xE5,sizeof(pMsg));

	pMsg.btFlag = 1;

	pMsg.wRequestUserIndex = lpMsg->wRequestUserIndex;

	pMsg.wTargetUserIndex = lpMsg->wTargetUserIndex;

	pMsg.btResult = 0;

	pMsg.btRelationShipType = lpMsg->btRelationShipType;

	pMsg.iRequestGuildNum = lpMsg->iRequestGuildNum;

	pMsg.iTargetGuildNum = lpMsg->iTargetGuildNum;

	GUILD_INFO* lpSourceGuildInfo;

	GUILD_INFO* lpTargetGuildInfo;

	if((lpSourceGuildInfo=gGuildManager.GetGuildInfo(lpMsg->iRequestGuildNum)) == 0 || (lpTargetGuildInfo=gGuildManager.GetGuildInfo(lpMsg->iTargetGuildNum)) == 0)
	{
		ESDataSend(index,(BYTE*)&pMsg,pMsg.h.size);
		return;
	}

	if((pMsg.btResult=gGuildManager.AddGuildRelationship(index,lpMsg->iRequestGuildNum,lpMsg->iTargetGuildNum,lpMsg->btRelationShipType)) != 1)
	{
		ESDataSend(index,(BYTE*)&pMsg,pMsg.h.size);
		return;
	}

	memcpy(pMsg.szRequestGuildName,lpSourceGuildInfo->szName,sizeof(pMsg.szRequestGuildName));

	memcpy(pMsg.szTargetGuildName,lpTargetGuildInfo->szName,sizeof(pMsg.szTargetGuildName));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.btFlag = ((n==index)?1:0);
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void GDUnionBreakOff(EXSDHP_RELATIONSHIP_BREAKOFF_REQ* lpMsg,int index)
{
	EXSDHP_RELATIONSHIP_BREAKOFF_RESULT pMsg;

	pMsg.h.set(0xE6,sizeof(pMsg));

	pMsg.btFlag = 1;

	pMsg.wRequestUserIndex = lpMsg->wRequestUserIndex;

	pMsg.wTargetUserIndex = lpMsg->wTargetUserIndex;

	pMsg.btResult = 0;

	pMsg.btRelationShipType = lpMsg->btRelationShipType;

	pMsg.iRequestGuildNum = lpMsg->iRequestGuildNum;

	pMsg.iTargetGuildNum = lpMsg->iTargetGuildNum;

	if((pMsg.btResult=gGuildManager.DelGuildRelationship(index,lpMsg->iRequestGuildNum,lpMsg->btRelationShipType)) != 1)
	{
		ESDataSend(index,(BYTE*)&pMsg,pMsg.h.size);
		return;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.btFlag = ((n==index)?1:0);
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void GDUnionListSend(EXSDHP_UNION_LIST_REQ* lpMsg,int index)
{
	BYTE send[2048];

	EXSDHP_UNION_LIST_COUNT pMsg;

	pMsg.h.set(0xE9,0);

	int size = sizeof(pMsg);

	pMsg.btCount = 0;

	pMsg.btResult = 0;

	pMsg.wRequestUserIndex = lpMsg->wRequestUserIndex;

	pMsg.iTimeStamp = 0;

	pMsg.btRivalMemberNum = 0;

	pMsg.btUnionMemberNum = 0;

	GUILD_INFO* lpGuildInfo = gGuildManager.GetGuildInfo(lpMsg->iUnionMasterGuildNumber);

	if(lpGuildInfo != 0)
	{
		pMsg.btResult = 1;

		EXSDHP_UNION_LIST UnionList;

		DWORD GuildUnionNumber[MAX_GUILD_UNION];

		DWORD GuildRivalNumber[MAX_GUILD_RIVAL];

		pMsg.btUnionMemberNum = (BYTE)gGuildManager.GetUnionList(lpGuildInfo->dwUnionNumber,GuildUnionNumber);

		pMsg.btRivalMemberNum = (BYTE)gGuildManager.GetRivalList(lpGuildInfo->dwRivalNumber,GuildRivalNumber);

		for(int n=0;n < pMsg.btUnionMemberNum;n++)
		{
			GUILD_INFO* lpGuildList = gGuildManager.GetGuildInfo(GuildUnionNumber[n]);

			if(lpGuildList != 0)
			{
				UnionList.btMemberNum = lpGuildList->GetMemberCount();

				memcpy(UnionList.szGuildName,lpGuildList->szName,sizeof(UnionList.szGuildName));

				memcpy(UnionList.Mark,lpGuildList->arMark,sizeof(UnionList.Mark));

				memcpy(&send[size],&UnionList,sizeof(UnionList));

				size += sizeof(UnionList);

				pMsg.btCount++;
			}
		}
	}

	pMsg.h.size[0] = SET_NUMBERHB(size);
	pMsg.h.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	ESDataSend(index,send,size);
}

void GDRelationShipReqKickOutUnionMember(EXSDHP_KICKOUT_UNIONMEMBER_REQ* lpMsg,int index)
{
	EXSDHP_KICKOUT_UNIONMEMBER_RESULT pMsg;

	pMsg.h.set(0xEB,0x01,sizeof(pMsg));

	pMsg.btFlag = 1;

	pMsg.wRequestUserIndex = lpMsg->wRequestUserIndex;

	pMsg.btRelationShipType = lpMsg->btRelationShipType;

	pMsg.btResult = 0;

	memcpy(pMsg.szUnionMasterGuildName,lpMsg->szUnionMasterGuildName,sizeof(pMsg.szUnionMasterGuildName));

	memcpy(pMsg.szUnionMemberGuildName,lpMsg->szUnionMemberGuildName,sizeof(pMsg.szUnionMemberGuildName));

	if((pMsg.btResult=gGuildManager.SetGuildRelationship(index,lpMsg->szUnionMemberGuildName,lpMsg->szUnionMasterGuildName)) != 1)
	{
		ESDataSend(index,(BYTE*)&pMsg,pMsg.h.size);
		return;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.btFlag = ((n==index)?1:0);
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void DGGuildMemberInfo(int index,char* GuildName,char* MemberID,BYTE Status,BYTE Type,BYTE Server)
{
	SDHP_GUILDMEMBER_INFO pMsg;

	pMsg.h.set(0x35,sizeof(pMsg));

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.MemberID,MemberID,sizeof(pMsg.MemberID));

	pMsg.btGuildStatus = Status;

	pMsg.btGuildType = Type;

	pMsg.pServer = Server;

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void DGGuildMasterListRecv(int index,int GuildNumber)
{
	GUILD_INFO* lpGuildInfo = gGuildManager.GetGuildInfo(GuildNumber);

	if(lpGuildInfo == 0)
	{
		return;
	}

	BYTE send[2048];

	SDHP_GUILDALL_COUNT pMsg;

	pMsg.h.set(0x36,0);

	int size = sizeof(pMsg);

	pMsg.Number = lpGuildInfo->dwNumber;

	memcpy(pMsg.GuildName,lpGuildInfo->szName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Master,lpGuildInfo->szMaster,sizeof(pMsg.Master));

	memcpy(pMsg.Mark,lpGuildInfo->arMark,sizeof(pMsg.Mark));

	pMsg.score = lpGuildInfo->dwScore;

	pMsg.btGuildType = lpGuildInfo->btType;

	pMsg.iGuildUnion = lpGuildInfo->dwUnionNumber;

	pMsg.iGuildRival = lpGuildInfo->dwRivalNumber;

	GUILD_INFO* lpRivalInfo = gGuildManager.GetGuildInfo(lpGuildInfo->dwRivalNumber);

	if(lpRivalInfo == 0)
	{
		memset(pMsg.szGuildRivalName,0,sizeof(pMsg.szGuildRivalName));
	}
	else
	{
		memcpy(pMsg.szGuildRivalName,lpRivalInfo->szName,sizeof(pMsg.szGuildRivalName));
	}

	pMsg.Count = 0;

	SDHP_GUILDALL GuildList;

	for(int n=0;n < MAX_GUILD_MEMBER;n++)
	{
		GUILD_MEMBER_INFO* lpGuildMemberInfo = &lpGuildInfo->arGuildMember[n];

		if(lpGuildMemberInfo->IsEmpty() == 0)
		{
			memcpy(GuildList.MemberID,lpGuildMemberInfo->szGuildMember,sizeof(GuildList.MemberID));

			GuildList.btGuildStatus = lpGuildMemberInfo->btStatus;

			GuildList.pServer = lpGuildMemberInfo->btServer;

			memcpy(&send[size],&GuildList,sizeof(GuildList));

			size += sizeof(GuildList);

			pMsg.Count++;
		}
	}

	pMsg.h.size[0] = SET_NUMBERHB(size);
	pMsg.h.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			ESDataSend(n,send,size);
		}
	}
}

void DGRelationShipListRecv(int index,int GuildNumber,int RelationshipType)
{
	GUILD_INFO* lpGuildInfo = gGuildManager.GetGuildInfo(GuildNumber);

	if(lpGuildInfo == 0)
	{
		return;
	}

	EXSDHP_UNION_RELATIONSHIP_LIST pMsg;

	pMsg.h.set(0xE7,sizeof(pMsg));

	pMsg.btFlag = 1;

	pMsg.btRelationShipType = RelationshipType;

	pMsg.btRelationShipMemberCount = 0;

	if(RelationshipType == 1){pMsg.btRelationShipMemberCount = (BYTE)gGuildManager.GetUnionList(lpGuildInfo->dwNumber,(DWORD*)pMsg.iRelationShipMember);}

	if(RelationshipType == 2){pMsg.btRelationShipMemberCount = (BYTE)gGuildManager.GetRivalList(lpGuildInfo->dwNumber,(DWORD*)pMsg.iRelationShipMember);}

	memcpy(pMsg.szUnionMasterGuildName,lpGuildInfo->szName,sizeof(pMsg.szUnionMasterGuildName));

	pMsg.iUnionMasterGuildNumber = lpGuildInfo->dwNumber;

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.btFlag = ((n==index)?1:0);
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void DGRelationShipNotificationRecv(int index,int UpdateFlag,int GuildListCount,int* GuildList)
{
	EXSDHP_NOTIFICATION_RELATIONSHIP pMsg;

	pMsg.h.set(0xE8,sizeof(pMsg));

	pMsg.btFlag = 1;

	pMsg.btUpdateFlag = UpdateFlag;

	pMsg.btGuildListCount = GuildListCount;

	memcpy(pMsg.iGuildList,GuildList,(GuildListCount*sizeof(int)));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.btFlag = ((n==index)?1:0);
			ESDataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

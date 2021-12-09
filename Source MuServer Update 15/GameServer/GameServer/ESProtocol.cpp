#include "stdafx.h"
#include "ESProtocol.h"
#include "CastleSiege.h"
#include "CastleSiegeSync.h"
#include "GameMain.h"
#include "Guild.h"
#include "GuildClass.h"
#include "Message.h"
#include "Notice.h"
#include "Protect.h"
#include "ServerInfo.h"
#include "Union.h"
#include "Util.h"
#include "Viewport.h"

void ESDataSend(BYTE* lpMsg,int size)
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

				gDataServerConnection.DataSend(send,size);
			}
			break;
		case 0xC2:
			{
				BYTE send[8192];

				PSWMSG_HEAD pMsg;

				pMsg.set(0xE0,lpMsg[3],(size=(size+1)));

				memcpy(&send[0],&pMsg,sizeof(pMsg));

				memcpy(&send[5],&lpMsg[4],(size-5));

				gDataServerConnection.DataSend(send,size);
			}
			break;
	}
}

void ESDataRecv(BYTE head,BYTE* lpMsg,int size) // OK
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

				ExDBServerProtocolCore(pMsg.head,recv,size);
			}
			break;
		case 0xC2:
			{
				BYTE recv[8192];

				PWMSG_HEAD pMsg;

				pMsg.set(lpMsg[4],(size=(size-1)));

				memcpy(&recv[0],&pMsg,sizeof(pMsg));

				memcpy(&recv[4],&lpMsg[5],(size-4));

				ExDBServerProtocolCore(pMsg.head,recv,size);
			}
			break;
	}
}

void ExDBServerProtocolCore(BYTE head,BYTE* lpMsg,int size) // OK
{
	PROTECT_START

	BYTE* aRecv = lpMsg;

	switch(head)
	{
		case 0x02:
			GDCharCloseRecv((SDHP_USERCLOSE *)aRecv);
			break;
		case 0x30:
			GDGuildCreateRecv((SDHP_GUILDCREATE_RESULT *)aRecv);
			break;
		case 0x31:
			GDGuildDestroyRecv((SDHP_GUILDDESTROY_RESULT *)aRecv);
			break;
		case 0x32:
			GDGuildMemberAddResult((SDHP_GUILDMEMBERADD_RESULT *)aRecv);
			break;
		case 0x33:
			GDGuildMemberDelResult((SDHP_GUILDMEMBERDEL_RESULT *)aRecv);
			break;
		case 0x35:
			DGGuildMemberInfo((SDHP_GUILDMEMBER_INFO *)aRecv);
			break;
		case 0x36:
			DGGuildMasterListRecv(aRecv);
			break;
		case 0x37:
			GDGuildScoreUpdateRecv((SDHP_GUILDSCOREUPDATE *)aRecv);
			break;
		case 0x38:
			DGGuildNoticeRecv((SDHP_GUILDNOTICE *)aRecv);
			break;
		case 0x50:
			DGGuildServerGroupChattingRecv((EXSDHP_SERVERGROUP_GUILD_CHATTING_RECV *)aRecv);
			break;
		case 0x51:
			DGUnionServerGroupChattingRecv((EXSDHP_SERVERGROUP_UNION_CHATTING_RECV *)aRecv);
			break;
		case 0xE1:
			DGGuildAnsAssignStatus((EXSDHP_GUILD_ASSIGN_STATUS_RESULT *)aRecv);
			break;
		case 0xE2:
			DGGuildAnsAssignType((EXSDHP_GUILD_ASSIGN_TYPE_RESULT *)aRecv);
			break;
		case 0xE5:
			DGRelationShipAnsJoin((EXSDHP_RELATIONSHIP_JOIN_RESULT *)aRecv);
			break;
		case 0xE6:
			DGRelationShipAnsBreakOff((EXSDHP_RELATIONSHIP_BREAKOFF_RESULT *)aRecv);
			break;
		case 0xE7:
			DGRelationShipListRecv((EXSDHP_UNION_RELATIONSHIP_LIST *)aRecv);
			break;
		case 0xE8:
			DGRelationShipNotificationRecv((EXSDHP_NOTIFICATION_RELATIONSHIP *)aRecv);
			break;
		case 0xE9:
			DGUnionListRecv(aRecv);
			break;
		case 0xEB:
			switch(lpMsg[3])
			{
				case 0x01:
					DGRelationShipAnsKickOutUnionMember((EXSDHP_KICKOUT_UNIONMEMBER_RESULT *)aRecv);
					break;
			}
			break;
	}

	PROTECT_FINAL
}

void GDCharClose(int type, char* GuildName, char* Name)
{
	SDHP_USERCLOSE pMsg;

	pMsg.h.type = 0xC1;
	pMsg.h.size = sizeof(pMsg);
	pMsg.h.head = 0x02;
	pMsg.Type = type;
	memcpy(pMsg.CharName, Name, sizeof(pMsg.CharName));
	memset(pMsg.GuildName, 0, sizeof(pMsg.GuildName));

	if ( GuildName != NULL )
	{
		memcpy(pMsg.GuildName, GuildName, sizeof(pMsg.GuildName));
	}

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GDCharCloseRecv(SDHP_USERCLOSE * lpMsg )
{
	char szName[11];
	char szGuildName[9];

	memset(szName, 0, sizeof(szName));
	memset(szGuildName, 0, sizeof(szGuildName));
	memcpy(szName, lpMsg->CharName, sizeof(szName));
	memcpy(szGuildName, lpMsg->GuildName, sizeof(szGuildName));

	gGuildClass.CloseMember(szGuildName, szName);
}

void GDGuildCreateSend(int aIndex, char* Name, char* Master,BYTE* Mark, int iGuildType)
{
	SDHP_GUILDCREATE pMsg;

	pMsg.h.type = 0xC1;
	pMsg.h.head = 0x30;
	pMsg.h.size = sizeof(pMsg);

	memcpy(pMsg.Mark, Mark, sizeof(pMsg.Mark));
	memcpy(pMsg.Master, Master, sizeof(pMsg.Master));
	memcpy(pMsg.GuildName, Name, sizeof(pMsg.GuildName));
	pMsg.NumberH = SET_NUMBERHB(aIndex);
	pMsg.NumberL = SET_NUMBERLB(aIndex);
	pMsg.btGuildType = iGuildType;

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GDGuildCreateRecv(SDHP_GUILDCREATE_RESULT * lpMsg)
{
	int aIndex = -1;
	PMSG_GUILDCREATED_RESULT pMsg;
	GUILD_INFO_STRUCT * lpNode;
	char szMaster[11];
	char szGuildName[9];

	memcpy(szMaster, lpMsg->Master, sizeof(szMaster));
	memcpy(szGuildName, lpMsg->GuildName, sizeof(szGuildName));

	if (lpMsg->Result == 0 )
	{
		aIndex = MAKE_NUMBERW(lpMsg->NumberH, lpMsg->NumberL);

		PMSG_GUILDCREATED_RESULT pResult;

		pResult.h.set(0x56, sizeof(pResult));
		pResult.Result = G_ERROR_NONE;
		pResult.btGuildType = lpMsg->btGuildType;

		DataSend(aIndex, (LPBYTE)&pResult, pResult.h.size);

		if ( gObj[aIndex].Interface.use != 0 && gObj[aIndex].Interface.type == 5 )
		{
			gObj[aIndex].Interface.use = 0;
		}
		return;
	}

	if ( lpMsg->Flag == 1 )
	{
		aIndex = MAKE_NUMBERW(lpMsg->NumberH, lpMsg->NumberL);

		if ( gObjIsConnected(aIndex) == TRUE )
		{
			if ( szMaster[0] == gObj[aIndex].Name[0] )
			{
				if ( strcmp(szMaster, gObj[aIndex].Name) == 0 )
				{
					pMsg.h.type = 0xC1;
					pMsg.h.head = 0x56;
					pMsg.h.size = sizeof(pMsg);
					pMsg.Result = lpMsg->Result;
					pMsg.btGuildType = lpMsg->btGuildType;

					DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
				}
			}
		}
	}

	if ( lpMsg->Result == 1 )
	{
		lpNode = gGuildClass.AddGuild(lpMsg->GuildNumber, szGuildName, lpMsg->Mark, szMaster, 0);

		if ( aIndex >= 0 && lpNode != NULL )
		{
			gObj[aIndex].Guild = lpNode;
			gObj[aIndex].GuildNumber = lpMsg->GuildNumber;
			gObj[aIndex].GuildStatus = G_MASTER;
			lpNode->GuildRival = 0;
			lpNode->GuildUnion = 0;
			lpNode->TimeStamp = 0;
			gObj[aIndex].GuildUnionTimeStamp = 0;
			memcpy(gObj[aIndex].GuildName,szGuildName,sizeof(gObj[aIndex].GuildName));
			gViewport.GCViewportSimpleGuildSend(&gObj[aIndex]);
		}
	}
}

void GDGuildDestroySend(int aIndex, LPSTR Name, LPSTR Master)
{
	SDHP_GUILDDESTROY pMsg;

	pMsg.h.type = 0xC1;
	pMsg.h.head = 0x31;
	pMsg.h.size = sizeof(pMsg);
	pMsg.NumberH = SET_NUMBERHB(aIndex);
	pMsg.NumberL = SET_NUMBERLB(aIndex);
	memcpy(pMsg.Master, Master, sizeof(pMsg.Master));
	memcpy(pMsg.GuildName, Name, sizeof(pMsg.GuildName));

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GDGuildDestroyRecv(SDHP_GUILDDESTROY_RESULT * lpMsg)
{
	int aIndex = -1;
	char szMaster[11];
	char szGuildName[9];

	memcpy(szMaster, lpMsg->Master, sizeof(szMaster));
	memcpy(szGuildName, lpMsg->GuildName, sizeof(szGuildName));

	if ( lpMsg->Flag == 1 )
	{
		aIndex = MAKE_NUMBERW(lpMsg->NumberH, lpMsg->NumberL);

		if ( gObjIsConnected(aIndex) == TRUE )
		{
			if ( szMaster[0] == gObj[aIndex].Name[0] )
			{
				if ( strcmp(szMaster, gObj[aIndex].Name) == 0 )
				{
					GCResultSend(aIndex, 0x53, lpMsg->Result);

					if ( lpMsg->Result == 1 || lpMsg->Result == 4 )
					{
						GCGuildViewportDelNow(aIndex, TRUE);
					}
				}
			}
		}
	}

	if ( lpMsg->Result == 1 || lpMsg->Result == 4)
	{
		GUILD_INFO_STRUCT * lpGuild = gGuildClass.SearchGuild(szGuildName);

		if ( lpGuild == NULL )
		{
			return;
		}

		GUILD_INFO_STRUCT * lpRival = gGuildClass.SearchGuild_Number(lpGuild->GuildRival);

		if ( lpRival != NULL )
		{
			lpRival->GuildRival = 0;
			lpRival->GuildRivalName[0] = 0;
		}

		lpGuild->GuildUnion = 0;
		lpGuild->GuildRival = 0;

		for ( int n=0;n<MAX_GUILD_USER;n++)
		{
			int iGuildMemberIndex = lpGuild->Index[n];

			if ( lpGuild->Use[n] > 0 && iGuildMemberIndex != -1 )
			{
				LPOBJ lpObj = &gObj[iGuildMemberIndex];
				
				if ( lpObj == NULL )
				{
					continue;
				}

				if ( gObjIsConnected(iGuildMemberIndex) == FALSE )
				{
					continue;
				}

				gObjNotifyUpdateUnionV1(lpObj);
				gObjNotifyUpdateUnionV2(lpObj);
				lpObj->Guild = NULL;
				lpObj->GuildNumber = 0;
				lpObj->GuildStatus = -1;
				lpObj->GuildUnionTimeStamp = 0;
				memset(lpObj->GuildName,0,sizeof(lpObj->GuildName));

				GCResultSend(n,0x53, 1);
				GCGuildViewportDelNow(lpObj->Index, FALSE);
			}
		}

		gUnionManager.DelUnion(lpGuild->Number);
		gGuildClass.DeleteGuild(szGuildName, szMaster);
	}
}

void GDGuildMemberAdd(int aIndex, LPSTR Name, LPSTR MemberId)
{
	SDHP_GUILDMEMBERADD pMsg;

	pMsg.h.type = 0xC1;
	pMsg.h.head = 0x32;
	pMsg.h.size = sizeof(pMsg);
	pMsg.NumberH = SET_NUMBERHB(aIndex);
	pMsg.NumberL = SET_NUMBERLB(aIndex);
	memcpy(pMsg.MemberID, MemberId, sizeof(pMsg.MemberID));
	memcpy(pMsg.GuildName, Name, sizeof(pMsg.GuildName));

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GDGuildMemberAddResult(SDHP_GUILDMEMBERADD_RESULT * lpMsg)
{
	GUILD_INFO_STRUCT * lpNode;
	int aIndex = -1;
	int HereUserIndex=-1;
	char szMember[11];
	char szGuildName[9];

	memcpy(szMember, lpMsg->MemberID, sizeof(szMember));
	memcpy(szGuildName, lpMsg->GuildName, sizeof(szGuildName));

	if ( lpMsg->Flag == 1 )
	{
		aIndex = MAKE_NUMBERW(lpMsg->NumberH, lpMsg->NumberL);

		if ( gObjIsConnected(aIndex) == TRUE )
		{
			if ( strcmp(szMember, gObj[aIndex].Name) == 0 )
			{
				GCResultSend(aIndex, 0x51, lpMsg->Result);
				HereUserIndex = aIndex;
			}
		}
	}

	if ( lpMsg->Result == 1 )
	{
		lpNode = gGuildClass.AddMember(szGuildName, szMember, HereUserIndex, -1, 0, lpMsg->pServer);

		if ( HereUserIndex >= 0 && lpNode != NULL )
		{
			gObj[HereUserIndex].Guild = lpNode;
			gObj[HereUserIndex].GuildStatus = 0;
			gObj[HereUserIndex].GuildNumber = lpNode->Number;
			gObj[HereUserIndex].GuildUnionTimeStamp = 0;
			memcpy(gObj[HereUserIndex].GuildName,szGuildName,sizeof(gObj[HereUserIndex].GuildName));
			gViewport.GCViewportSimpleGuildSend(&gObj[HereUserIndex]);
		}
	}
}

void GDGuildMemberDel(int aIndex, LPSTR Name, LPSTR MemberId)
{
	SDHP_GUILDMEMBERDEL pMsg;

	pMsg.h.type = 0xC1;
	pMsg.h.head = 0x33;
	pMsg.h.size = sizeof(pMsg);
	pMsg.NumberH = SET_NUMBERHB(aIndex);
	pMsg.NumberL = SET_NUMBERLB(aIndex);
	memcpy(pMsg.MemberID, MemberId, sizeof(pMsg.MemberID));
	memcpy(pMsg.GuildName, Name, sizeof(pMsg.GuildName));

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GDGuildMemberDelResult(SDHP_GUILDMEMBERDEL_RESULT * lpMsg)
{
	int aIndex = -1;
	char szMember[11];
	char szGuildName[9];

	memcpy(szMember, lpMsg->MemberID, sizeof(szMember));
	memcpy(szGuildName, lpMsg->GuildName, sizeof(szGuildName));

	if ( lpMsg->Flag == 1 )
	{
		aIndex = MAKE_NUMBERW(lpMsg->NumberH, lpMsg->NumberL);

		if ( gObjIsConnected(aIndex) == TRUE )
		{
			if ( strcmp(szMember, gObj[aIndex].Name) == 0 )
			{
				GCResultSend(aIndex, 0x53, lpMsg->Result);
			}
			else
			{
				GCResultSend(aIndex, 0x53, 5);
			}
		}
	}

	if ( lpMsg->Result == 1 )
	{
		int iMember = 0;

		while ( true )
		{
			if ( gObj[iMember].Connected > OBJECT_LOGGED)
			{
				if ( gObj[iMember].GuildNumber > 0 )
				{
					if ( gObj[iMember].Name[0] == szMember[0] && gObj[iMember].Name[1] == szMember[1] )
					{
						if ( strcmp(gObj[iMember].Name, szMember) == 0 )
						{
							GCGuildViewportDelNow(iMember, FALSE);
							gObj[iMember].Guild = NULL;
							gObj[iMember].GuildNumber = 0;
							gObj[iMember].GuildStatus = -1;
							gObj[iMember].GuildUnionTimeStamp = 0;
							memset(gObj[iMember].GuildName,0,sizeof(gObj[iMember].GuildName));
							gObjNotifyUpdateUnionV1(&gObj[iMember]);
							break;
						}
					}
				}
			}

			if ( iMember < MAX_OBJECT-1 )
			{
				iMember++;
			}
			else
			{
				break;
			}
		}

		gGuildClass.DelMember(szGuildName, szMember);
	}
}

void DGGuildMemberInfoRequest(int aIndex)
{
	SDHP_GUILDMEMBER_INFO_REQUEST pMsg;

	pMsg.h.set(0x35, sizeof(pMsg));
	pMsg.NumberH = SET_NUMBERHB(aIndex);
	pMsg.NumberL = SET_NUMBERLB(aIndex);
	memcpy(pMsg.MemberID, gObj[aIndex].Name, sizeof(pMsg.MemberID));

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void DGGuildMemberInfo(SDHP_GUILDMEMBER_INFO * lpMsg)
{
	BYTE GuildUserBuf[256] = {0};
	BYTE GuildInfoBuf[256] = {0};
	PMSG_SIMPLE_GUILDVIEWPORT pMsg;
	char szGuildName[9];
	char szName[11];

	memcpy(szGuildName, lpMsg->GuildName, sizeof(szGuildName));
	memcpy(szName, lpMsg->MemberID, sizeof(szName));

	int GuildInfoOfs = sizeof(PMSG_SIMPLE_GUILDVIEWPORT_COUNT);
	int GuildUserOfs = sizeof(PMSG_SIMPLE_GUILDVIEWPORT_COUNT);

	for ( int n=OBJECT_START_USER;n<MAX_OBJECT;n++)
	{
		if ( gObj[n].Connected > OBJECT_LOGGED )
		{
			if ( gObj[n].Name[0] == lpMsg->MemberID[0] )
			{
				if ( strcmp(lpMsg->MemberID, gObj[n].Name ) == 0 )
				{
					#if(GAMESERVER_TYPE==1)
					szGuildName[8] = NULL;
					gCastleSiege.GetCsJoinSide(szGuildName,&gObj[n].CsJoinSide,&gObj[n].CsGuildInvolved);
					gCastleSiege.NotifySelfCsJoinSide(n);
					#endif
					memcpy(gObj[n].GuildName,szGuildName,sizeof(gObj[n].GuildName));
					gObj[n].Guild = gGuildClass.SearchGuild(gObj[n].GuildName);

					if ( gObj[n].Guild != NULL )
					{
						gObj[n].GuildStatus = lpMsg->btGuildStatus;
						gObj[n].GuildNumber = gObj[n].Guild->Number;
						gGuildClass.ConnectUser(gObj[n].Guild, lpMsg->GuildName, gObj[n].Name, n,  lpMsg->pServer);
	
						if ( strlen(gObj[n].Guild->Notice) > 0 )
						{
							gNotice.GCNoticeSend(n,2,0,0,0,0,0,"%s",gObj[n].Guild->Notice);
						}

						pMsg.GuildNumber = gObj[n].GuildNumber;
						pMsg.NumberH = SET_NUMBERHB(n) & 0x7F;
						pMsg.NumberL = SET_NUMBERLB(n);

						if ( strcmp(gObj[n].Guild->Names[0], gObj[n].Name) == 0 )
						{
							pMsg.NumberH |= 0x80;
						}

						pMsg.btGuildStatus = gObj[n].GuildStatus;

						if ( gObj[n].Guild != NULL )
						{
							pMsg.btGuildType = gObj[n].Guild->GuildType;
						}
						else
						{
							pMsg.btGuildType = G_TYPE_NONE;
						}

						//Season 4.5 Addon start
                        if(gCastleSiegeSync.CheckCastleOwnerMember(gObj[n].Index) == 1 ||
                           gCastleSiegeSync.CheckCastleOwnerUnionMember(gObj[n].Index) == 1)
                        {
                            pMsg.btOwnerStatus = 1;
                        }
                        else
                        {
                            pMsg.btOwnerStatus = 0;
                        }
                        //Season 4.5 Addon end

						memcpy(&GuildInfoBuf[GuildInfoOfs], &pMsg, sizeof(PMSG_SIMPLE_GUILDVIEWPORT));
						GuildInfoOfs += sizeof(PMSG_SIMPLE_GUILDVIEWPORT);

						PMSG_SIMPLE_GUILDVIEWPORT_COUNT pGVCount;
						
						pGVCount.h.type = 0xC2;
						pGVCount.h.head = 0x65;
						pGVCount.h.size[0] = SET_NUMBERHB(GuildInfoOfs);
						pGVCount.h.size[1] = SET_NUMBERLB(GuildInfoOfs);
						pGVCount.Count = 0x01;
						memcpy(GuildInfoBuf, &pGVCount, sizeof(PMSG_SIMPLE_GUILDVIEWPORT_COUNT));

						DataSend(n, (LPBYTE)GuildInfoBuf, GuildInfoOfs);

						if ( gObj[n].Guild->WarState != 0 )
						{
							gGuild.GCGuildWarScoreSend(n);
							GCGuildWarDeclare(n, gObj[n].Guild->TargetGuildName);
						}
					}
					else
					{
						#if(GAMESERVER_TYPE==1)
						gObj[n].CsJoinSide = 0;
						gObj[n].CsGuildInvolved = 0;
						#endif
					}

					return;
				}
			}
		}
	}

	gGuildClass.SetServer(szGuildName, szName, lpMsg->pServer);
}

void DGGuildMasterListRecv(LPBYTE lpData)
{
	SDHP_GUILDALL_COUNT * lpCount;
	SDHP_GUILDALL * lpList;
	int lOfs = sizeof(SDHP_GUILDALL_COUNT);
	char szGuildName[9];
	char szMasterName[11];

	memset(szMasterName, 0, sizeof(szMasterName));
	memset(szGuildName, 0, sizeof(szGuildName));
	lpCount = (SDHP_GUILDALL_COUNT *)lpData;
	memcpy(szGuildName, lpCount->GuildName, sizeof(lpCount->GuildName));
	memcpy(szMasterName, lpCount->Master, sizeof(lpCount->Master));

	if ( strlen(szGuildName) < 1 )
	{
		return;
	}

	if ( lpCount->Count < 1 )
	{
		return;
	}

	gGuildClass.AddGuild(lpCount->Number, szGuildName, lpCount->Mark, szMasterName, lpCount->score);
	gGuildClass.SetGuildType(szGuildName, lpCount->btGuildType);

	GUILD_INFO_STRUCT * lpGuild = gGuildClass.SearchGuild_Number(lpCount->Number);

	if ( lpGuild != NULL )
	{
		lpGuild->GuildUnion = lpCount->iGuildUnion;
		lpGuild->GuildRival = lpCount->iGuildRival;
		memcpy(lpGuild->GuildRivalName, lpCount->szGuildRivalName, sizeof(lpGuild->GuildRivalName));
	}

	for ( int n=0;n<lpCount->Count;n++)
	{
		lpList = (SDHP_GUILDALL *)&lpData[lOfs];

		memcpy(szMasterName, lpList->MemberID, sizeof(szMasterName));
		
		if ( gGuildClass.AddMember(szGuildName, szMasterName, -1, lpCount->Count,lpList->btGuildStatus, lpList->pServer) == FALSE )
		{
			break;
		}

		lOfs += sizeof(SDHP_GUILDALL);
	}
}

void DGGuildScoreUpdate(LPSTR guildname, int score)
{
	SDHP_GUILDSCOREUPDATE pMsg;

	pMsg.h.set(0x37, sizeof(SDHP_GUILDSCOREUPDATE));
	pMsg.Score = score;
	memcpy(pMsg.GuildName,guildname,sizeof(pMsg.GuildName));

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GDGuildScoreUpdateRecv(SDHP_GUILDSCOREUPDATE * lpMsg)
{
	GUILD_INFO_STRUCT * lpNode = gGuildClass.SearchGuild(lpMsg->GuildName);

	if ( lpNode == NULL )
	{
		return;
	}

	lpNode->TotalScore = lpMsg->Score;
}

void GDGuildNoticeSave(LPSTR guild_name, LPSTR guild_notice)
{
	SDHP_GUILDNOTICE pMsg;

	int len = strlen((char*)guild_notice);

	if ( len < 1 )
	{
		return;
	}

	if ( len > 58 )
	{
		return;
	}

	pMsg.h.type = 0xC1;
	pMsg.h.head = 0x38;
	pMsg.h.size = sizeof(SDHP_GUILDNOTICE);
	memcpy(pMsg.GuildName, guild_name, sizeof(pMsg.GuildName));
	memcpy(pMsg.szGuildNotice, guild_notice, sizeof(pMsg.szGuildNotice));

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void DGGuildNoticeRecv(SDHP_GUILDNOTICE * lpMsg)
{
	GUILD_INFO_STRUCT * lpNode = gGuildClass.SearchGuild(lpMsg->GuildName);

	if ( lpNode == NULL )
	{
		return;
	}

	memcpy(lpNode->Notice, lpMsg->szGuildNotice, sizeof(lpNode->Notice));

	for ( int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( lpNode->Use[n] > 0 && lpNode->Index[n] >= 0 )
		{
			gNotice.GCNoticeSend(lpNode->Index[n],2,0,0,0,0,0,"%s",lpMsg->szGuildNotice);
		}
	}
}

void GDGuildServerGroupChattingSend(int iGuildNum, char* szCharacterName, char* szChattingMsg)
{
	EXSDHP_SERVERGROUP_GUILD_CHATTING_SEND pMsg = {0};

	pMsg.h.set(0x50, sizeof(EXSDHP_SERVERGROUP_GUILD_CHATTING_SEND));
	pMsg.iGuildNum = iGuildNum;
	memcpy(pMsg.szCharacterName, szCharacterName, sizeof(pMsg.szCharacterName));
	memcpy(pMsg.szChattingMsg, szChattingMsg, sizeof(pMsg.szChattingMsg));

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void DGGuildServerGroupChattingRecv( EXSDHP_SERVERGROUP_GUILD_CHATTING_RECV* lpMsg)
{
	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00, sizeof(pMsg));
	memcpy(pMsg.name, lpMsg->szCharacterName, sizeof(pMsg.name));
	memcpy(pMsg.message, lpMsg->szChattingMsg, sizeof(pMsg.message));
	GUILD_INFO_STRUCT * lpGuildInfo = gGuildClass.SearchGuild_Number(lpMsg->iGuildNum);

	if ( lpGuildInfo == NULL )
		return;

	int iUserIndex = -1;

	if ( lpGuildInfo->Count >= 0 )
	{
		for ( int i=0;i<MAX_GUILD_USER;i++)
		{
			if ( lpGuildInfo->Use[i] != FALSE )
			{
				iUserIndex = lpGuildInfo->Index[i];

				if ( iUserIndex >= 0 )
				{
					if ( lpGuildInfo->Names[i][0] == gObj[iUserIndex].Name[0] )
					{
						if ( !strcmp(lpGuildInfo->Names[i], gObj[iUserIndex].Name ))
						{
							DataSend(iUserIndex, (LPBYTE)&pMsg, pMsg.header.size);
						}
					}
				}
			}
		}
	}
}

void GDUnionServerGroupChattingSend(int iUnionNum, char* szCharacterName, char* szChattingMsg)
{
	EXSDHP_SERVERGROUP_UNION_CHATTING_SEND pMsg = {0};

	pMsg.h.set(0x51, sizeof(pMsg));
	pMsg.iUnionNum = iUnionNum;
	memcpy(pMsg.szCharacterName, szCharacterName, sizeof(pMsg.szCharacterName));
	memcpy(pMsg.szChattingMsg, szChattingMsg, sizeof(pMsg.szChattingMsg));

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void DGUnionServerGroupChattingRecv( EXSDHP_SERVERGROUP_UNION_CHATTING_RECV* lpMsg)
{
	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00, sizeof(pMsg));
	memcpy(pMsg.name, lpMsg->szCharacterName, sizeof(pMsg.name));
	memcpy(pMsg.message, lpMsg->szChattingMsg, sizeof(pMsg.message));
	int iUnionNum = lpMsg->iUnionNum;
	int iGuildCount =0;
	int iGuildList[100] ={0};

	if ( gUnionManager.GetGuildUnionMemberList(iUnionNum, iGuildCount, iGuildList) == TRUE)
	{
		int iUserIndex = -1;

		for ( int i=0;i<iGuildCount;i++)
		{
			GUILD_INFO_STRUCT * lpGuildInfo = gGuildClass.SearchGuild_Number(iGuildList[i]);

			if ( lpGuildInfo == NULL )
				continue;

			for ( int n=0;n<MAX_GUILD_USER;n++)
			{
				if ( lpGuildInfo->Use[n] != FALSE )
				{
					iUserIndex = lpGuildInfo->Index[n];

					if ( iUserIndex >= 0 )
					{
						if ( lpGuildInfo->Names[n][0] == gObj[iUserIndex].Name[0] )
						{
							if ( !strcmp(lpGuildInfo->Names[n], gObj[iUserIndex].Name ))
							{
								DataSend(iUserIndex, (LPBYTE)&pMsg, pMsg.header.size);
							}
						}
					}
				}
			}
		}
	}
}

void GDGuildReqAssignStatus(int aIndex, int iAssignType, LPSTR szTagetName, int iGuildStatus)
{
	EXSDHP_GUILD_ASSIGN_STATUS_REQ pMsg = {0};

	pMsg.h.set(0xE1, sizeof(EXSDHP_GUILD_ASSIGN_STATUS_REQ));
	pMsg.btType = iAssignType;
	pMsg.btGuildStatus = iGuildStatus;
	pMsg.wUserIndex = aIndex;
	memcpy(pMsg.szGuildName, gObj[aIndex].Guild->Name, sizeof(pMsg.szGuildName));
	memcpy(pMsg.szTargetName, szTagetName, sizeof(pMsg.szTargetName));

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void DGGuildAnsAssignStatus(EXSDHP_GUILD_ASSIGN_STATUS_RESULT * lpMsg)
{
	if ( lpMsg->btFlag == 1 )
	{
		GUILD_INFO_STRUCT * lpNode = gGuildClass.SearchGuild(lpMsg->szGuildName);

		if ( lpNode == NULL )
		{
			lpMsg->btResult = 0;
		}

		PMSG_GUILD_ASSIGN_STATUS_RESULT pMsg = {0};

		pMsg.h.set(0xE1, sizeof(PMSG_GUILD_ASSIGN_STATUS_RESULT));
		pMsg.btType = lpMsg->btType;
		pMsg.btResult = lpMsg->btResult;
		memcpy(pMsg.szTagetName, lpMsg->szTargetName, sizeof(pMsg.szTagetName));

		DataSend(lpMsg->wUserIndex, (LPBYTE)&pMsg, sizeof(pMsg));

		if ( lpMsg->btResult == 0 )
		{
			return;
		}

		gGuildClass.SetGuildMemberStatus(lpMsg->szGuildName, lpMsg->szTargetName, lpMsg->btGuildStatus);

		for ( int i=0;i<MAX_GUILD_USER;i++)
		{
			if ( lpNode->Use[i] > 0 && lpNode->Index[i] >= 0)
			{
				if ( lpMsg->btGuildStatus == GUILD_ASSISTANT )
				{
					gNotice.GCNoticeSend(lpNode->Index[i],0,0,0,0,0,0,gMessage.GetMessage(432),lpMsg->szTargetName);
				}
				else if ( lpMsg->btGuildStatus == GUILD_BATTLE_MASTER )
				{
					gNotice.GCNoticeSend(lpNode->Index[i],0,0,0,0,0,0,gMessage.GetMessage(433),lpMsg->szTargetName);
				}
				else
				{
					gNotice.GCNoticeSend(lpNode->Index[i],0,0,0,0,0,0,gMessage.GetMessage(434),lpMsg->szTargetName);
				}
			}
		}
	}
	else if ( lpMsg->btFlag == 0 )
	{
		GUILD_INFO_STRUCT * lpNode = gGuildClass.SearchGuild(lpMsg->szGuildName);

		if ( lpNode == NULL )
		{
			return;
		}
		
		gGuildClass.SetGuildMemberStatus(lpMsg->szGuildName, lpMsg->szTargetName, lpMsg->btGuildStatus);
		
		if ( lpMsg->btResult == 0 )
		{
			return;
		}

		for ( int n=0;n<MAX_GUILD_USER;n++)
		{
			if ( lpNode->Use[n] > 0 && lpNode->Index[n] >= 0)
			{
				if ( lpMsg->btGuildStatus == GUILD_ASSISTANT )
				{
					gNotice.GCNoticeSend(lpNode->Index[n],0,0,0,0,0,0,gMessage.GetMessage(432),lpMsg->szTargetName);
				}
				else if ( lpMsg->btGuildStatus == GUILD_BATTLE_MASTER )
				{
					gNotice.GCNoticeSend(lpNode->Index[n],0,0,0,0,0,0,gMessage.GetMessage(433),lpMsg->szTargetName);
				}
				else
				{
					gNotice.GCNoticeSend(lpNode->Index[n],0,0,0,0,0,0,gMessage.GetMessage(434),lpMsg->szTargetName);
				}
			}
		}
	}
}

void GDGuildReqAssignType(int aIndex, int iGuildType)
{
	EXSDHP_GUILD_ASSIGN_TYPE_REQ pMsg = {0};

	pMsg.h.set(0xE2, sizeof(EXSDHP_GUILD_ASSIGN_TYPE_REQ));
	pMsg.btGuildType = iGuildType;
	pMsg.wUserIndex = aIndex;
	memcpy(pMsg.szGuildName, gObj[aIndex].Guild->Name, sizeof(pMsg.szGuildName));

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void DGGuildAnsAssignType(EXSDHP_GUILD_ASSIGN_TYPE_RESULT * lpMsg)
{
	if ( lpMsg->btFlag == G_TYPE_GUARD )
	{
		GUILD_INFO_STRUCT * lpNode = gGuildClass.SearchGuild(lpMsg->szGuildName);

		if ( lpNode == NULL )
		{
			lpMsg->btResult = 0;
		}

		PMSG_GUILD_ASSIGN_TYPE_RESULT pMsg = {0};

		pMsg.h.set(0xE2, sizeof(PMSG_GUILD_ASSIGN_TYPE_RESULT));
		pMsg.btGuildType = lpMsg->btGuildType;
		pMsg.btResult = lpMsg->btResult;

		DataSend(lpMsg->wUserIndex, (LPBYTE)&pMsg,sizeof(pMsg));

		if ( lpMsg->btResult == G_ERROR_NONE )
		{
			return;
		}

		gGuildClass.SetGuildType(lpMsg->szGuildName, lpMsg->btGuildType);

		for ( int i=0;i<MAX_GUILD_USER;i++)
		{
			if ( lpNode->Use[i] > 0 && lpNode->Index[i] >= 0)
			{
				gNotice.GCNoticeSend(lpNode->Index[i],0,0,0,0,0,0,gMessage.GetMessage(435),lpMsg->szGuildName,lpMsg->btGuildType);
			}
		}
	}
	else if ( lpMsg->btFlag == G_TYPE_COMMON )
	{
		GUILD_INFO_STRUCT * lpNode = gGuildClass.SearchGuild(lpMsg->szGuildName);

		if ( lpNode == NULL )
		{
			return;
		}

		gGuildClass.SetGuildType(lpMsg->szGuildName, lpMsg->btGuildType);

		if ( lpMsg->btResult == G_ERROR_NONE )
		{
			return;
		}		
		
		for ( int n=0;n<MAX_GUILD_USER;n++)
		{
			if ( lpNode->Use[n] > 0 && lpNode->Index[n] >= 0)
			{
				gNotice.GCNoticeSend(lpNode->Index[n],0,0,0,0,0,0,gMessage.GetMessage(435),lpMsg->szGuildName,lpMsg->btGuildType);
			}
		}
	}
}

void GDRelationShipReqJoin(int aIndex, int iTargetUserIndex, int iRelationShipType)
{
	GUILD_INFO_STRUCT * lpGuildInfo = gObj[aIndex].Guild;
	GUILD_INFO_STRUCT * lpTargetGuildInfo = gObj[iTargetUserIndex].Guild;

	if ( !lpGuildInfo || !lpTargetGuildInfo )
	{
		return;
	}

	EXSDHP_RELATIONSHIP_JOIN_REQ pMsg = {0};

	pMsg.h.set(0xE5, sizeof(EXSDHP_RELATIONSHIP_JOIN_REQ));
	pMsg.wRequestUserIndex = aIndex;
	pMsg.wTargetUserIndex = iTargetUserIndex;
	pMsg.btRelationShipType = iRelationShipType;
	pMsg.iRequestGuildNum = gObj[aIndex].Guild->Number;
	pMsg.iTargetGuildNum = gObj[iTargetUserIndex].Guild->Number;

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void DGRelationShipAnsJoin(EXSDHP_RELATIONSHIP_JOIN_RESULT * lpMsg)
{
	GUILD_INFO_STRUCT * lpReqGuild = gGuildClass.SearchGuild_Number(lpMsg->iRequestGuildNum);
	GUILD_INFO_STRUCT * lpTargetGuild = gGuildClass.SearchGuild_Number(lpMsg->iTargetGuildNum);

	if ( lpMsg->btFlag == 1 )
	{
		if ( !lpReqGuild || !lpTargetGuild )
		{
			lpMsg->btResult = 0;
		}

		PMSG_RELATIONSHIP_JOIN_BREAKOFF_ANS pMsg ={0};

		pMsg.h.set(0xE6, sizeof(pMsg));
		pMsg.btResult = lpMsg->btResult;
		pMsg.btRequestType = G_RELATION_OPERATION_JOIN;
		pMsg.btRelationShipType = lpMsg->btRelationShipType;
		pMsg.btTargetUserIndexH = SET_NUMBERHB(lpMsg->wTargetUserIndex);
		pMsg.btTargetUserIndexL = SET_NUMBERLB(lpMsg->wTargetUserIndex);

		DataSend(lpMsg->wRequestUserIndex, (LPBYTE)&pMsg, sizeof(pMsg));

		pMsg.btTargetUserIndexH = SET_NUMBERHB(lpMsg->wRequestUserIndex);
		pMsg.btTargetUserIndexL = SET_NUMBERLB(lpMsg->wRequestUserIndex);

		DataSend(lpMsg->wTargetUserIndex, (LPBYTE)&pMsg, sizeof(pMsg));

		if ( lpMsg->btResult == 0 ||lpMsg->btResult == 16 )
			return;

	}
	else if ( lpMsg->btFlag == 0 )
	{
		if ( !lpReqGuild || !lpTargetGuild )
			return;

		if ( lpMsg->btResult == 0 ||lpMsg->btResult == 16 )
			return;

	}

	if (lpMsg->btRelationShipType == G_RELATIONSHIP_UNION )
	{
		lpReqGuild->SetGuildUnion(lpTargetGuild->Number);
		lpTargetGuild->SetGuildUnion(lpTargetGuild->Number);
	}
	else if ( lpMsg->btRelationShipType == G_RELATIONSHIP_RIVAL )
	{
		lpReqGuild->SetGuildRival(lpTargetGuild->Number);
		lpTargetGuild->SetGuildRival(lpReqGuild->Number);
		memcpy(lpReqGuild->GuildRivalName, lpMsg->szTargetGuildName, sizeof(lpReqGuild->GuildRivalName));
		memcpy(lpTargetGuild->GuildRivalName, lpMsg->szRequestGuildName, sizeof(lpTargetGuild->GuildRivalName));
	}

	for ( int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( lpReqGuild->Use[n] > 0 )
		{
			if ( lpReqGuild->Index[n] >= 0 )
			{
				if ( lpMsg->btRelationShipType == G_RELATIONSHIP_UNION )
				{
					gNotice.GCNoticeSend(lpReqGuild->Index[n],1,0,0,0,0,0,gMessage.GetMessage(436),lpTargetGuild->Name);
				}
				else if ( lpMsg->btRelationShipType == G_RELATIONSHIP_RIVAL )
				{
					gNotice.GCNoticeSend(lpReqGuild->Index[n],1,0,0,0,0,0,gMessage.GetMessage(437),lpTargetGuild->Name);
				}
			}
		}
	}

	for (int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( lpTargetGuild->Use[n] > 0 )
		{
			if ( lpTargetGuild->Index[n] >= 0 )
			{
				if ( lpMsg->btRelationShipType == G_RELATIONSHIP_UNION )
				{
					gNotice.GCNoticeSend(lpTargetGuild->Index[n],1,0,0,0,0,0,gMessage.GetMessage(438),lpReqGuild->Name,lpTargetGuild->Name);
				}
				else if ( lpMsg->btRelationShipType == G_RELATIONSHIP_RIVAL )
				{
					gNotice.GCNoticeSend(lpTargetGuild->Index[n],1,0,0,0,0,0,gMessage.GetMessage(437),lpReqGuild->Name);
				}
			}
		}
	}
}

void GDUnionBreakOff(int aIndex, int iUnionNumber)
{
	GUILD_INFO_STRUCT * lpGuildInfo = gObj[aIndex].Guild;

	if ( !lpGuildInfo )
	{
		return;
	}

	EXSDHP_RELATIONSHIP_BREAKOFF_REQ pMsg = {0};

	pMsg.h.set(0xE6, sizeof(EXSDHP_RELATIONSHIP_BREAKOFF_REQ));
	pMsg.wRequestUserIndex = aIndex;
	pMsg.wTargetUserIndex = -1;
	pMsg.btRelationShipType = G_RELATIONSHIP_UNION;
	pMsg.iRequestGuildNum = lpGuildInfo->Number;
	pMsg.iTargetGuildNum = iUnionNumber;

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);

}

void DGUnionBreakOff(EXSDHP_RELATIONSHIP_BREAKOFF_RESULT * lpMsg)
{
	GUILD_INFO_STRUCT * lpReqGuild = gGuildClass.SearchGuild_Number(lpMsg->iRequestGuildNum);
	GUILD_INFO_STRUCT * lpTargetGuild = gGuildClass.SearchGuild_Number(lpMsg->iTargetGuildNum);

	if ( lpMsg->btFlag == 1 )
	{
		if ( !lpReqGuild )
		{
			lpMsg->btResult = 0;
		}

		PMSG_RELATIONSHIP_JOIN_BREAKOFF_ANS pMsg ={0};

		pMsg.h.set(0xE6, sizeof(pMsg));
		pMsg.btResult = lpMsg->btResult;
		pMsg.btRequestType = G_RELATION_OPERATION_BREAKOFF;
		pMsg.btRelationShipType = G_RELATIONSHIP_UNION;
		pMsg.btTargetUserIndexH = SET_NUMBERHB(lpMsg->wTargetUserIndex);
		pMsg.btTargetUserIndexL = SET_NUMBERLB(lpMsg->wTargetUserIndex);

		DataSend(lpMsg->wRequestUserIndex, (LPBYTE)&pMsg, sizeof(pMsg));

		if ( lpMsg->btResult == 0 ||lpMsg->btResult == 16 )
		{
			return;
		}

	}
	else if ( lpMsg->btFlag == 0 )
	{
		if ( !lpReqGuild )
		{
			return;
		}

		if ( lpMsg->btResult == 0 ||lpMsg->btResult == 16 )
		{
			return;
		}

	}

	lpReqGuild->SetGuildUnion(0);

	for ( int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( lpReqGuild->Use[n] > 0 )
		{
			if ( lpReqGuild->Index[n] >= 0 )
			{
				gNotice.GCNoticeSend(lpReqGuild->Index[n],1,0,0,0,0,0,gMessage.GetMessage(439));
			}
		}
	}

	if ( !lpTargetGuild)
	{
		return;
	}

	for (int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( lpTargetGuild->Use[n] > 0 )
		{
			if ( lpTargetGuild->Index[n] >= 0 )
			{
				gNotice.GCNoticeSend(lpTargetGuild->Index[n],1,0,0,0,0,0,gMessage.GetMessage(440),lpReqGuild->Name,lpTargetGuild->Name);
			}
		}
	}
}

void GDRelationShipReqBreakOff(int aIndex, int iTargetUserIndex, int iRelationShipType)
{
	GUILD_INFO_STRUCT * lpGuildInfo = gObj[aIndex].Guild;
	GUILD_INFO_STRUCT * lpTargetGuildInfo = gObj[iTargetUserIndex].Guild;

	if ( !lpGuildInfo || !lpTargetGuildInfo )
		return;

	EXSDHP_RELATIONSHIP_BREAKOFF_REQ pMsg = {0};

	pMsg.h.set(0xE6, sizeof(EXSDHP_RELATIONSHIP_BREAKOFF_REQ));
	pMsg.wRequestUserIndex = aIndex;
	pMsg.wTargetUserIndex = iTargetUserIndex;
	pMsg.btRelationShipType = iRelationShipType;
	pMsg.iRequestGuildNum = lpGuildInfo->Number;
	pMsg.iTargetGuildNum = lpTargetGuildInfo->Number;

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void DGRelationShipAnsBreakOff(EXSDHP_RELATIONSHIP_BREAKOFF_RESULT * lpMsg)
{
	if ( lpMsg->btRelationShipType == G_RELATIONSHIP_UNION )
	{
		DGUnionBreakOff(lpMsg);
		return;
	}

	GUILD_INFO_STRUCT * lpReqGuild = gGuildClass.SearchGuild_Number(lpMsg->iRequestGuildNum);
	GUILD_INFO_STRUCT * lpTargetGuild = gGuildClass.SearchGuild_Number(lpMsg->iTargetGuildNum);

	if ( lpMsg->btFlag == 1 )
	{
		if ( !lpReqGuild || !lpTargetGuild )
		{
			lpMsg->btResult = 0;
		}

		PMSG_RELATIONSHIP_JOIN_BREAKOFF_ANS pMsg ={0};

		pMsg.h.set(0xE6, sizeof(pMsg));
		pMsg.btResult = lpMsg->btResult;
		pMsg.btRequestType = 2;
		pMsg.btRelationShipType = lpMsg->btRelationShipType;
		pMsg.btTargetUserIndexH = SET_NUMBERHB(lpMsg->wTargetUserIndex);
		pMsg.btTargetUserIndexL = SET_NUMBERLB(lpMsg->wTargetUserIndex);

		if ( lpMsg->wRequestUserIndex != -1 )
			DataSend(lpMsg->wRequestUserIndex, (LPBYTE)&pMsg, sizeof(pMsg));

		pMsg.btTargetUserIndexH = SET_NUMBERHB(lpMsg->wRequestUserIndex);
		pMsg.btTargetUserIndexL = SET_NUMBERLB(lpMsg->wRequestUserIndex);

		if ( lpMsg->wTargetUserIndex != -1 )
			DataSend(lpMsg->wTargetUserIndex, (LPBYTE)&pMsg, sizeof(pMsg));

		if ( lpMsg->btResult == 0 ||lpMsg->btResult == 16 )
			return;

	}
	else if ( lpMsg->btFlag == 0 )
	{
		if ( !lpReqGuild || !lpTargetGuild )
			return;

		if ( lpMsg->btResult == 0 ||lpMsg->btResult == 16 )
			return;

	}

	if ( lpMsg->btRelationShipType == G_RELATIONSHIP_RIVAL )
	{
		lpReqGuild->SetGuildRival(0);
		lpTargetGuild->SetGuildRival(0);
		lpReqGuild->GuildRivalName[0]=0;
		lpTargetGuild->GuildRivalName[0]=0;
	}

	for ( int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( lpReqGuild->Use[n] > 0 )
		{
			if ( lpReqGuild->Index[n] >= 0 )
			{
				if ( lpMsg->btRelationShipType == G_RELATIONSHIP_UNION )
				{
					gNotice.GCNoticeSend(lpReqGuild->Index[n],1,0,0,0,0,0,gMessage.GetMessage(441),lpTargetGuild->Name);
				}
				else if ( lpMsg->btRelationShipType == G_RELATIONSHIP_RIVAL )
				{
					gNotice.GCNoticeSend(lpReqGuild->Index[n],1,0,0,0,0,0,gMessage.GetMessage(442),lpTargetGuild->Name);
				}
			}
		}
	}
}

void DGRelationShipNotificationRecv(EXSDHP_NOTIFICATION_RELATIONSHIP * lpMsg)
{
	if ( lpMsg->btUpdateFlag == G_NOTIFICATION_UNION_BREAKOFF )
	{
		GUILD_INFO_STRUCT * lpGuildInfo = gGuildClass.SearchGuild_Number(lpMsg->iGuildList[0]);

		if ( lpGuildInfo )
		{
			lpGuildInfo->GuildUnion = 0;
			lpGuildInfo->SetTimeStamp();

			for ( int n=0;n<MAX_GUILD_USER;n++)
			{
				if ( lpGuildInfo->Use[n] > 0 )
				{
					if ( lpGuildInfo->Index[n] >= 0 )
					{
						gNotice.GCNoticeSend(lpGuildInfo->Index[n],G_NOTIFICATION_SET,0,0,0,0,0,gMessage.GetMessage(443),lpGuildInfo->Name);
					}
				}
			}
		}
	}

	for ( int i=0;i<lpMsg->btGuildListCount;i++)
	{
		GUILD_INFO_STRUCT * lpGuildInfo = gGuildClass.SearchGuild_Number(lpMsg->iGuildList[i]);

		if ( lpGuildInfo )
		{
			lpGuildInfo->SetTimeStamp();
		}
	}
}

void DGRelationShipListRecv(EXSDHP_UNION_RELATIONSHIP_LIST * lpMsg)
{
	if ( lpMsg->btRelationShipType == G_RELATIONSHIP_UNION )
	{
		gUnionManager.AddUnion(lpMsg->iUnionMasterGuildNumber, lpMsg->szUnionMasterGuildName);
		gUnionManager.SetGuildUnionMemberList(lpMsg->iUnionMasterGuildNumber, lpMsg->btRelationShipMemberCount, lpMsg->iRelationShipMember);
	}
	else if ( lpMsg->btRelationShipType == G_RELATIONSHIP_RIVAL )
	{
		gUnionManager.AddUnion(lpMsg->iUnionMasterGuildNumber, lpMsg->szUnionMasterGuildName);
		gUnionManager.SetGuildRivalMemberList(lpMsg->iUnionMasterGuildNumber, lpMsg->btRelationShipMemberCount, lpMsg->iRelationShipMember);
	}
}

void GDUnionListSend(int iUserIndex, int iUnionMasterGuildNumber)
{
	EXSDHP_UNION_LIST_REQ pMsg = {0};

	pMsg.h.set(0xE9, sizeof(EXSDHP_UNION_LIST_REQ));
	pMsg.wRequestUserIndex = iUserIndex;
	pMsg.iUnionMasterGuildNumber = iUnionMasterGuildNumber;

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void DGUnionListRecv(LPBYTE aRecv)
{
	EXSDHP_UNION_LIST_COUNT * lpRecvMsg = (EXSDHP_UNION_LIST_COUNT *)aRecv;
	EXSDHP_UNION_LIST * lpRecvMsgBody = (EXSDHP_UNION_LIST *)((DWORD)aRecv + 0x10 ) ;
	char cBUFFER_V1[6000];
	memset(cBUFFER_V1, 0, sizeof(cBUFFER_V1));
	int iSize = MAKE_NUMBERW(lpRecvMsg->h.size[0], lpRecvMsg->h.size[1]);
	int iBodySize = iSize - 0x10;

	if ( iBodySize < 0 )
		return;

	if ( iBodySize > 6000 )
		return;

	if ( lpRecvMsg->btCount < 0 || lpRecvMsg->btCount > 100 )
		return;

	PMSG_UNIONLIST_COUNT * lpMsg = (PMSG_UNIONLIST_COUNT *)cBUFFER_V1;
	PMSG_UNIONLIST * lpMsgBody = (PMSG_UNIONLIST *)((DWORD)cBUFFER_V1+sizeof(PMSG_UNIONLIST_COUNT));
	lpMsg->btResult = lpRecvMsg->btResult;
	lpMsg->btCount = lpRecvMsg->btCount;
	lpMsg->btRivalMemberNum = lpRecvMsg->btRivalMemberNum;
	lpMsg->btUnionMemberNum = lpRecvMsg->btUnionMemberNum;

	for (int i=0;i<lpMsg->btCount;i++)
	{
		lpMsgBody[i].btMemberNum = lpRecvMsgBody[i].btMemberNum;
		memcpy(lpMsgBody[i].szGuildName, lpRecvMsgBody[i].szGuildName, sizeof(lpMsgBody[i].szGuildName));
		memcpy(lpMsgBody[i].Mark, lpRecvMsgBody[i].Mark, sizeof(lpMsgBody[i].Mark));
	}

	lpMsg->h.set(0xE9, lpMsg->btCount * sizeof(PMSG_UNIONLIST) + 0x10);
	DataSend(lpRecvMsg->wRequestUserIndex, (LPBYTE)lpMsg, MAKE_NUMBERW(lpMsg->h.size[0], lpMsg->h.size[1]));
}

void GDRelationShipReqKickOutUnionMember(int aIndex, LPSTR szTargetGuildName)
{
	GUILD_INFO_STRUCT * lpGuildInfo = gObj[aIndex].Guild;

	if ( lpGuildInfo == NULL )
	{
		return;
	}

	EXSDHP_KICKOUT_UNIONMEMBER_REQ pMsg = {0};
	pMsg.h.set(0xEB, 0x01, sizeof(EXSDHP_KICKOUT_UNIONMEMBER_REQ));
	pMsg.wRequestUserIndex = aIndex;
	pMsg.btRelationShipType = GUILD_RELATION_UNION;
	memcpy(pMsg.szUnionMasterGuildName, lpGuildInfo->Name, sizeof(pMsg.szUnionMasterGuildName));
	memcpy(pMsg.szUnionMemberGuildName, szTargetGuildName, sizeof(pMsg.szUnionMemberGuildName));

	ESDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void DGRelationShipAnsKickOutUnionMember(EXSDHP_KICKOUT_UNIONMEMBER_RESULT * lpMsg)
{
	GUILD_INFO_STRUCT * lpUnionMasterGuild = gGuildClass.SearchGuild(lpMsg->szUnionMasterGuildName);
	GUILD_INFO_STRUCT * lpUnionMemberGuild = gGuildClass.SearchGuild(lpMsg->szUnionMemberGuildName);

	if ( lpMsg->btFlag == 1 )
	{
		if ( !lpUnionMasterGuild )
		{
			lpMsg->btResult = 0;
		}

		PMSG_KICKOUT_UNIONMEMBER_RESULT pMsg ={0};

		pMsg.h.set(0xEB, 0x01, sizeof(pMsg));
		pMsg.btResult = lpMsg->btResult;
		pMsg.btRequestType = 2;
		pMsg.btRelationShipType = 1;

		DataSend(lpMsg->wRequestUserIndex, (LPBYTE)&pMsg, sizeof(pMsg));

		if ( lpMsg->btResult == 0 || lpMsg->btResult == 16 )
			return;
	}
	else if ( lpMsg->btFlag == 0 )
	{
		if ( !lpUnionMemberGuild )
			return;

		if ( lpMsg->btResult == 0 || lpMsg->btResult == 16 )
			return;
	}		

	if ( lpUnionMasterGuild )
	{
		for ( int n=0;n<MAX_GUILD_USER;n++)
		{
			if ( lpUnionMasterGuild->Use[n] > 0 )
			{
				if ( lpUnionMasterGuild->Index[n] >= 0 )
				{
					gNotice.GCNoticeSend(lpUnionMasterGuild->Index[n],1,0,0,0,0,0,gMessage.GetMessage(440),lpMsg->szUnionMemberGuildName,lpMsg->szUnionMasterGuildName);
				}
			}
		}
	}

	if ( !lpUnionMemberGuild )
		return;

	lpUnionMemberGuild->SetGuildUnion(0);

	for ( int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( lpUnionMemberGuild->Use[n] > 0 )
		{
			if ( lpUnionMemberGuild->Index[n] >= 0 )
			{
				gNotice.GCNoticeSend(lpUnionMemberGuild->Index[n],1,0,0,0,0,0,gMessage.GetMessage(439));
			}
		}
	}
}

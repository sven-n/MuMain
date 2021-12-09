// PartyMatching.cpp: implementation of the CPartyMatching class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PartyMatching.h"
#include "GameMain.h"
#include "GensSystem.h"
#include "Party.h"
#include "ServerInfo.h"
#include "Util.h"

CPartyMatching gPartyMatching;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPartyMatching::CPartyMatching() // OK
{

}

CPartyMatching::~CPartyMatching() // OK
{

}

void CPartyMatching::CGPartyMatchingInsertRecv(PMSG_PARTY_MATCHING_INSERT_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_PARTY_MATCHING_INSERT_SEND pMsg;

	pMsg.header.set(0xEF,0x00,sizeof(pMsg));

	if(lpObj->Interface.use != 0 && lpObj->Interface.type == INTERFACE_PARTY)
	{
		pMsg.result = 0xFFFFFFFE;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->UsePartyMatching != 0 || lpObj->UsePartyMatchingJoin != 0)
	{
		pMsg.result = 0xFFFFFFFE;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(OBJECT_RANGE(lpObj->PartyNumber) != 0 && gParty.IsLeader(lpObj->PartyNumber,aIndex) == 0)
	{
		pMsg.result = 0xFFFFFFFD;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	this->GDPartyMatchingInsertSend(aIndex,lpMsg->text,lpMsg->password,lpMsg->MinLevel,lpMsg->MaxLevel,lpMsg->HuntingGround,lpMsg->WantedClass,lpMsg->WantedClassDetailInfo,lpMsg->UsePassword,lpMsg->ApprovalType);

	#endif
}

void CPartyMatching::CGPartyMatchingListRecv(PMSG_PARTY_MATCHING_LIST_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->GDPartyMatchingListSend(aIndex,lpMsg->page,lpMsg->SearchWord,lpMsg->UseSearchWord);

	#endif
}

void CPartyMatching::CGPartyMatchingJoinInsertRecv(PMSG_PARTY_MATCHING_JOIN_INSERT_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_PARTY_MATCHING_JOIN_INSERT_SEND pMsg;

	pMsg.header.set(0xEF,0x02,sizeof(pMsg));

	if(lpObj->Interface.use != 0 && lpObj->Interface.type == INTERFACE_PARTY)
	{
		pMsg.result = 0xFFFFFFFC;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->UsePartyMatching != 0 || lpObj->UsePartyMatchingJoin != 0)
	{
		pMsg.result = 0xFFFFFFFC;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(OBJECT_RANGE(lpObj->PartyNumber) != 0)
	{
		pMsg.result = 0xFFFFFFFA;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	this->GDPartyMatchingJoinInsertSend(aIndex,lpMsg->name,lpMsg->password,lpMsg->UsePassword,lpMsg->UseRandomParty);

	#endif
}

void CPartyMatching::CGPartyMatchingJoinInfoRecv(PMSG_PARTY_MATCHING_JOIN_INFO_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->GDPartyMatchingJoinInfoSend(aIndex);

	#endif
}

void CPartyMatching::CGPartyMatchingJoinListRecv(PMSG_PARTY_MATCHING_JOIN_LIST_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->GDPartyMatchingJoinListSend(aIndex);

	#endif
}

void CPartyMatching::CGPartyMatchingJoinAcceptRecv(PMSG_PARTY_MATCHING_JOIN_ACCEPT_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_PARTY_MATCHING_JOIN_ACCEPT_SEND pMsg;

	pMsg.header.set(0xEF,0x05,sizeof(pMsg));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.type = lpMsg->type;

	pMsg.flag = 0;

	if(lpMsg->type == 0)
	{
		this->GDPartyMatchingJoinAcceptSend(aIndex,lpMsg->name,lpMsg->type);
		return;
	}

	if(OBJECT_RANGE(lpObj->PartyNumber) != 0 && gParty.IsLeader(lpObj->PartyNumber,aIndex) == 0)
	{
		pMsg.result = 0xFFFFFFFF;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(OBJECT_RANGE(lpObj->PartyNumber) != 0 && gParty.GetMemberCount(lpObj->PartyNumber) >= MAX_PARTY_USER)
	{
		pMsg.result = 0xFFFFFFFE;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	this->GDPartyMatchingJoinAcceptSend(aIndex,lpMsg->name,lpMsg->type);

	#endif
}

void CPartyMatching::CGPartyMatchingJoinCancelRecv(PMSG_PARTY_MATCHING_JOIN_CANCEL_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->GDPartyMatchingJoinCancelSend(aIndex,lpMsg->type,0);

	#endif
}

void CPartyMatching::DGPartyMatchingInsertRecv(SDHP_PARTY_MATCHING_INSERT_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPartyMatchingInsertRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	lpObj->UsePartyMatching = ((lpMsg->result==0)?1:lpObj->UsePartyMatching);

	PMSG_PARTY_MATCHING_INSERT_SEND pMsg;

	pMsg.header.set(0xEF,0x00,sizeof(pMsg));

	pMsg.result = lpMsg->result;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPartyMatching::DGPartyMatchingListRecv(SDHP_PARTY_MATCHING_LIST_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPartyMatchingListRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	BYTE send[1024];

	PMSG_PARTY_MATCHING_LIST_SEND pMsg;

	pMsg.header.set(0xEF,0x01,0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	pMsg.CurPage = lpMsg->CurPage;

	pMsg.MaxPage = lpMsg->MaxPage;

	pMsg.result = lpMsg->result;

	PMSG_PARTY_MATCHING_LIST info;

	for(DWORD n=0;n < lpMsg->count;n++)
	{
		SDHP_PARTY_MATCHING_LIST* lpInfo = (SDHP_PARTY_MATCHING_LIST*)(((BYTE*)lpMsg)+sizeof(SDHP_PARTY_MATCHING_LIST_RECV)+(sizeof(SDHP_PARTY_MATCHING_LIST)*n));

		memcpy(info.name,lpInfo->name,sizeof(info.name));

		memcpy(info.text,lpInfo->text,sizeof(info.text));

		info.MinLevel = lpInfo->MinLevel;

		info.MaxLevel = lpInfo->MaxLevel;

		info.HuntingGround = lpInfo->HuntingGround;

		info.LeaderLevel = lpInfo->LeaderLevel;

		info.LeaderClass = lpInfo->LeaderClass;

		info.WantedClass = lpInfo->WantedClass;

		info.PartyMemberCount = lpInfo->PartyMemberCount;

		info.UsePassword = lpInfo->UsePassword;

		memcpy(info.WantedClassDetailInfo,lpInfo->WantedClassDetailInfo,sizeof(info.WantedClassDetailInfo));

		info.ServerCode = lpInfo->ServerCode+1;

		info.GensType = ((gServerInfo.m_GensSystemPartyLock==0)?lpObj->GensFamily:lpInfo->GensType);

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

void CPartyMatching::DGPartyMatchingJoinInsertRecv(SDHP_PARTY_MATCHING_JOIN_INSERT_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPartyMatchingJoinInsertRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	lpObj->UsePartyMatchingJoin = ((lpMsg->result==0)?1:lpObj->UsePartyMatchingJoin);

	PMSG_PARTY_MATCHING_JOIN_INSERT_SEND pMsg;

	pMsg.header.set(0xEF,0x02,sizeof(pMsg));

	pMsg.result = lpMsg->result;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPartyMatching::DGPartyMatchingJoinInfoRecv(SDHP_PARTY_MATCHING_JOIN_INFO_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPartyMatchingJoinInfoRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	PMSG_PARTY_MATCHING_JOIN_INFO_SEND pMsg;

	pMsg.header.set(0xEF,0x03,sizeof(pMsg));

	pMsg.result = lpMsg->result;

	pMsg.ServerCode = lpMsg->LeaderServerCode+1;

	memcpy(pMsg.name,lpMsg->LeaderName,sizeof(pMsg.name));

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPartyMatching::DGPartyMatchingJoinListRecv(SDHP_PARTY_MATCHING_JOIN_LIST_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPartyMatchingJoinListRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	BYTE send[1024];

	PMSG_PARTY_MATCHING_JOIN_LIST_SEND pMsg;

	pMsg.header.set(0xEF,0x04,0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	pMsg.result = lpMsg->result;

	PMSG_PARTY_MATCHING_JOIN_LIST info;

	for(DWORD n=0;n < lpMsg->count;n++)
	{
		SDHP_PARTY_MATCHING_JOIN_LIST* lpInfo = (SDHP_PARTY_MATCHING_JOIN_LIST*)(((BYTE*)lpMsg)+sizeof(SDHP_PARTY_MATCHING_JOIN_LIST_RECV)+(sizeof(SDHP_PARTY_MATCHING_JOIN_LIST)*n));

		memcpy(info.name,lpInfo->name,sizeof(info.name));

		info.Class = (((lpInfo->Class & 7)==0)?1:(((lpInfo->Class & 7)==1)?2:(((lpInfo->Class & 7)==2)?0:(lpInfo->Class & 7)))) | (lpInfo->Class & 24);

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

void CPartyMatching::DGPartyMatchingJoinAcceptRecv(SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPartyMatchingJoinAcceptRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	PMSG_PARTY_MATCHING_JOIN_ACCEPT_SEND pMsg;

	pMsg.header.set(0xEF,0x05,sizeof(pMsg));

	memcpy(pMsg.name,lpMsg->MemberName,sizeof(pMsg.name));

	pMsg.type = lpMsg->type;

	pMsg.flag = lpMsg->flag;

	pMsg.result = lpMsg->result;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPartyMatching::DGPartyMatchingJoinCancelRecv(SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPartyMatchingJoinCancelRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	lpObj->UsePartyMatching = ((lpMsg->type==0&&lpMsg->result==0)?0:lpObj->UsePartyMatching);

	lpObj->UsePartyMatchingJoin = ((lpMsg->type!=0&&lpMsg->result==0)?0:lpObj->UsePartyMatchingJoin);

	PMSG_PARTY_MATCHING_JOIN_CANCEL_SEND pMsg;

	pMsg.header.set(0xEF,0x06,sizeof(pMsg));

	pMsg.type = lpMsg->type;

	pMsg.result = lpMsg->result;

	if(lpMsg->flag == 0){DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);}

	#endif
}

void CPartyMatching::DGPartyMatchingNotifyRecv(SDHP_PARTY_MATCHING_NOTIFY_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPartyMatchingNotifyRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	LPOBJ lpTarget = gObjFind(lpMsg->LeaderName);

	if(lpMsg->result == 1 && lpTarget != 0)
	{
		if(OBJECT_RANGE(lpTarget->PartyNumber) == 0)
		{
			if(gParty.Create(lpTarget->Index) == 0 || gParty.AddMember(lpTarget->PartyNumber,lpObj->Index) == 0)
			{
				gParty.GCPartyResultSend(lpObj->Index,2);
				gParty.GCPartyResultSend(lpTarget->Index,2);
				gParty.Destroy(lpTarget->PartyNumber);
			}
		}
		else
		{
			if(gParty.AddMember(lpTarget->PartyNumber,lpObj->Index) == 0)
			{
				gParty.GCPartyResultSend(lpObj->Index,2);
				gParty.GCPartyResultSend(lpTarget->Index,2);
			}
		}
	}

	this->GDPartyMatchingJoinCancelSend(lpObj->Index,1,1);

	#endif
}

void CPartyMatching::DGPartyMatchingNotifyLeaderRecv(SDHP_PARTY_MATCHING_NOTIFY_LEADER_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPartyMatchingNotifyLeaderRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	PMSG_PARTY_MATCHING_JOIN_NOTIFY_SEND pMsg;

	pMsg.header.set(0xEF,0x08,sizeof(pMsg));

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPartyMatching::GDPartyMatchingInsertSend(int aIndex,char* text,char* password,WORD MinLevel,WORD MaxLevel,WORD HuntingGround,BYTE WantedClass,BYTE WantedClassDetailInfo[7],BYTE UsePassword,BYTE ApprovalType) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	if(gObj[aIndex].UsePartyMatching != 0)
	{
		return;
	}

	SDHP_PARTY_MATCHING_INSERT_SEND pMsg;

	pMsg.header.set(0x29,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	memcpy(pMsg.text,text,sizeof(pMsg.text));

	memcpy(pMsg.password,password,sizeof(pMsg.password));

	pMsg.MinLevel = MinLevel;

	pMsg.MaxLevel = MaxLevel;

	pMsg.HuntingGround = HuntingGround;

	pMsg.LeaderLevel = ((gObj[aIndex].Level>=MAX_CHARACTER_LEVEL)?(gObj[aIndex].Level+gObj[aIndex].MasterLevel):gObj[aIndex].Level);

	pMsg.LeaderClass = gObj[aIndex].Class | ((gObj[aIndex].ChangeUp==1)?8:((gObj[aIndex].ChangeUp==2)?24:0));

	pMsg.WantedClass = WantedClass;

	memcpy(pMsg.WantedClassDetailInfo,WantedClassDetailInfo,sizeof(pMsg.WantedClassDetailInfo));

	pMsg.PartyMemberCount = ((OBJECT_RANGE(gObj[aIndex].PartyNumber)==0)?1:gParty.GetMemberCount(gObj[aIndex].PartyNumber));

	pMsg.ApprovalType = ApprovalType;

	pMsg.UsePassword = UsePassword;

	pMsg.GensType = ((gServerInfo.m_GensSystemPartyLock==0)?GENS_FAMILY_NONE:gObj[aIndex].GensFamily);

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPartyMatching::GDPartyMatchingListSend(int aIndex,DWORD page,char* SearchWord,BYTE UseSearchWord) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_PARTY_MATCHING_LIST_SEND pMsg;

	pMsg.header.set(0x29,0x01,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.page = page;

	pMsg.Level = ((gObj[aIndex].Level>=MAX_CHARACTER_LEVEL)?(gObj[aIndex].Level+gObj[aIndex].MasterLevel):gObj[aIndex].Level);

	pMsg.Class = 1 << ((gObj[aIndex].Class==0)?2:((gObj[aIndex].Class==1)?0:((gObj[aIndex].Class==2)?1:gObj[aIndex].Class)));

	pMsg.GensType = ((gServerInfo.m_GensSystemPartyLock==0)?GENS_FAMILY_NONE:gObj[aIndex].GensFamily);

	pMsg.UseSearchWord = UseSearchWord;

	memcpy(pMsg.SearchWord,SearchWord,sizeof(pMsg.SearchWord));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPartyMatching::GDPartyMatchingJoinInsertSend(int aIndex,char* LeaderName,char* password,BYTE UsePassword,BYTE UseRandomParty) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	if(gObj[aIndex].UsePartyMatchingJoin != 0)
	{
		return;
	}

	SDHP_PARTY_MATCHING_JOIN_INSERT_SEND pMsg;

	pMsg.header.set(0x29,0x02,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	memcpy(pMsg.LeaderName,LeaderName,sizeof(pMsg.LeaderName));

	memcpy(pMsg.password,password,sizeof(pMsg.password));

	pMsg.Level = ((gObj[aIndex].Level>=MAX_CHARACTER_LEVEL)?(gObj[aIndex].Level+gObj[aIndex].MasterLevel):gObj[aIndex].Level);

	pMsg.Class = ((gObj[aIndex].Class==0)?2:((gObj[aIndex].Class==1)?0:((gObj[aIndex].Class==2)?1:gObj[aIndex].Class))) | ((gObj[aIndex].ChangeUp==1)?8:((gObj[aIndex].ChangeUp==2)?24:0));

	pMsg.UsePassword = UsePassword;

	pMsg.UseRandomParty = UseRandomParty;

	pMsg.GensType = ((gServerInfo.m_GensSystemPartyLock==0)?GENS_FAMILY_NONE:gObj[aIndex].GensFamily);

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPartyMatching::GDPartyMatchingJoinInfoSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_PARTY_MATCHING_JOIN_INFO_SEND pMsg;

	pMsg.header.set(0x29,0x03,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPartyMatching::GDPartyMatchingJoinListSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_PARTY_MATCHING_JOIN_LIST_SEND pMsg;

	pMsg.header.set(0x29,0x04,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPartyMatching::GDPartyMatchingJoinAcceptSend(int aIndex,char* MemberName,BYTE type) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_PARTY_MATCHING_JOIN_ACCEPT_SEND pMsg;

	pMsg.header.set(0x29,0x05,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	memcpy(pMsg.MemberName,MemberName,sizeof(pMsg.MemberName));

	pMsg.type = type;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPartyMatching::GDPartyMatchingJoinCancelSend(int aIndex,BYTE type,BYTE flag) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	if((type == 0 && gObj[aIndex].UsePartyMatching == 0) || (type != 0 && gObj[aIndex].UsePartyMatchingJoin == 0))
	{
		return;
	}

	SDHP_PARTY_MATCHING_JOIN_CANCEL_SEND pMsg;

	pMsg.header.set(0x29,0x06,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.type = type;

	pMsg.flag = flag;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPartyMatching::GDPartyMatchingInsertSaveSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObj[aIndex].UsePartyMatching == 0)
	{
		return;
	}

	SDHP_PARTY_MATCHING_INSERT_SAVE_SEND pMsg;

	pMsg.header.set(0x29,0x30,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.LeaderLevel = ((gObj[aIndex].Level>=MAX_CHARACTER_LEVEL)?(gObj[aIndex].Level+gObj[aIndex].MasterLevel):gObj[aIndex].Level);

	pMsg.LeaderClass = gObj[aIndex].Class | ((gObj[aIndex].ChangeUp==1)?8:((gObj[aIndex].ChangeUp==2)?24:0));

	pMsg.PartyMemberCount = ((OBJECT_RANGE(gObj[aIndex].PartyNumber)==0)?1:gParty.GetMemberCount(gObj[aIndex].PartyNumber));

	pMsg.GensType = ((gServerInfo.m_GensSystemPartyLock==0)?GENS_FAMILY_NONE:gObj[aIndex].GensFamily);

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

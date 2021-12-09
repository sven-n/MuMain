// PartyMatching.cpp: implementation of the CPartyMatching class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PartyMatching.h"
#include "CharacterManager.h"
#include "SocketManager.h"
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

void CPartyMatching::ClearPartyMatchingInfo(WORD ServerCode) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	for(std::map<std::string,PARTY_MATCHING_INFO>::iterator it=this->m_PartyMatchingInfo.begin();it != this->m_PartyMatchingInfo.end();)
	{
		CHARACTER_INFO CharacterInfo;

		if(gCharacterManager.GetCharacterInfo(&CharacterInfo,it->second.Name) != 0)
		{
			it++;
			continue;
		}

		this->RemovePartyMatchingJoinInfoNotifyAll(it->second);

		it = this->m_PartyMatchingInfo.erase(it);
	}

	this->m_critical.unlock();

	#endif
}

void CPartyMatching::ClearPartyMatchingJoinInfo(WORD ServerCode) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	for(std::map<std::string,PARTY_MATCHING_JOIN_INFO>::iterator it=this->m_PartyMatchingJoinInfo.begin();it != this->m_PartyMatchingJoinInfo.end();)
	{
		CHARACTER_INFO CharacterInfo;

		if(gCharacterManager.GetCharacterInfo(&CharacterInfo,it->second.Name) != 0)
		{
			it++;
			continue;
		}

		it = this->m_PartyMatchingJoinInfo.erase(it);
	}

	this->m_critical.unlock();

	#endif
}

bool CPartyMatching::GetPartyMatchingInfo(PARTY_MATCHING_INFO* lpPartyMatchingInfo,char* name) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::string nme(name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,PARTY_MATCHING_INFO>::iterator it = this->m_PartyMatchingInfo.find(nme);

	if(it != this->m_PartyMatchingInfo.end())
	{
		(*lpPartyMatchingInfo) = it->second;
		this->m_critical.unlock();
		return 1;
	}

	this->m_critical.unlock();
	return 0;

	#else

	return 0;

	#endif
}

bool CPartyMatching::GetPartyMatchingInfo(PARTY_MATCHING_INFO* lpPartyMatchingInfo,WORD ServerCode,WORD Level,BYTE Class,BYTE GensType) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::vector<PARTY_MATCHING_INFO> RandomPartyMatchingInfo;

	for(std::map<std::string,PARTY_MATCHING_INFO>::iterator it=this->m_PartyMatchingInfo.begin();it != this->m_PartyMatchingInfo.end();it++)
	{
		if(it->second.UsePassword == 0)
		{
			if(it->second.GensType == GensType)
			{
				if(it->second.ServerCode == ServerCode)
				{
					if(it->second.PartyMemberCount < MAX_PARTY_USER)
					{
						if((it->second.WantedClass & (1 << (Class & 7))) != 0)
						{
							if(it->second.MinLevel <= Level && it->second.MaxLevel >= Level)
							{
								RandomPartyMatchingInfo.push_back(it->second);
							}
						}
					}
				}
			}
		}
	}

	if(RandomPartyMatchingInfo.size() == 0)
	{
		this->m_critical.unlock();
		return 0;
	}
	else
	{
		(*lpPartyMatchingInfo) = RandomPartyMatchingInfo[rand()%RandomPartyMatchingInfo.size()];
		this->m_critical.unlock();
		return 1;
	}

	#else

	return 0;

	#endif
}

void CPartyMatching::InsertPartyMatchingInfo(PARTY_MATCHING_INFO PartyMatchingInfo) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::string nme(PartyMatchingInfo.Name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,PARTY_MATCHING_INFO>::iterator it = this->m_PartyMatchingInfo.find(nme);

	if(it == this->m_PartyMatchingInfo.end())
	{
		this->m_PartyMatchingInfo.insert(std::pair<std::string,PARTY_MATCHING_INFO>(nme,PartyMatchingInfo));
	}
	else
	{
		it->second = PartyMatchingInfo;
	}

	this->m_critical.unlock();

	#endif
}

void CPartyMatching::RemovePartyMatchingInfo(PARTY_MATCHING_INFO PartyMatchingInfo) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::string nme(PartyMatchingInfo.Name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,PARTY_MATCHING_INFO>::iterator it = this->m_PartyMatchingInfo.find(nme);

	if(it != this->m_PartyMatchingInfo.end())
	{
		this->m_PartyMatchingInfo.erase(it);
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();

	#endif
}

bool CPartyMatching::GetPartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO* lpPartyMatchingJoinInfo,char* name) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::string nme(name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,PARTY_MATCHING_JOIN_INFO>::iterator it = this->m_PartyMatchingJoinInfo.find(nme);

	if(it != this->m_PartyMatchingJoinInfo.end())
	{
		(*lpPartyMatchingJoinInfo) = it->second;
		this->m_critical.unlock();
		return 1;
	}

	this->m_critical.unlock();
	return 0;

	#else

	return 0;

	#endif
}

void CPartyMatching::InsertPartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::string nme(PartyMatchingJoinInfo.Name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,PARTY_MATCHING_JOIN_INFO>::iterator it = this->m_PartyMatchingJoinInfo.find(nme);

	if(it == this->m_PartyMatchingJoinInfo.end())
	{
		this->m_PartyMatchingJoinInfo.insert(std::pair<std::string,PARTY_MATCHING_JOIN_INFO>(nme,PartyMatchingJoinInfo));
	}
	else
	{
		it->second = PartyMatchingJoinInfo;
	}

	this->m_critical.unlock();

	#endif
}

void CPartyMatching::RemovePartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::string nme(PartyMatchingJoinInfo.Name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,PARTY_MATCHING_JOIN_INFO>::iterator it = this->m_PartyMatchingJoinInfo.find(nme);

	if(it != this->m_PartyMatchingJoinInfo.end())
	{
		this->m_PartyMatchingJoinInfo.erase(it);
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();

	#endif
}

void CPartyMatching::RemovePartyMatchingJoinInfoNotifyAll(PARTY_MATCHING_INFO PartyMatchingInfo) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	for(std::map<std::string,PARTY_MATCHING_JOIN_INFO>::iterator it=this->m_PartyMatchingJoinInfo.begin();it != this->m_PartyMatchingJoinInfo.end();it++)
	{
		if(_stricmp(it->second.LeaderName,PartyMatchingInfo.Name) == 0)
		{
			this->DGPartyMatchingNotifySend(it->second.Name,it->second.LeaderName,2);
		}
	}

	this->m_critical.unlock();

	#endif
}

DWORD CPartyMatching::GeneratePartyMatchingList(DWORD* CurPage,DWORD* MaxPage,BYTE UseSearchWord,char* SearchWord,BYTE* lpMsg,int* size) // OK
{
	#if(DATASERVER_UPDATE>=801)

	DWORD count = 0;

	DWORD PageCount = 0;

	this->m_critical.lock();

	SDHP_PARTY_MATCHING_LIST info;

	for(std::map<std::string,PARTY_MATCHING_INFO>::iterator it=this->m_PartyMatchingInfo.begin();it != this->m_PartyMatchingInfo.end();it++)
	{
		if(UseSearchWord == 0 || strstr(it->second.Text,SearchWord) != 0)
		{
			if(PageCount >= (((*CurPage)-1)*9) && PageCount < ((*CurPage)*9))
			{
				memcpy(info.text,it->second.Text,sizeof(info.text));

				memcpy(info.name,it->second.Name,sizeof(info.name));

				info.MinLevel = it->second.MinLevel;

				info.MaxLevel = it->second.MaxLevel;

				info.HuntingGround = it->second.HuntingGround;

				info.LeaderLevel = it->second.LeaderLevel;

				info.LeaderClass = it->second.LeaderClass;

				info.WantedClass = it->second.WantedClass;

				info.PartyMemberCount = it->second.PartyMemberCount;

				info.UsePassword = it->second.UsePassword;

				memcpy(info.WantedClassDetailInfo,it->second.WantedClassDetailInfo,sizeof(info.WantedClassDetailInfo));

				info.ServerCode = it->second.ServerCode;

				info.GensType = it->second.GensType;

				memcpy(&lpMsg[(*size)],&info,sizeof(info));
				(*size) += sizeof(info);

				count++;
			}

			PageCount++;
		}
	}

	(*MaxPage) = ((PageCount==0)?1:(((PageCount-1)/9)+1));

	this->m_critical.unlock();

	return count;

	#else

	return 0;

	#endif
}

DWORD CPartyMatching::GeneratePartyMatchingJoinList(char* LeaderName,BYTE* lpMsg,int* size) // OK
{
	#if(DATASERVER_UPDATE>=801)

	DWORD count = 0;

	this->m_critical.lock();

	SDHP_PARTY_MATCHING_JOIN_LIST info;

	for(std::map<std::string,PARTY_MATCHING_JOIN_INFO>::iterator it=this->m_PartyMatchingJoinInfo.begin();it != this->m_PartyMatchingJoinInfo.end();it++)
	{
		if(_stricmp(it->second.LeaderName,LeaderName) == 0)
		{
			memcpy(info.name,it->second.Name,sizeof(info.name));

			info.Class = it->second.Class;

			info.Level = it->second.Level;

			memcpy(&lpMsg[(*size)],&info,sizeof(info));
			(*size) += sizeof(info);

			count++;
		}
	}

	this->m_critical.unlock();

	return count;

	#else

	return 0;

	#endif
}

void CPartyMatching::GDPartyMatchingInsertRecv(SDHP_PARTY_MATCHING_INSERT_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	SDHP_PARTY_MATCHING_INSERT_SEND pMsg;

	pMsg.header.set(0x29,0x00,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	PARTY_MATCHING_INFO PartyMatchingInfo;

	if(this->GetPartyMatchingInfo(&PartyMatchingInfo,lpMsg->name) != 0)
	{
		pMsg.result = 0xFFFFFFFE;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	memcpy(PartyMatchingInfo.Name,lpMsg->name,sizeof(PartyMatchingInfo.Name));

	memcpy(PartyMatchingInfo.Text,lpMsg->text,sizeof(PartyMatchingInfo.Text));

	memcpy(PartyMatchingInfo.Password,lpMsg->password,sizeof(PartyMatchingInfo.Password));

	PartyMatchingInfo.MinLevel = lpMsg->MinLevel;

	PartyMatchingInfo.MaxLevel = lpMsg->MaxLevel;

	PartyMatchingInfo.HuntingGround = lpMsg->HuntingGround;

	PartyMatchingInfo.LeaderLevel = lpMsg->LeaderLevel;

	PartyMatchingInfo.LeaderClass = lpMsg->LeaderClass;

	PartyMatchingInfo.WantedClass = lpMsg->WantedClass;

	memcpy(PartyMatchingInfo.WantedClassDetailInfo,lpMsg->WantedClassDetailInfo,sizeof(PartyMatchingInfo.WantedClassDetailInfo));

	PartyMatchingInfo.PartyMemberCount = lpMsg->PartyMemberCount;

	PartyMatchingInfo.ApprovalType = lpMsg->ApprovalType;

	PartyMatchingInfo.UsePassword = lpMsg->UsePassword;

	PartyMatchingInfo.GensType = lpMsg->GensType;

	PartyMatchingInfo.ServerCode = gServerManager[index].m_ServerCode;

	this->InsertPartyMatchingInfo(PartyMatchingInfo);

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CPartyMatching::GDPartyMatchingListRecv(SDHP_PARTY_MATCHING_LIST_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	BYTE send[1024];

	SDHP_PARTY_MATCHING_LIST_SEND pMsg;

	pMsg.header.set(0x29,0x01,0);

	int size = sizeof(pMsg);

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	pMsg.CurPage = lpMsg->page;

	pMsg.MaxPage = 1;

	pMsg.count = this->GeneratePartyMatchingList(&pMsg.CurPage,&pMsg.MaxPage,lpMsg->UseSearchWord,lpMsg->SearchWord,send,&size);

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	gSocketManager.DataSend(index,send,size);

	#endif
}

void CPartyMatching::GDPartyMatchingJoinInsertRecv(SDHP_PARTY_MATCHING_JOIN_INSERT_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	SDHP_PARTY_MATCHING_JOIN_INSERT_SEND pMsg;

	pMsg.header.set(0x29,0x02,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	PARTY_MATCHING_INFO PartyMatchingInfo;

	PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo;

	if(lpMsg->UseRandomParty == 0 && this->GetPartyMatchingInfo(&PartyMatchingInfo,lpMsg->LeaderName) == 0)
	{
		pMsg.result = 0xFFFFFFFE;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	if(lpMsg->UseRandomParty != 0 && this->GetPartyMatchingInfo(&PartyMatchingInfo,gServerManager[index].m_ServerCode,lpMsg->Level,lpMsg->Class,lpMsg->GensType) == 0)
	{
		pMsg.result = 0xFFFFFFFD;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	if(PartyMatchingInfo.UsePassword != 0 && strcmp(PartyMatchingInfo.Password,lpMsg->password) != 0)
	{
		pMsg.result = 0xFFFFFFFF;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	if(this->GetPartyMatchingJoinInfo(&PartyMatchingJoinInfo,lpMsg->name) != 0)
	{
		pMsg.result = 0xFFFFFFFC;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	if(_stricmp(PartyMatchingInfo.Name,lpMsg->name) == 0)
	{
		pMsg.result = 0xFFFFFFFB;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	memcpy(PartyMatchingJoinInfo.Name,lpMsg->name,sizeof(PartyMatchingJoinInfo.Name));

	memcpy(PartyMatchingJoinInfo.LeaderName,PartyMatchingInfo.Name,sizeof(PartyMatchingJoinInfo.LeaderName));

	PartyMatchingJoinInfo.LeaderServerCode = PartyMatchingInfo.ServerCode;

	PartyMatchingJoinInfo.Class = lpMsg->Class;

	PartyMatchingJoinInfo.Level = lpMsg->Level;

	this->InsertPartyMatchingJoinInfo(PartyMatchingJoinInfo);

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	if(PartyMatchingInfo.ApprovalType == 0)
	{
		this->DGPartyMatchingNotifyLeaderSend(PartyMatchingInfo.Name,0);
	}
	else
	{
		this->DGPartyMatchingNotifySend(PartyMatchingJoinInfo.Name,PartyMatchingInfo.Name,1);
	}

	#endif
}

void CPartyMatching::GDPartyMatchingJoinInfoRecv(SDHP_PARTY_MATCHING_JOIN_INFO_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	SDHP_PARTY_MATCHING_JOIN_INFO_SEND pMsg;

	pMsg.header.set(0x29,0x03,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo;

	if(this->GetPartyMatchingJoinInfo(&PartyMatchingJoinInfo,lpMsg->name) == 0)
	{
		pMsg.result = 0xFFFFFFFF;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	pMsg.LeaderServerCode = PartyMatchingJoinInfo.LeaderServerCode;

	memcpy(pMsg.LeaderName,PartyMatchingJoinInfo.LeaderName,sizeof(pMsg.LeaderName));

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CPartyMatching::GDPartyMatchingJoinListRecv(SDHP_PARTY_MATCHING_JOIN_LIST_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	BYTE send[1024];

	SDHP_PARTY_MATCHING_JOIN_LIST_SEND pMsg;

	pMsg.header.set(0x29,0x04,0);

	int size = sizeof(pMsg);

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	pMsg.count = this->GeneratePartyMatchingJoinList(lpMsg->name,send,&size);

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	gSocketManager.DataSend(index,send,size);

	#endif
}

void CPartyMatching::GDPartyMatchingJoinAcceptRecv(SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	SDHP_PARTY_MATCHING_JOIN_ACCEPT_SEND pMsg;

	pMsg.header.set(0x29,0x05,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	memcpy(pMsg.MemberName,lpMsg->MemberName,sizeof(pMsg.MemberName));

	pMsg.type = lpMsg->type;

	pMsg.flag = 0;

	PARTY_MATCHING_INFO PartyMatchingInfo;

	PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo;

	if(this->GetPartyMatchingInfo(&PartyMatchingInfo,lpMsg->name) == 0 || this->GetPartyMatchingJoinInfo(&PartyMatchingJoinInfo,lpMsg->MemberName) == 0)
	{
		pMsg.result = 0xFFFFFFFF;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	this->DGPartyMatchingNotifySend(lpMsg->MemberName,lpMsg->name,((lpMsg->type==0)?2:1));

	#endif
}

void CPartyMatching::GDPartyMatchingJoinCancelRecv(SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	if(lpMsg->type == 0)
	{
		SDHP_PARTY_MATCHING_JOIN_CANCEL_SEND pMsg;

		pMsg.header.set(0x29,0x06,sizeof(pMsg));

		pMsg.index = lpMsg->index;

		memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

		memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

		pMsg.result = 0;

		pMsg.type = lpMsg->type;

		pMsg.flag = lpMsg->flag;

		PARTY_MATCHING_INFO PartyMatchingInfo;

		if(this->GetPartyMatchingInfo(&PartyMatchingInfo,lpMsg->name) == 0)
		{
			pMsg.result = 0xFFFFFFFF;
			gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
			return;
		}

		this->RemovePartyMatchingInfo(PartyMatchingInfo);

		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

		this->RemovePartyMatchingJoinInfoNotifyAll(PartyMatchingInfo);
	}
	else
	{
		SDHP_PARTY_MATCHING_JOIN_CANCEL_SEND pMsg;

		pMsg.header.set(0x29,0x06,sizeof(pMsg));

		pMsg.index = lpMsg->index;

		memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

		memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

		pMsg.result = 0;

		pMsg.type = lpMsg->type;

		pMsg.flag = lpMsg->flag;

		PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo;

		if(this->GetPartyMatchingJoinInfo(&PartyMatchingJoinInfo,lpMsg->name) == 0)
		{
			pMsg.result = 0xFFFFFFFF;
			gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
			return;
		}

		this->RemovePartyMatchingJoinInfo(PartyMatchingJoinInfo);

		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
	}

	#endif
}

void CPartyMatching::GDPartyMatchingInsertSaveRecv(SDHP_PARTY_MATCHING_INSERT_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=801)

	PARTY_MATCHING_INFO PartyMatchingInfo;

	if(this->GetPartyMatchingInfo(&PartyMatchingInfo,lpMsg->name) == 0)
	{
		return;
	}

	PartyMatchingInfo.LeaderLevel = lpMsg->LeaderLevel;

	PartyMatchingInfo.LeaderClass = lpMsg->LeaderClass;

	PartyMatchingInfo.PartyMemberCount = lpMsg->PartyMemberCount;

	PartyMatchingInfo.GensType = lpMsg->GensType;

	this->InsertPartyMatchingInfo(PartyMatchingInfo);

	#endif
}

void CPartyMatching::DGPartyMatchingNotifySend(char* name,char* LeaderName,DWORD result) // OK
{
	#if(DATASERVER_UPDATE>=801)

	CHARACTER_INFO CharacterInfo;

	if(gCharacterManager.GetCharacterInfo(&CharacterInfo,name) == 0)
	{
		return;
	}

	SDHP_PARTY_MATCHING_NOTIFY_SEND pMsg;

	pMsg.header.set(0x29,0x07,sizeof(pMsg));

	pMsg.index = CharacterInfo.UserIndex;

	memcpy(pMsg.account,CharacterInfo.Account,sizeof(pMsg.account));

	memcpy(pMsg.name,CharacterInfo.Name,sizeof(pMsg.name));

	memcpy(pMsg.LeaderName,LeaderName,sizeof(pMsg.LeaderName));

	pMsg.result = result;

	CServerManager* lpServerManager = FindServerByCode(CharacterInfo.GameServerCode);

	if(lpServerManager != 0)
	{
		gSocketManager.DataSend(lpServerManager->m_index,(BYTE*)&pMsg,pMsg.header.size);
	}

	#endif
}

void CPartyMatching::DGPartyMatchingNotifyLeaderSend(char* name,DWORD result) // OK
{
	#if(DATASERVER_UPDATE>=801)

	CHARACTER_INFO CharacterInfo;

	if(gCharacterManager.GetCharacterInfo(&CharacterInfo,name) == 0)
	{
		return;
	}

	SDHP_PARTY_MATCHING_NOTIFY_LEADER_SEND pMsg;

	pMsg.header.set(0x29,0x08,sizeof(pMsg));

	pMsg.index = CharacterInfo.UserIndex;

	memcpy(pMsg.account,CharacterInfo.Account,sizeof(pMsg.account));

	memcpy(pMsg.name,CharacterInfo.Name,sizeof(pMsg.name));

	pMsg.result = result;

	CServerManager* lpServerManager = FindServerByCode(CharacterInfo.GameServerCode);

	if(lpServerManager != 0)
	{
		gSocketManager.DataSend(lpServerManager->m_index,(BYTE*)&pMsg,pMsg.header.size);
	}

	#endif
}

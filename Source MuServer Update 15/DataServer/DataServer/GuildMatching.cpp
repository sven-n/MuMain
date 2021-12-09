// GuildMatching.cpp: implementation of the CGuildMatching class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GuildMatching.h"
#include "CharacterManager.h"
#include "SocketManager.h"
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

void CGuildMatching::ClearGuildMatchingInfo(WORD ServerCode) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	for(std::map<std::string,GUILD_MATCHING_INFO>::iterator it=this->m_GuildMatchingInfo.begin();it != this->m_GuildMatchingInfo.end();)
	{
		CHARACTER_INFO CharacterInfo;

		if(gCharacterManager.GetCharacterInfo(&CharacterInfo,it->second.Name) != 0)
		{
			it++;
			continue;
		}

		this->RemoveGuildMatchingJoinInfoNotifyAll(it->second);

		it = this->m_GuildMatchingInfo.erase(it);
	}

	this->m_critical.unlock();

	#endif
}

void CGuildMatching::ClearGuildMatchingJoinInfo(WORD ServerCode) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	for(std::map<std::string,GUILD_MATCHING_JOIN_INFO>::iterator it=this->m_GuildMatchingJoinInfo.begin();it != this->m_GuildMatchingJoinInfo.end();)
	{
		CHARACTER_INFO CharacterInfo;

		if(gCharacterManager.GetCharacterInfo(&CharacterInfo,it->second.Name) != 0)
		{
			it++;
			continue;
		}

		it = this->m_GuildMatchingJoinInfo.erase(it);
	}

	this->m_critical.unlock();

	#endif
}

bool CGuildMatching::GetGuildMatchingInfo(GUILD_MATCHING_INFO* lpGuildMatchingInfo,char* name) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::string nme(name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,GUILD_MATCHING_INFO>::iterator it = this->m_GuildMatchingInfo.find(nme);

	if(it != this->m_GuildMatchingInfo.end())
	{
		(*lpGuildMatchingInfo) = it->second;
		this->m_critical.unlock();
		return 1;
	}

	this->m_critical.unlock();
	return 0;

	#else

	return 0;

	#endif
}

void CGuildMatching::InsertGuildMatchingInfo(GUILD_MATCHING_INFO GuildMatchingInfo) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::string nme(GuildMatchingInfo.GuildName);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,GUILD_MATCHING_INFO>::iterator it = this->m_GuildMatchingInfo.find(nme);

	if(it == this->m_GuildMatchingInfo.end())
	{
		this->m_GuildMatchingInfo.insert(std::pair<std::string,GUILD_MATCHING_INFO>(nme,GuildMatchingInfo));
	}
	else
	{
		it->second = GuildMatchingInfo;
	}

	this->m_critical.unlock();

	#endif
}

void CGuildMatching::RemoveGuildMatchingInfo(GUILD_MATCHING_INFO GuildMatchingInfo) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::string nme(GuildMatchingInfo.GuildName);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,GUILD_MATCHING_INFO>::iterator it = this->m_GuildMatchingInfo.find(nme);

	if(it != this->m_GuildMatchingInfo.end())
	{
		this->m_GuildMatchingInfo.erase(it);
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();

	#endif
}

bool CGuildMatching::GetGuildMatchingJoinInfo(GUILD_MATCHING_JOIN_INFO* lpGuildMatchingJoinInfo,char* name) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::string nme(name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,GUILD_MATCHING_JOIN_INFO>::iterator it = this->m_GuildMatchingJoinInfo.find(nme);

	if(it != this->m_GuildMatchingJoinInfo.end())
	{
		(*lpGuildMatchingJoinInfo) = it->second;
		this->m_critical.unlock();
		return 1;
	}

	this->m_critical.unlock();
	return 0;

	#else

	return 0;

	#endif
}

void CGuildMatching::InsertGuildMatchingJoinInfo(GUILD_MATCHING_JOIN_INFO GuildMatchingJoinInfo) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::string nme(GuildMatchingJoinInfo.Name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,GUILD_MATCHING_JOIN_INFO>::iterator it = this->m_GuildMatchingJoinInfo.find(nme);

	if(it == this->m_GuildMatchingJoinInfo.end())
	{
		this->m_GuildMatchingJoinInfo.insert(std::pair<std::string,GUILD_MATCHING_JOIN_INFO>(nme,GuildMatchingJoinInfo));
	}
	else
	{
		it->second = GuildMatchingJoinInfo;
	}

	this->m_critical.unlock();

	#endif
}

void CGuildMatching::RemoveGuildMatchingJoinInfo(GUILD_MATCHING_JOIN_INFO GuildMatchingJoinInfo) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	std::string nme(GuildMatchingJoinInfo.Name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,GUILD_MATCHING_JOIN_INFO>::iterator it = this->m_GuildMatchingJoinInfo.find(nme);

	if(it != this->m_GuildMatchingJoinInfo.end())
	{
		this->m_GuildMatchingJoinInfo.erase(it);
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();

	#endif
}

void CGuildMatching::RemoveGuildMatchingJoinInfoNotifyAll(GUILD_MATCHING_INFO GuildMatchingInfo) // OK
{
	#if(DATASERVER_UPDATE>=801)

	this->m_critical.lock();

	for(std::map<std::string,GUILD_MATCHING_JOIN_INFO>::iterator it=this->m_GuildMatchingJoinInfo.begin();it != this->m_GuildMatchingJoinInfo.end();it++)
	{
		if(_stricmp(it->second.GuildName,GuildMatchingInfo.GuildName) == 0)
		{
			this->DGGuildMatchingNotifySend(it->second.Name,it->second.GuildName,2);
		}
	}

	this->m_critical.unlock();

	#endif
}

DWORD CGuildMatching::GenerateGuildMatchingList(DWORD* CurPage,DWORD* MaxPage,BYTE* lpMsg,int* size) // OK
{
	#if(DATASERVER_UPDATE>=801)

	DWORD count = 0;

	DWORD PageCount = 0;

	this->m_critical.lock();

	SDHP_GUILD_MATCHING_LIST info;

	for(std::map<std::string,GUILD_MATCHING_INFO>::iterator it=this->m_GuildMatchingInfo.begin();it != this->m_GuildMatchingInfo.end();it++)
	{
		if(PageCount >= (((*CurPage)-1)*9) && PageCount < ((*CurPage)*9))
		{
			memcpy(info.text,it->second.Text,sizeof(info.text));

			memcpy(info.name,it->second.Name,sizeof(info.name));

			memcpy(info.GuildName,it->second.GuildName,sizeof(info.GuildName));

			info.GuildMemberCount = it->second.GuildMemberCount;

			info.GuildMasterClass = it->second.GuildMasterClass;

			info.InterestType = it->second.InterestType;

			info.LevelRange = it->second.LevelRange;

			info.ClassType = it->second.ClassType;

			info.GuildMasterLevel = it->second.GuildMasterLevel;

			info.BoardNumber = it->second.BoardNumber;

			info.GuildNumber = it->second.GuildNumber;

			info.GensType = it->second.GensType;

			memcpy(&lpMsg[(*size)],&info,sizeof(info));
			(*size) += sizeof(info);

			count++;
		}

		PageCount++;
	}

	(*MaxPage) = ((PageCount==0)?1:(((PageCount-1)/9)+1));

	this->m_critical.unlock();

	return count;

	#else

	return 0;

	#endif
}

DWORD CGuildMatching::GenerateGuildMatchingList(DWORD* CurPage,DWORD* MaxPage,char* SearchWord,BYTE* lpMsg,int* size) // OK
{
	#if(DATASERVER_UPDATE>=801)

	DWORD count = 0;

	DWORD PageCount = 0;

	this->m_critical.lock();

	SDHP_GUILD_MATCHING_LIST info;

	for(std::map<std::string,GUILD_MATCHING_INFO>::iterator it=this->m_GuildMatchingInfo.begin();it != this->m_GuildMatchingInfo.end();it++)
	{
		if(strstr(it->second.Text,SearchWord) != 0)
		{
			if(PageCount >= (((*CurPage)-1)*9) && PageCount < ((*CurPage)*9))
			{
				memcpy(info.text,it->second.Text,sizeof(info.text));

				memcpy(info.name,it->second.Name,sizeof(info.name));

				memcpy(info.GuildName,it->second.GuildName,sizeof(info.GuildName));

				info.GuildMemberCount = it->second.GuildMemberCount;

				info.GuildMasterClass = it->second.GuildMasterClass;

				info.InterestType = it->second.InterestType;

				info.LevelRange = it->second.LevelRange;

				info.ClassType = it->second.ClassType;

				info.GuildMasterLevel = it->second.GuildMasterLevel;

				info.BoardNumber = it->second.BoardNumber;

				info.GuildNumber = it->second.GuildNumber;

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

DWORD CGuildMatching::GenerateGuildMatchingJoinList(char* GuildName,BYTE* lpMsg,int* size) // OK
{
	#if(DATASERVER_UPDATE>=801)

	DWORD count = 0;

	this->m_critical.lock();

	SDHP_GUILD_MATCHING_JOIN_LIST info;

	for(std::map<std::string,GUILD_MATCHING_JOIN_INFO>::iterator it=this->m_GuildMatchingJoinInfo.begin();it != this->m_GuildMatchingJoinInfo.end();it++)
	{
		if(_stricmp(it->second.GuildName,GuildName) == 0)
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

void CGuildMatching::GDGuildMatchingListRecv(SDHP_GUILD_MATCHING_LIST_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	BYTE send[1024];

	SDHP_GUILD_MATCHING_LIST_SEND pMsg;

	pMsg.header.set(0x28,0x00,0);

	int size = sizeof(pMsg);

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	pMsg.CurPage = lpMsg->page;

	pMsg.MaxPage = 1;

	pMsg.count = this->GenerateGuildMatchingList(&pMsg.CurPage,&pMsg.MaxPage,send,&size);

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	gSocketManager.DataSend(index,send,size);

	#endif
}

void CGuildMatching::GDGuildMatchingListSearchRecv(SDHP_GUILD_MATCHING_LIST_SEARCH_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	BYTE send[1024];

	SDHP_GUILD_MATCHING_LIST_SEND pMsg;

	pMsg.header.set(0x28,0x00,0);

	int size = sizeof(pMsg);

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	pMsg.CurPage = lpMsg->page;

	pMsg.MaxPage = 1;

	pMsg.count = this->GenerateGuildMatchingList(&pMsg.CurPage,&pMsg.MaxPage,lpMsg->SearchWord,send,&size);

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	gSocketManager.DataSend(index,send,size);

	#endif
}

void CGuildMatching::GDGuildMatchingInsertRecv(SDHP_GUILD_MATCHING_INSERT_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	SDHP_GUILD_MATCHING_INSERT_SEND pMsg;

	pMsg.header.set(0x28,0x02,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	GUILD_MATCHING_INFO GuildMatchingInfo;

	if(this->GetGuildMatchingInfo(&GuildMatchingInfo,lpMsg->GuildName) != 0)
	{
		pMsg.result = 0xFFFFFFFF;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	memcpy(GuildMatchingInfo.Name,lpMsg->name,sizeof(GuildMatchingInfo.Name));

	memcpy(GuildMatchingInfo.Text,lpMsg->text,sizeof(GuildMatchingInfo.Text));

	memcpy(GuildMatchingInfo.GuildName,lpMsg->GuildName,sizeof(GuildMatchingInfo.GuildName));

	GuildMatchingInfo.GuildMemberCount = lpMsg->GuildMemberCount;

	GuildMatchingInfo.GuildMasterClass = lpMsg->GuildMasterClass;

	GuildMatchingInfo.InterestType = lpMsg->InterestType;

	GuildMatchingInfo.LevelRange = lpMsg->LevelRange;

	GuildMatchingInfo.ClassType = lpMsg->ClassType;

	GuildMatchingInfo.GuildMasterLevel = lpMsg->GuildMasterLevel;

	GuildMatchingInfo.BoardNumber = 0;

	GuildMatchingInfo.GuildNumber = lpMsg->GuildNumber;

	GuildMatchingInfo.GensType = lpMsg->GensType;

	this->InsertGuildMatchingInfo(GuildMatchingInfo);

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CGuildMatching::GDGuildMatchingCancelRecv(SDHP_GUILD_MATCHING_CANCEL_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	SDHP_GUILD_MATCHING_CANCEL_SEND pMsg;

	pMsg.header.set(0x28,0x03,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	pMsg.flag = lpMsg->flag;

	GUILD_MATCHING_INFO GuildMatchingInfo;

	if(this->GetGuildMatchingInfo(&GuildMatchingInfo,lpMsg->GuildName) == 0)
	{
		pMsg.result = 0xFFFFFFFF;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	this->RemoveGuildMatchingInfo(GuildMatchingInfo);

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	this->RemoveGuildMatchingJoinInfoNotifyAll(GuildMatchingInfo);

	#endif
}

void CGuildMatching::GDGuildMatchingJoinInsertRecv(SDHP_GUILD_MATCHING_JOIN_INSERT_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	SDHP_GUILD_MATCHING_JOIN_INSERT_SEND pMsg;

	pMsg.header.set(0x28,0x04,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	GUILD_MATCHING_INFO GuildMatchingInfo;

	GUILD_MATCHING_JOIN_INFO GuildMatchingJoinInfo;

	if(this->GetGuildMatchingInfo(&GuildMatchingInfo,lpMsg->GuildName) == 0 || this->GetGuildMatchingJoinInfo(&GuildMatchingJoinInfo,lpMsg->name) != 0)
	{
		pMsg.result = 0xFFFFFFFF;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	memcpy(GuildMatchingJoinInfo.Name,lpMsg->name,sizeof(GuildMatchingInfo.Name));

	memcpy(GuildMatchingJoinInfo.GuildName,GuildMatchingInfo.GuildName,sizeof(GuildMatchingJoinInfo.GuildName));

	memcpy(GuildMatchingJoinInfo.GuildMasterName,GuildMatchingInfo.Name,sizeof(GuildMatchingJoinInfo.GuildMasterName));

	GuildMatchingJoinInfo.Class = lpMsg->Class;

	GuildMatchingJoinInfo.Level = lpMsg->Level;

	this->InsertGuildMatchingJoinInfo(GuildMatchingJoinInfo);

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	this->DGGuildMatchingNotifyMasterSend(GuildMatchingInfo.Name,0);

	#endif
}

void CGuildMatching::GDGuildMatchingJoinCancelRecv(SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	SDHP_GUILD_MATCHING_JOIN_CANCEL_SEND pMsg;

	pMsg.header.set(0x28,0x05,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	pMsg.flag = lpMsg->flag;

	GUILD_MATCHING_JOIN_INFO GuildMatchingJoinInfo;

	if(this->GetGuildMatchingJoinInfo(&GuildMatchingJoinInfo,lpMsg->name) == 0)
	{
		pMsg.result = 0xFFFFFFFF;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	this->RemoveGuildMatchingJoinInfo(GuildMatchingJoinInfo);

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CGuildMatching::GDGuildMatchingJoinAcceptRecv(SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	SDHP_GUILD_MATCHING_JOIN_ACCEPT_SEND pMsg;

	pMsg.header.set(0x28,0x06,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	pMsg.type = lpMsg->type;

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.MemberName,lpMsg->MemberName,sizeof(pMsg.MemberName));

	GUILD_MATCHING_INFO GuildMatchingInfo;

	GUILD_MATCHING_JOIN_INFO GuildMatchingJoinInfo;

	if(this->GetGuildMatchingInfo(&GuildMatchingInfo,lpMsg->GuildName) == 0 || this->GetGuildMatchingJoinInfo(&GuildMatchingJoinInfo,lpMsg->MemberName) == 0)
	{
		pMsg.result = 0xFFFFFFFF;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	CHARACTER_INFO CharacterInfo;

	if(gCharacterManager.GetCharacterInfo(&CharacterInfo,lpMsg->MemberName) == 0)
	{
		pMsg.result = 1;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		this->RemoveGuildMatchingJoinInfo(GuildMatchingJoinInfo);
	}
	else
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		this->DGGuildMatchingNotifySend(lpMsg->MemberName,lpMsg->GuildName,((lpMsg->type==0)?2:1));
	}

	#endif
}

void CGuildMatching::GDGuildMatchingJoinListRecv(SDHP_GUILD_MATCHING_JOIN_LIST_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	BYTE send[2048];

	SDHP_GUILD_MATCHING_JOIN_LIST_SEND pMsg;

	pMsg.header.set(0x28,0x07,0);

	int size = sizeof(pMsg);

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	pMsg.count = this->GenerateGuildMatchingJoinList(lpMsg->GuildName,send,&size);

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	gSocketManager.DataSend(index,send,size);

	#endif
}

void CGuildMatching::GDGuildMatchingJoinInfoRecv(SDHP_GUILD_MATCHING_JOIN_INFO_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=801)

	SDHP_GUILD_MATCHING_JOIN_INFO_SEND pMsg;

	pMsg.header.set(0x28,0x08,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 0;

	GUILD_MATCHING_JOIN_INFO GuildMatchingJoinInfo;

	if(this->GetGuildMatchingJoinInfo(&GuildMatchingJoinInfo,lpMsg->name) == 0)
	{
		pMsg.result = 0xFFFFFFFE;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	memcpy(pMsg.GuildName,GuildMatchingJoinInfo.GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.GuildMasterName,GuildMatchingJoinInfo.GuildMasterName,sizeof(pMsg.GuildMasterName));

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CGuildMatching::GDGuildMatchingInsertSaveRecv(SDHP_GUILD_MATCHING_INSERT_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=801)

	GUILD_MATCHING_INFO GuildMatchingInfo;

	if(this->GetGuildMatchingInfo(&GuildMatchingInfo,lpMsg->GuildName) == 0)
	{
		return;
	}

	GuildMatchingInfo.GuildMemberCount = lpMsg->GuildMemberCount;

	GuildMatchingInfo.GuildMasterClass = lpMsg->GuildMasterClass;

	GuildMatchingInfo.GuildMasterLevel = lpMsg->GuildMasterLevel;

	GuildMatchingInfo.GensType = lpMsg->GensType;

	this->InsertGuildMatchingInfo(GuildMatchingInfo);

	#endif
}

void CGuildMatching::DGGuildMatchingNotifySend(char* name,char* GuildName,DWORD result) // OK
{
	#if(DATASERVER_UPDATE>=801)

	CHARACTER_INFO CharacterInfo;

	if(gCharacterManager.GetCharacterInfo(&CharacterInfo,name) == 0)
	{
		return;
	}

	SDHP_GUILD_MATCHING_NOTIFY_SEND pMsg;

	pMsg.header.set(0x28,0x09,sizeof(pMsg));

	pMsg.index = CharacterInfo.UserIndex;

	memcpy(pMsg.account,CharacterInfo.Account,sizeof(pMsg.account));

	memcpy(pMsg.name,CharacterInfo.Name,sizeof(pMsg.name));

	pMsg.result = result;

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	CServerManager* lpServerManager = FindServerByCode(CharacterInfo.GameServerCode);

	if(lpServerManager != 0)
	{
		gSocketManager.DataSend(lpServerManager->m_index,(BYTE*)&pMsg,pMsg.header.size);
	}

	#endif
}

void CGuildMatching::DGGuildMatchingNotifyMasterSend(char* name,DWORD result) // OK
{
	#if(DATASERVER_UPDATE>=801)

	CHARACTER_INFO CharacterInfo;

	if(gCharacterManager.GetCharacterInfo(&CharacterInfo,name) == 0)
	{
		return;
	}

	SDHP_GUILD_MATCHING_NOTIFY_MASTER_SEND pMsg;

	pMsg.header.set(0x28,0x0A,sizeof(pMsg));

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

// GuildMatching.h: interface for the CGuildMatching class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CriticalSection.h"
#include "DataServerProtocol.h"

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_GUILD_MATCHING_LIST_RECV
{
	PSBMSG_HEAD header; // C1:28:00
	WORD index;
	char account[11];
	char name[11];
	DWORD page;
};

struct SDHP_GUILD_MATCHING_LIST_SEARCH_RECV
{
	PSBMSG_HEAD header; // C1:28:01
	WORD index;
	char account[11];
	char name[11];
	DWORD page;
	char SearchWord[11];
};

struct SDHP_GUILD_MATCHING_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:28:02
	WORD index;
	char account[11];
	char name[11];
	char text[41];
	char GuildName[9];
	DWORD GuildNumber;
	BYTE GuildMemberCount;
	WORD GuildMasterLevel;
	BYTE GuildMasterClass;
	BYTE InterestType;
	BYTE LevelRange;
	BYTE ClassType;
	BYTE GensType;
};

struct SDHP_GUILD_MATCHING_CANCEL_RECV
{
	PSBMSG_HEAD header; // C1:28:03
	WORD index;
	char account[11];
	char name[11];
	char GuildName[9];
	BYTE flag;
};

struct SDHP_GUILD_MATCHING_JOIN_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:28:04
	WORD index;
	char account[11];
	char name[11];
	char GuildName[9];
	BYTE Class;
	WORD Level;
};

struct SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV
{
	PSBMSG_HEAD header; // C1:28:05
	WORD index;
	char account[11];
	char name[11];
	BYTE flag;
};

struct SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV
{
	PSBMSG_HEAD header; // C1:28:06
	WORD index;
	char account[11];
	char name[11];
	BYTE type;
	char GuildName[9];
	char MemberName[11];
};

struct SDHP_GUILD_MATCHING_JOIN_LIST_RECV
{
	PSBMSG_HEAD header; // C1:28:07
	WORD index;
	char account[11];
	char name[11];
	char GuildName[9];
};

struct SDHP_GUILD_MATCHING_JOIN_INFO_RECV
{
	PSBMSG_HEAD header; // C1:28:08
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_GUILD_MATCHING_INSERT_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:28:30
	WORD index;
	char account[11];
	char name[11];
	char GuildName[9];
	BYTE GuildMemberCount;
	WORD GuildMasterLevel;
	BYTE GuildMasterClass;
	BYTE GensType;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_GUILD_MATCHING_LIST_SEND
{
	PSWMSG_HEAD header; // C2:28:00
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	DWORD CurPage;
	DWORD MaxPage;
	DWORD count;
};

struct SDHP_GUILD_MATCHING_LIST
{
	char text[41];
	char name[11];
	char GuildName[9];
	BYTE GuildMemberCount;
	BYTE GuildMasterClass;
	BYTE InterestType;
	BYTE LevelRange;
	BYTE ClassType;
	DWORD GuildMasterLevel;
	DWORD BoardNumber;
	DWORD GuildNumber;
	BYTE GensType;
};

struct SDHP_GUILD_MATCHING_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:28:02
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
};

struct SDHP_GUILD_MATCHING_CANCEL_SEND
{
	PSBMSG_HEAD header; // C1:28:03
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	BYTE flag;
};

struct SDHP_GUILD_MATCHING_JOIN_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:28:04
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
};

struct SDHP_GUILD_MATCHING_JOIN_CANCEL_SEND
{
	PSBMSG_HEAD header; // C1:28:05
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	BYTE flag;
};

struct SDHP_GUILD_MATCHING_JOIN_ACCEPT_SEND
{
	PSBMSG_HEAD header; // C1:28:06
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	DWORD type;
	char GuildName[9];
	char MemberName[11];
};

struct SDHP_GUILD_MATCHING_JOIN_LIST_SEND
{
	PSWMSG_HEAD header; // C2:28:07
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	DWORD count;
};

struct SDHP_GUILD_MATCHING_JOIN_LIST
{
	char name[11];
	BYTE Class;
	DWORD Level;
};

struct SDHP_GUILD_MATCHING_JOIN_INFO_SEND
{
	PSBMSG_HEAD header; // C1:28:08
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	char GuildName[9];
	char GuildMasterName[11];
};

struct SDHP_GUILD_MATCHING_NOTIFY_SEND
{
	PSBMSG_HEAD header; // C1:28:09
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	char GuildName[9];
};

struct SDHP_GUILD_MATCHING_NOTIFY_MASTER_SEND
{
	PSBMSG_HEAD header; // C1:28:0A
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct GUILD_MATCHING_INFO
{
	char Name[11];
	char Text[41];
	char GuildName[9];
	BYTE GuildMemberCount;
	BYTE GuildMasterClass;
	BYTE InterestType;
	BYTE LevelRange;
	BYTE ClassType;
	DWORD GuildMasterLevel;
	DWORD BoardNumber;
	DWORD GuildNumber;
	BYTE GensType;
};

struct GUILD_MATCHING_JOIN_INFO
{
	char Name[11];
	char GuildName[9];
	char GuildMasterName[11];
	BYTE Class;
	DWORD Level;
};

class CGuildMatching
{
public:
	CGuildMatching();
	virtual ~CGuildMatching();
	void ClearGuildMatchingInfo(WORD ServerCode);
	void ClearGuildMatchingJoinInfo(WORD ServerCode);
	bool GetGuildMatchingInfo(GUILD_MATCHING_INFO* lpGuildMatchingInfo,char* name);
	void InsertGuildMatchingInfo(GUILD_MATCHING_INFO GuildMatchingInfo);
	void RemoveGuildMatchingInfo(GUILD_MATCHING_INFO GuildMatchingInfo);
	bool GetGuildMatchingJoinInfo(GUILD_MATCHING_JOIN_INFO* lpGuildMatchingJoinInfo,char* name);
	void InsertGuildMatchingJoinInfo(GUILD_MATCHING_JOIN_INFO GuildMatchingJoinInfo);
	void RemoveGuildMatchingJoinInfo(GUILD_MATCHING_JOIN_INFO GuildMatchingJoinInfo);
	void RemoveGuildMatchingJoinInfoNotifyAll(GUILD_MATCHING_INFO GuildMatchingInfo);
	DWORD GenerateGuildMatchingList(DWORD* CurPage,DWORD* MaxPage,BYTE* lpMsg,int* size);
	DWORD GenerateGuildMatchingList(DWORD* CurPage,DWORD* MaxPage,char* SearchWord,BYTE* lpMsg,int* size);
	DWORD GenerateGuildMatchingJoinList(char* GuildName,BYTE* lpMsg,int* size);
	void GDGuildMatchingListRecv(SDHP_GUILD_MATCHING_LIST_RECV* lpMsg,int index);
	void GDGuildMatchingListSearchRecv(SDHP_GUILD_MATCHING_LIST_SEARCH_RECV* lpMsg,int index);
	void GDGuildMatchingInsertRecv(SDHP_GUILD_MATCHING_INSERT_RECV* lpMsg,int index);
	void GDGuildMatchingCancelRecv(SDHP_GUILD_MATCHING_CANCEL_RECV* lpMsg,int index);
	void GDGuildMatchingJoinInsertRecv(SDHP_GUILD_MATCHING_JOIN_INSERT_RECV* lpMsg,int index);
	void GDGuildMatchingJoinCancelRecv(SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV* lpMsg,int index);
	void GDGuildMatchingJoinAcceptRecv(SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV* lpMsg,int index);
	void GDGuildMatchingJoinListRecv(SDHP_GUILD_MATCHING_JOIN_LIST_RECV* lpMsg,int index);
	void GDGuildMatchingJoinInfoRecv(SDHP_GUILD_MATCHING_JOIN_INFO_RECV* lpMsg,int index);
	void GDGuildMatchingInsertSaveRecv(SDHP_GUILD_MATCHING_INSERT_SAVE_RECV* lpMsg);
	void DGGuildMatchingNotifySend(char* name,char* GuildName,DWORD result);
	void DGGuildMatchingNotifyMasterSend(char* name,DWORD result);
private:
	CCriticalSection m_critical;
	std::map<std::string,GUILD_MATCHING_INFO> m_GuildMatchingInfo;
	std::map<std::string,GUILD_MATCHING_JOIN_INFO> m_GuildMatchingJoinInfo;
};

extern CGuildMatching gGuildMatching;

// GuildMatching.h: interface for the CGuildMatching class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_GUILD_MATCHING_LIST_RECV
{
	PSBMSG_HEAD header; // C1:ED:00
	DWORD page;
};

struct PMSG_GUILD_MATCHING_LIST_SEARCH_RECV
{
	PSBMSG_HEAD header; // C1:ED:01
	DWORD page;
	char SearchWord[11];
};

struct PMSG_GUILD_MATCHING_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:ED:02
	char text[41];
	BYTE InterestType;
	BYTE LevelRange;
	BYTE ClassType;
};

struct PMSG_GUILD_MATCHING_CANCEL_RECV
{
	PSBMSG_HEAD header; // C1:ED:03
};

struct PMSG_GUILD_MATCHING_JOIN_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:ED:04
	DWORD GuildNumber;
};

struct PMSG_GUILD_MATCHING_JOIN_CANCEL_RECV
{
	PSBMSG_HEAD header; // C1:ED:05
};

struct PMSG_GUILD_MATCHING_JOIN_ACCEPT_RECV
{
	PSBMSG_HEAD header; // C1:ED:06
	DWORD type;
	char name[11];
};

struct PMSG_GUILD_MATCHING_JOIN_LIST_RECV
{
	PSBMSG_HEAD header; // C1:ED:07
};

struct PMSG_GUILD_MATCHING_JOIN_INFO_RECV
{
	PSBMSG_HEAD header; // C1:ED:08
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_GUILD_MATCHING_LIST_SEND
{
	PSWMSG_HEAD header; // C2:ED:00
	DWORD CurPage;
	DWORD MaxPage;
	DWORD count;
	DWORD result;
};

struct PMSG_GUILD_MATCHING_LIST
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

struct PMSG_GUILD_MATCHING_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:ED:02
	DWORD result;
};

struct PMSG_GUILD_MATCHING_CANCEL_SEND
{
	PSBMSG_HEAD header; // C1:ED:03
	DWORD result;
};

struct PMSG_GUILD_MATCHING_JOIN_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:ED:04
	DWORD result;
};

struct PMSG_GUILD_MATCHING_JOIN_CANCEL_SEND
{
	PSBMSG_HEAD header; // C1:ED:05
	DWORD result;
};

struct PMSG_GUILD_MATCHING_JOIN_ACCEPT_SEND
{
	PSBMSG_HEAD header; // C1:ED:06
	char name[11];
	DWORD type;
	DWORD result;
};

struct PMSG_GUILD_MATCHING_JOIN_LIST_SEND
{
	PSWMSG_HEAD header; // C2:ED:07
	DWORD count;
	DWORD result;
};

struct PMSG_GUILD_MATCHING_JOIN_LIST
{
	char name[11];
	BYTE Class;
	DWORD Level;
};

struct PMSG_GUILD_MATCHING_JOIN_INFO_SEND
{
	PSBMSG_HEAD header; // C1:ED:08
	char GuildMasterName[11];
	char GuildName[9];
	DWORD result;
};

struct PMSG_GUILD_MATCHING_NOTIFY_SEND
{
	PSBMSG_HEAD header; // C1:ED:09
	DWORD result;
};

struct PMSG_GUILD_MATCHING_NOTIFY_MASTER_SEND
{
	PSBMSG_HEAD header; // C1:ED:10
	DWORD result;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_GUILD_MATCHING_LIST_RECV
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

struct SDHP_GUILD_MATCHING_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:28:02
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
};

struct SDHP_GUILD_MATCHING_CANCEL_RECV
{
	PSBMSG_HEAD header; // C1:28:03
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	BYTE flag;
};

struct SDHP_GUILD_MATCHING_JOIN_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:28:04
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
};

struct SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV
{
	PSBMSG_HEAD header; // C1:28:05
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	BYTE flag;
};

struct SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV
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

struct SDHP_GUILD_MATCHING_JOIN_LIST_RECV
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

struct SDHP_GUILD_MATCHING_JOIN_INFO_RECV
{
	PSBMSG_HEAD header; // C1:28:08
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	char GuildName[9];
	char GuildMasterName[11];
};

struct SDHP_GUILD_MATCHING_NOTIFY_RECV
{
	PSBMSG_HEAD header; // C1:28:09
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	char GuildName[9];
};

struct SDHP_GUILD_MATCHING_NOTIFY_MASTER_RECV
{
	PSBMSG_HEAD header; // C1:28:0A
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_GUILD_MATCHING_LIST_SEND
{
	PSBMSG_HEAD header; // C1:28:00
	WORD index;
	char account[11];
	char name[11];
	DWORD page;
};

struct SDHP_GUILD_MATCHING_LIST_SEARCH_SEND
{
	PSBMSG_HEAD header; // C1:28:01
	WORD index;
	char account[11];
	char name[11];
	DWORD page;
	char SearchWord[11];
};

struct SDHP_GUILD_MATCHING_INSERT_SEND
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

struct SDHP_GUILD_MATCHING_CANCEL_SEND
{
	PSBMSG_HEAD header; // C1:28:03
	WORD index;
	char account[11];
	char name[11];
	char GuildName[9];
	BYTE flag;
};

struct SDHP_GUILD_MATCHING_JOIN_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:28:04
	WORD index;
	char account[11];
	char name[11];
	char GuildName[9];
	BYTE Class;
	WORD Level;
};

struct SDHP_GUILD_MATCHING_JOIN_CANCEL_SEND
{
	PSBMSG_HEAD header; // C1:28:05
	WORD index;
	char account[11];
	char name[11];
	BYTE flag;
};

struct SDHP_GUILD_MATCHING_JOIN_ACCEPT_SEND
{
	PSBMSG_HEAD header; // C1:28:06
	WORD index;
	char account[11];
	char name[11];
	BYTE type;
	char GuildName[9];
	char MemberName[11];
};

struct SDHP_GUILD_MATCHING_JOIN_LIST_SEND
{
	PSBMSG_HEAD header; // C1:28:07
	WORD index;
	char account[11];
	char name[11];
	char GuildName[9];
};

struct SDHP_GUILD_MATCHING_JOIN_INFO_SEND
{
	PSBMSG_HEAD header; // C1:28:08
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_GUILD_MATCHING_INSERT_SAVE_SEND
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
//**********************************************//
//**********************************************//

class CGuildMatching
{
public:
	CGuildMatching();
	virtual ~CGuildMatching();
	void CGGuildMatchingListRecv(PMSG_GUILD_MATCHING_LIST_RECV* lpMsg,int aIndex);
	void CGGuildMatchingListSearchRecv(PMSG_GUILD_MATCHING_LIST_SEARCH_RECV* lpMsg,int aIndex);
	void CGGuildMatchingInsertRecv(PMSG_GUILD_MATCHING_INSERT_RECV* lpMsg,int aIndex);
	void CGGuildMatchingCancelRecv(PMSG_GUILD_MATCHING_CANCEL_RECV* lpMsg,int aIndex);
	void CGGuildMatchingJoinInsertRecv(PMSG_GUILD_MATCHING_JOIN_INSERT_RECV* lpMsg,int aIndex);
	void CGGuildMatchingJoinCancelRecv(PMSG_GUILD_MATCHING_JOIN_CANCEL_RECV* lpMsg,int aIndex);
	void CGGuildMatchingJoinAcceptRecv(PMSG_GUILD_MATCHING_JOIN_ACCEPT_RECV* lpMsg,int aIndex);
	void CGGuildMatchingJoinListRecv(PMSG_GUILD_MATCHING_JOIN_LIST_RECV* lpMsg,int aIndex);
	void CGGuildMatchingJoinInfoRecv(PMSG_GUILD_MATCHING_JOIN_INFO_RECV* lpMsg,int aIndex);
	void DGGuildMatchingNotifyRecv(SDHP_GUILD_MATCHING_NOTIFY_RECV* lpMsg);
	void DGGuildMatchingNotifyMasterRecv(SDHP_GUILD_MATCHING_NOTIFY_MASTER_RECV* lpMsg);
	void DGGuildMatchingListRecv(SDHP_GUILD_MATCHING_LIST_RECV* lpMsg);
	void DGGuildMatchingInsertRecv(SDHP_GUILD_MATCHING_INSERT_RECV* lpMsg);
	void DGGuildMatchingCancelRecv(SDHP_GUILD_MATCHING_CANCEL_RECV* lpMsg);
	void DGGuildMatchingJoinInsertRecv(SDHP_GUILD_MATCHING_JOIN_INSERT_RECV* lpMsg);
	void DGGuildMatchingJoinCancelRecv(SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV* lpMsg);
	void DGGuildMatchingJoinAcceptRecv(SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV* lpMsg);
	void DGGuildMatchingJoinListRecv(SDHP_GUILD_MATCHING_JOIN_LIST_RECV* lpMsg);
	void DGGuildMatchingJoinInfoRecv(SDHP_GUILD_MATCHING_JOIN_INFO_RECV* lpMsg);
	void GDGuildMatchingListSend(int aIndex,DWORD page);
	void GDGuildMatchingListSearchSend(int aIndex,DWORD page,char* SearchWord);
	void GDGuildMatchingInsertSend(int aIndex,char* text,BYTE InterestType,BYTE LevelRange,BYTE ClassType);
	void GDGuildMatchingCancelSend(int aIndex,BYTE flag);
	void GDGuildMatchingJoinInsertSend(int aIndex,char* GuildName);
	void GDGuildMatchingJoinCancelSend(int aIndex,BYTE flag);
	void GDGuildMatchingJoinAcceptSend(int aIndex,BYTE type,char* name);
	void GDGuildMatchingJoinListSend(int aIndex);
	void GDGuildMatchingJoinInfoSend(int aIndex);
	void GDGuildMatchingInsertSaveSend(int aIndex);
private:
};

extern CGuildMatching gGuildMatching;

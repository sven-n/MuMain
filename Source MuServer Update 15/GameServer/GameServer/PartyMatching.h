// PartyMatching.h: interface for the CPartyMatching class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_PARTY_MATCHING_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:EF:00
	char text[41];
	char password[5];
	WORD MinLevel;
	WORD MaxLevel;
	WORD HuntingGround;
	BYTE WantedClass;
	BYTE WantedClassDetailInfo[7];
	BYTE UsePassword;
	BYTE ApprovalType;
};

struct PMSG_PARTY_MATCHING_LIST_RECV
{
	PSBMSG_HEAD header; // C1:EF:01
	DWORD page;
	char SearchWord[11];
	BYTE UseSearchWord;
};

struct PMSG_PARTY_MATCHING_JOIN_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:EF:02
	char name[11];
	char password[5];
	BYTE UsePassword;
	BYTE UseRandomParty;
};

struct PMSG_PARTY_MATCHING_JOIN_INFO_RECV
{
	PSBMSG_HEAD header; // C1:EF:03
};

struct PMSG_PARTY_MATCHING_JOIN_LIST_RECV
{
	PSBMSG_HEAD header; // C1:EF:04
};

struct PMSG_PARTY_MATCHING_JOIN_ACCEPT_RECV
{
	PSBMSG_HEAD header; // C1:EF:05
	char name[11];
	BYTE type;
};

struct PMSG_PARTY_MATCHING_JOIN_CANCEL_RECV
{
	PSBMSG_HEAD header; // C1:EF:06
	BYTE type;
	DWORD result;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_PARTY_MATCHING_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:EF:00
	DWORD result;
};

struct PMSG_PARTY_MATCHING_LIST_SEND
{
	PSWMSG_HEAD header; // C2:EF:01
	DWORD count;
	DWORD CurPage;
	DWORD MaxPage;
	DWORD result;
};

struct PMSG_PARTY_MATCHING_LIST
{
	char name[11];
	char text[41];
	WORD MinLevel;
	WORD MaxLevel;
	WORD HuntingGround;
	DWORD LeaderLevel;
	BYTE LeaderClass;
	BYTE WantedClass;
	BYTE PartyMemberCount;
	BYTE UsePassword;
	BYTE WantedClassDetailInfo[7];
	DWORD ServerCode;
	BYTE GensType;
};

struct PMSG_PARTY_MATCHING_JOIN_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:EF:02
	DWORD result;
};

struct PMSG_PARTY_MATCHING_JOIN_INFO_SEND
{
	PSBMSG_HEAD header; // C1:EF:03
	DWORD result;
	DWORD ServerCode;
	char name[11];
};

struct PMSG_PARTY_MATCHING_JOIN_LIST_SEND
{
	PSWMSG_HEAD header; // C2:EF:04
	DWORD count;
	DWORD result;
};

struct PMSG_PARTY_MATCHING_JOIN_LIST
{
	char name[11];
	BYTE Class;
	DWORD Level;
};

struct PMSG_PARTY_MATCHING_JOIN_ACCEPT_SEND
{
	PSBMSG_HEAD header; // C1:EF:05
	char name[11];
	BYTE type;
	BYTE flag;
	DWORD result;
};

struct PMSG_PARTY_MATCHING_JOIN_CANCEL_SEND
{
	PSBMSG_HEAD header; // C1:EF:06
	BYTE type;
	DWORD result;
};

struct PMSG_PARTY_MATCHING_JOIN_NOTIFY_SEND
{
	PSBMSG_HEAD header; // C1:EF:08
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_PARTY_MATCHING_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:29:00
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
};

struct SDHP_PARTY_MATCHING_LIST_RECV
{
	PSWMSG_HEAD header; // C2:29:01
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	DWORD CurPage;
	DWORD MaxPage;
	DWORD count;
};

struct SDHP_PARTY_MATCHING_LIST
{
	char name[11];
	char text[41];
	WORD MinLevel;
	WORD MaxLevel;
	WORD HuntingGround;
	DWORD LeaderLevel;
	BYTE LeaderClass;
	BYTE WantedClass;
	BYTE PartyMemberCount;
	BYTE UsePassword;
	BYTE WantedClassDetailInfo[7];
	DWORD ServerCode;
	BYTE GensType;
};

struct SDHP_PARTY_MATCHING_JOIN_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:29:02
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
};

struct SDHP_PARTY_MATCHING_JOIN_INFO_RECV
{
	PSBMSG_HEAD header; // C1:29:03
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	DWORD LeaderServerCode;
	char LeaderName[11];
};

struct SDHP_PARTY_MATCHING_JOIN_LIST_RECV
{
	PSWMSG_HEAD header; // C2:29:04
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	DWORD count;
};

struct SDHP_PARTY_MATCHING_JOIN_LIST
{
	char name[11];
	BYTE Class;
	DWORD Level;
};

struct SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV
{
	PSBMSG_HEAD header; // C1:29:05
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	char MemberName[11];
	BYTE type;
	BYTE flag;
};

struct SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV
{
	PSBMSG_HEAD header; // C1:29:06
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	BYTE type;
	BYTE flag;
};

struct SDHP_PARTY_MATCHING_NOTIFY_RECV
{
	PSBMSG_HEAD header; // C1:29:07
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	char LeaderName[11];
};

struct SDHP_PARTY_MATCHING_NOTIFY_LEADER_RECV
{
	PSBMSG_HEAD header; // C1:29:08
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_PARTY_MATCHING_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:29:00
	WORD index;
	char account[11];
	char name[11];
	char text[41];
	char password[5];
	WORD MinLevel;
	WORD MaxLevel;
	WORD HuntingGround;
	WORD LeaderLevel;
	BYTE LeaderClass;
	BYTE WantedClass;
	BYTE WantedClassDetailInfo[7];
	BYTE PartyMemberCount;
	BYTE ApprovalType;
	BYTE UsePassword;
	BYTE GensType;
};

struct SDHP_PARTY_MATCHING_LIST_SEND
{
	PSBMSG_HEAD header; // C1:29:01
	WORD index;
	char account[11];
	char name[11];
	DWORD page;
	WORD Level;
	WORD Class;
	BYTE GensType;
	BYTE UseSearchWord;
	char SearchWord[11];
};

struct SDHP_PARTY_MATCHING_JOIN_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:29:02
	WORD index;
	char account[11];
	char name[11];
	char LeaderName[11];
	char password[5];
	WORD Level;
	BYTE Class;
	BYTE UsePassword;
	BYTE UseRandomParty;
	BYTE GensType;
};

struct SDHP_PARTY_MATCHING_JOIN_INFO_SEND
{
	PSBMSG_HEAD header; // C1:29:03
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_PARTY_MATCHING_JOIN_LIST_SEND
{
	PSBMSG_HEAD header; // C1:29:04
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_PARTY_MATCHING_JOIN_ACCEPT_SEND
{
	PSBMSG_HEAD header; // C1:29:05
	WORD index;
	char account[11];
	char name[11];
	char MemberName[11];
	BYTE type;
};

struct SDHP_PARTY_MATCHING_JOIN_CANCEL_SEND
{
	PSBMSG_HEAD header; // C1:29:06
	WORD index;
	char account[11];
	char name[11];
	BYTE type;
	BYTE flag;
};

struct SDHP_PARTY_MATCHING_INSERT_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:29:30
	WORD index;
	char account[11];
	char name[11];
	WORD LeaderLevel;
	BYTE LeaderClass;
	BYTE PartyMemberCount;
	BYTE GensType;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CPartyMatching
{
public:
	CPartyMatching();
	virtual ~CPartyMatching();
	void CGPartyMatchingInsertRecv(PMSG_PARTY_MATCHING_INSERT_RECV* lpMsg,int aIndex);
	void CGPartyMatchingListRecv(PMSG_PARTY_MATCHING_LIST_RECV* lpMsg,int aIndex);
	void CGPartyMatchingJoinInsertRecv(PMSG_PARTY_MATCHING_JOIN_INSERT_RECV* lpMsg,int aIndex);
	void CGPartyMatchingJoinInfoRecv(PMSG_PARTY_MATCHING_JOIN_INFO_RECV* lpMsg,int aIndex);
	void CGPartyMatchingJoinListRecv(PMSG_PARTY_MATCHING_JOIN_LIST_RECV* lpMsg,int aIndex);
	void CGPartyMatchingJoinAcceptRecv(PMSG_PARTY_MATCHING_JOIN_ACCEPT_RECV* lpMsg,int aIndex);
	void CGPartyMatchingJoinCancelRecv(PMSG_PARTY_MATCHING_JOIN_CANCEL_RECV* lpMsg,int aIndex);
	void DGPartyMatchingInsertRecv(SDHP_PARTY_MATCHING_INSERT_RECV* lpMsg);
	void DGPartyMatchingListRecv(SDHP_PARTY_MATCHING_LIST_RECV* lpMsg);
	void DGPartyMatchingJoinInsertRecv(SDHP_PARTY_MATCHING_JOIN_INSERT_RECV* lpMsg);
	void DGPartyMatchingJoinInfoRecv(SDHP_PARTY_MATCHING_JOIN_INFO_RECV* lpMsg);
	void DGPartyMatchingJoinListRecv(SDHP_PARTY_MATCHING_JOIN_LIST_RECV* lpMsg);
	void DGPartyMatchingJoinAcceptRecv(SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV* lpMsg);
	void DGPartyMatchingJoinCancelRecv(SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV* lpMsg);
	void DGPartyMatchingNotifyRecv(SDHP_PARTY_MATCHING_NOTIFY_RECV* lpMsg);
	void DGPartyMatchingNotifyLeaderRecv(SDHP_PARTY_MATCHING_NOTIFY_LEADER_RECV* lpMsg);
	void GDPartyMatchingInsertSend(int aIndex,char* text,char* password,WORD MinLevel,WORD MaxLevel,WORD HuntingGround,BYTE WantedClass,BYTE WantedClassDetailInfo[7],BYTE UsePassword,BYTE ApprovalType);
	void GDPartyMatchingListSend(int aIndex,DWORD page,char* SearchWord,BYTE UseSearchWord);
	void GDPartyMatchingJoinInsertSend(int aIndex,char* LeaderName,char* password,BYTE UsePassword,BYTE UseRandomParty);
	void GDPartyMatchingJoinInfoSend(int aIndex);
	void GDPartyMatchingJoinListSend(int aIndex);
	void GDPartyMatchingJoinAcceptSend(int aIndex,char* MemberName,BYTE type);
	void GDPartyMatchingJoinCancelSend(int aIndex,BYTE type,BYTE flag);
	void GDPartyMatchingInsertSaveSend(int aIndex);
private:
};

extern CPartyMatching gPartyMatching;

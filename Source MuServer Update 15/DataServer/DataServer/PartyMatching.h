// PartyMatching.h: interface for the CPartyMatching class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CriticalSection.h"
#include "DataServerProtocol.h"

#define MAX_PARTY_USER 5

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_PARTY_MATCHING_INSERT_RECV
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

struct SDHP_PARTY_MATCHING_LIST_RECV
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

struct SDHP_PARTY_MATCHING_JOIN_INSERT_RECV
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

struct SDHP_PARTY_MATCHING_JOIN_INFO_RECV
{
	PSBMSG_HEAD header; // C1:29:03
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_PARTY_MATCHING_JOIN_LIST_RECV
{
	PSBMSG_HEAD header; // C1:29:04
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV
{
	PSBMSG_HEAD header; // C1:29:05
	WORD index;
	char account[11];
	char name[11];
	char MemberName[11];
	BYTE type;
};

struct SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV
{
	PSBMSG_HEAD header; // C1:29:06
	WORD index;
	char account[11];
	char name[11];
	BYTE type;
	BYTE flag;
};

struct SDHP_PARTY_MATCHING_INSERT_SAVE_RECV
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
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_PARTY_MATCHING_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:29:00
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
};

struct SDHP_PARTY_MATCHING_LIST_SEND
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

struct SDHP_PARTY_MATCHING_JOIN_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:29:02
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
};

struct SDHP_PARTY_MATCHING_JOIN_INFO_SEND
{
	PSBMSG_HEAD header; // C1:29:03
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	DWORD LeaderServerCode;
	char LeaderName[11];
};

struct SDHP_PARTY_MATCHING_JOIN_LIST_SEND
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

struct SDHP_PARTY_MATCHING_JOIN_ACCEPT_SEND
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

struct SDHP_PARTY_MATCHING_JOIN_CANCEL_SEND
{
	PSBMSG_HEAD header; // C1:29:06
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	BYTE type;
	BYTE flag;
};

struct SDHP_PARTY_MATCHING_NOTIFY_SEND
{
	PSBMSG_HEAD header; // C1:29:07
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
	char LeaderName[11];
};

struct SDHP_PARTY_MATCHING_NOTIFY_LEADER_SEND
{
	PSBMSG_HEAD header; // C1:29:08
	WORD index;
	char account[11];
	char name[11];
	DWORD result;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct PARTY_MATCHING_INFO
{
	char Name[11];
	char Text[41];
	char Password[5];
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
	WORD ServerCode;
};

struct PARTY_MATCHING_JOIN_INFO
{
	char Name[11];
	char LeaderName[11];
	WORD LeaderServerCode;
	BYTE Class;
	DWORD Level;
};

class CPartyMatching
{
public:
	CPartyMatching();
	virtual ~CPartyMatching();
	void ClearPartyMatchingInfo(WORD ServerCode);
	void ClearPartyMatchingJoinInfo(WORD ServerCode);
	bool GetPartyMatchingInfo(PARTY_MATCHING_INFO* lpPartyMatchingInfo,char* name);
	bool GetPartyMatchingInfo(PARTY_MATCHING_INFO* lpPartyMatchingInfo,WORD ServerCode,WORD Level,BYTE Class,BYTE GensType);
	void InsertPartyMatchingInfo(PARTY_MATCHING_INFO PartyMatchingInfo);
	void RemovePartyMatchingInfo(PARTY_MATCHING_INFO PartyMatchingInfo);
	bool GetPartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO* lpPartyMatchingJoinInfo,char* name);
	void InsertPartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo);
	void RemovePartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo);
	void RemovePartyMatchingJoinInfoNotifyAll(PARTY_MATCHING_INFO PartyMatchingInfo);
	DWORD GeneratePartyMatchingList(DWORD* CurPage,DWORD* MaxPage,BYTE UseSearchWord,char* SearchWord,BYTE* lpMsg,int* size);
	DWORD GeneratePartyMatchingJoinList(char* LeaderName,BYTE* lpMsg,int* size);
	void GDPartyMatchingInsertRecv(SDHP_PARTY_MATCHING_INSERT_RECV* lpMsg,int index);
	void GDPartyMatchingListRecv(SDHP_PARTY_MATCHING_LIST_RECV* lpMsg,int index);
	void GDPartyMatchingJoinInsertRecv(SDHP_PARTY_MATCHING_JOIN_INSERT_RECV* lpMsg,int index);
	void GDPartyMatchingJoinInfoRecv(SDHP_PARTY_MATCHING_JOIN_INFO_RECV* lpMsg,int index);
	void GDPartyMatchingJoinListRecv(SDHP_PARTY_MATCHING_JOIN_LIST_RECV* lpMsg,int index);
	void GDPartyMatchingJoinAcceptRecv(SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV* lpMsg,int index);
	void GDPartyMatchingJoinCancelRecv(SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV* lpMsg,int index);
	void GDPartyMatchingInsertSaveRecv(SDHP_PARTY_MATCHING_INSERT_SAVE_RECV* lpMsg);
	void DGPartyMatchingNotifySend(char* name,char* LeaderName,DWORD result);
	void DGPartyMatchingNotifyLeaderSend(char* name,DWORD result);
private:
	CCriticalSection m_critical;
	std::map<std::string,PARTY_MATCHING_INFO> m_PartyMatchingInfo;
	std::map<std::string,PARTY_MATCHING_JOIN_INFO> m_PartyMatchingJoinInfo;
};

extern CPartyMatching gPartyMatching;

// Party.h: interface for the CParty class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

#define MAX_PARTY_USER 5
#define MAX_PARTY_DISTANCE 10

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_PARTY_REQUEST_RECV
{
	PBMSG_HEAD header; // C1:40
	BYTE index[2];
};

struct PMSG_PARTY_REQUEST_RESULT_RECV
{
	PBMSG_HEAD header; // C1:41
	BYTE result;
	BYTE index[2];
};

struct PMSG_PARTY_DEL_MEMBER_RECV
{
	PBMSG_HEAD header; // C1:43
	BYTE number;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_PARTY_REQUEST_SEND
{
	PBMSG_HEAD header; // C1:40
	BYTE index[2];
};

struct PMSG_PARTY_RESULT_SEND
{
	PBMSG_HEAD header; // C1:41
	BYTE result;
};

struct PMSG_PARTY_LIST_SEND
{
	PBMSG_HEAD header; // C1:42
	BYTE result;
	BYTE count;
};

struct PMSG_PARTY_LIST
{
	char name[10];
	BYTE number;
	BYTE map;
	BYTE x;
	BYTE y;
	DWORD CurLife;
	DWORD MaxLife;
	#if(GAMESERVER_UPDATE>=802)
	DWORD ServerCode;
	DWORD CurMana;
	DWORD MaxMana;
	#endif
};

struct PMSG_PARTY_LIFE_SEND
{
	PBMSG_HEAD header; // C1:44
	BYTE count;
};

struct PMSG_PARTY_LIFE
{
	#if(GAMESERVER_UPDATE>=802)
	BYTE life;
	BYTE mana;
	char name[11];
	#else
	BYTE number;
	#endif
};

//**********************************************//
//**********************************************//
//**********************************************//

struct PARTY_INFO
{
	int Count;
	int Index[MAX_PARTY_USER];
};

class CParty
{
public:
	CParty();
	virtual ~CParty();
	bool IsParty(int index);
	bool IsLeader(int index,int aIndex);
	bool IsMember(int index,int aIndex);
	int GetMemberCount(int index);
	int GetMemberIndex(int index,int number);
	int GetMemberNumber(int index,int aIndex);
	bool Create(int aIndex);
	bool Destroy(int index);
	bool AddMember(int index,int aIndex);
	bool DelMember(int index,int aIndex);
	void ChangeLeader(int index,int number);
	bool AutoAcceptPartyRequest(LPOBJ lpObj,LPOBJ lpTarget);
	void CGPartyRequestRecv(PMSG_PARTY_REQUEST_RECV* lpMsg,int aIndex);
	void CGPartyRequestResultRecv(PMSG_PARTY_REQUEST_RESULT_RECV* lpMsg,int aIndex);
	void CGPartyListRecv(int aIndex);
	void CGPartyDelMemberRecv(PMSG_PARTY_DEL_MEMBER_RECV* lpMsg,int aIndex);
	void GCPartyResultSend(int aIndex,BYTE result);
	void GCPartyListSend(int index);
	void GCPartyDelMemberSend(int aIndex);
	void GCPartyLifeSend(int index);
public:
	int m_PartyCount;
	PARTY_INFO m_PartyInfo[MAX_OBJECT];
};

extern CParty gParty;

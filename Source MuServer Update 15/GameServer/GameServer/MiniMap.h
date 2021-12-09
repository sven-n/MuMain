// MiniMap.h: interface for the CMiniMap class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_MINI_MAP_INFO_RECV
{
	PSBMSG_HEAD header; // C1:E7:03
	BYTE flag;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_MINI_MAP_PARTY_INFO_SEND
{
	PSBMSG_HEAD header; // C1:E7:01
	BYTE count;
};

struct PMSG_MINI_MAP_PARTY_INFO
{
	char name[11];
	BYTE map;
	BYTE x;
	BYTE y;
};

struct PMSG_MINI_MAP_INFO_SEND
{
	PSBMSG_HEAD header; // C1:E7:03
	BYTE index;
	BYTE group;
	BYTE type;
	BYTE flag;
	BYTE x;
	BYTE y;
	char text[31];
};

//**********************************************//
//**********************************************//
//**********************************************//

struct MINI_MAP_INFO
{
	int Index;
	int Group;
	int Type;
	int X;
	int Y;
	char Text[32];
};

class CMiniMap
{
public:
	CMiniMap();
	virtual ~CMiniMap();
	void Load(char* path);
	void CGMiniMapStartPartyInfoRecv(int aIndex);
	void CGMiniMapClosePartyInfoRecv(int aIndex);
	void CGMiniMapInfoRecv(PMSG_MINI_MAP_INFO_RECV* lpMsg,int aIndex);
	void GCMiniMapPartyInfoSend(int aIndex);
	void GCMiniMapInfoSend(int aIndex);
private:
	std::vector<MINI_MAP_INFO> m_MiniMapInfo;
};

extern CMiniMap gMiniMap;

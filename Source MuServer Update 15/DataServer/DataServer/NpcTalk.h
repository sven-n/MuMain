// NpcTalk.h: interface for the CNpcTalk class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_NPC_LEO_THE_HELPER_RECV
{
	PSBMSG_HEAD header; // C1:0E:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_NPC_SANTA_CLAUS_RECV
{
	PSBMSG_HEAD header; // C1:0E:01
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_NPC_LEO_THE_HELPER_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:0E:30
	WORD index;
	char account[11];
	char name[11];
	BYTE status;
};

struct SDHP_NPC_SANTA_CLAUS_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:0E:31
	WORD index;
	char account[11];
	char name[11];
	BYTE status;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_NPC_LEO_THE_HELPER_SEND
{
	PSBMSG_HEAD header; // C1:0E:00
	WORD index;
	char account[11];
	char name[11];
	BYTE status;
};

struct SDHP_NPC_SANTA_CLAUS_SEND
{
	PSBMSG_HEAD header; // C1:0E:01
	WORD index;
	char account[11];
	char name[11];
	BYTE status;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CNpcTalk
{
public:
	CNpcTalk();
	virtual ~CNpcTalk();
	void GDNpcLeoTheHelperRecv(SDHP_NPC_LEO_THE_HELPER_RECV* lpMsg,int index);
	void GDNpcSantaClausRecv(SDHP_NPC_SANTA_CLAUS_RECV* lpMsg,int index);
	void GDNpcLeoTheHelperSaveRecv(SDHP_NPC_LEO_THE_HELPER_SAVE_RECV* lpMsg);
	void GDNpcSantaClausSaveRecv(SDHP_NPC_SANTA_CLAUS_SAVE_RECV* lpMsg);
};

extern CNpcTalk gNpcTalk;

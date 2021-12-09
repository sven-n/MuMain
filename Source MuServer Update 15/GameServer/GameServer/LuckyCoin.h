// LuckyCoin.h: interface for the CLuckyCoin class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_LUCKY_COIN_COUNT_RECV
{
	PSBMSG_HEAD header; // C1:BF:0B
};

struct PMSG_LUCKY_COIN_REGISTER_RECV
{
	PSBMSG_HEAD header; // C1:BF:0C
};

struct PMSG_LUCKY_COIN_EXCHANGE_RECV
{
	PSBMSG_HEAD header; // C1:BF:0D
	DWORD TradeCoin;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_LUCKY_COIN_COUNT_SEND
{
	PSBMSG_HEAD header; // C1:BF:0B
	DWORD LuckyCoin;
};

struct PMSG_LUCKY_COIN_REGISTER_SEND
{
	PSBMSG_HEAD header; // C1:BF:0C
	BYTE result;
	DWORD LuckyCoin;
};

struct PMSG_LUCKY_COIN_EXCHANGE_SEND
{
	PSBMSG_HEAD header; // C1:BF:0D
	BYTE result;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_LUCKY_COIN_COUNT_RECV
{
	PSBMSG_HEAD header; // C1:1A:00
	WORD index;
	char account[11];
	BYTE result;
	DWORD LuckyCoin;
};

struct SDHP_LUCKY_COIN_REGISTER_RECV
{
	PSBMSG_HEAD header; // C1:1A:01
	WORD index;
	char account[11];
	BYTE result;
	BYTE slot;
	DWORD LuckyCoin;
};

struct SDHP_LUCKY_COIN_EXCHANGE_RECV
{
	PSBMSG_HEAD header; // C1:1A:02
	WORD index;
	char account[11];
	BYTE result;
	DWORD TradeCoin;
	DWORD LuckyCoin;
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_LUCKY_COIN_COUNT_SEND
{
	PSBMSG_HEAD header; // C1:1A:00
	WORD index;
	char account[11];
};

struct SDHP_LUCKY_COIN_REGISTER_SEND
{
	PSBMSG_HEAD header; // C1:1A:01
	WORD index;
	char account[11];
	BYTE slot;
};

struct SDHP_LUCKY_COIN_EXCHANGE_SEND
{
	PSBMSG_HEAD header; // C1:1A:02
	WORD index;
	char account[11];
	DWORD TradeCoin;
};

struct SDHP_LUCKY_COIN_ADD_COUNT_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:1A:30
	WORD index;
	char account[11];
	DWORD AddLuckyCoin;
};

struct SDHP_LUCKY_COIN_SUB_COUNT_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:1A:31
	WORD index;
	char account[11];
	DWORD SubLuckyCoin;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CLuckyCoin
{
public:
	CLuckyCoin();
	virtual ~CLuckyCoin();
	void CGLuckyCoinCountRecv(PMSG_LUCKY_COIN_COUNT_RECV* lpMsg,int aIndex);
	void CGLuckyCoinRegisterRecv(PMSG_LUCKY_COIN_REGISTER_RECV* lpMsg,int aIndex);
	void CGLuckyCoinExchangeRecv(PMSG_LUCKY_COIN_EXCHANGE_RECV* lpMsg,int aIndex);
	void DGLuckyCoinCountRecv(SDHP_LUCKY_COIN_COUNT_RECV* lpMsg);
	void DGLuckyCoinRegisterRecv(SDHP_LUCKY_COIN_REGISTER_RECV* lpMsg);
	void DGLuckyCoinExchangeRecv(SDHP_LUCKY_COIN_EXCHANGE_RECV* lpMsg);
	void GDLuckyCoinAddCountSaveSend(int aIndex,DWORD AddLuckyCoin);
	void GDLuckyCoinSubCountSaveSend(int aIndex,DWORD SubLuckyCoin);
};

extern CLuckyCoin gLuckyCoin;

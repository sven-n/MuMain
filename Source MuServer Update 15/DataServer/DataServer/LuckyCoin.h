// LuckyCoin.h: interface for the CLuckyCoin class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_LUCKY_COIN_COUNT_RECV
{
	PSBMSG_HEAD header; // C1:1A:00
	WORD index;
	char account[11];
};

struct SDHP_LUCKY_COIN_REGISTER_RECV
{
	PSBMSG_HEAD header; // C1:1A:01
	WORD index;
	char account[11];
	BYTE slot;
};

struct SDHP_LUCKY_COIN_EXCHANGE_RECV
{
	PSBMSG_HEAD header; // C1:1A:02
	WORD index;
	char account[11];
	DWORD TradeCoin;
};

struct SDHP_LUCKY_COIN_ADD_COUNT_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:1A:30
	WORD index;
	char account[11];
	DWORD AddLuckyCoin;
};

struct SDHP_LUCKY_COIN_SUB_COUNT_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:1A:31
	WORD index;
	char account[11];
	DWORD SubLuckyCoin;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_LUCKY_COIN_COUNT_SEND
{
	PSBMSG_HEAD header; // C1:1A:00
	WORD index;
	char account[11];
	BYTE result;
	DWORD LuckyCoin;
};

struct SDHP_LUCKY_COIN_REGISTER_SEND
{
	PSBMSG_HEAD header; // C1:1A:01
	WORD index;
	char account[11];
	BYTE result;
	BYTE slot;
	DWORD LuckyCoin;
};

struct SDHP_LUCKY_COIN_EXCHANGE_SEND
{
	PSBMSG_HEAD header; // C1:1A:02
	WORD index;
	char account[11];
	BYTE result;
	DWORD TradeCoin;
	DWORD LuckyCoin;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CLuckyCoin
{
public:
	CLuckyCoin();
	virtual ~CLuckyCoin();
	void GDLuckyCoinCountRecv(SDHP_LUCKY_COIN_COUNT_RECV* lpMsg,int index);
	void GDLuckyCoinRegisterRecv(SDHP_LUCKY_COIN_REGISTER_RECV* lpMsg,int index);
	void GDLuckyCoinExchangeRecv(SDHP_LUCKY_COIN_EXCHANGE_RECV* lpMsg,int index);
	void GDLuckyCoinAddCountSaveRecv(SDHP_LUCKY_COIN_ADD_COUNT_SAVE_RECV* lpMsg);
	void GDLuckyCoinSubCountSaveRecv(SDHP_LUCKY_COIN_SUB_COUNT_SAVE_RECV* lpMsg);
};

extern CLuckyCoin gLuckyCoin;

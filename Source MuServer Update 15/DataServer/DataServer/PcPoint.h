// PcPoint.h: interface for the CPcPoint class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_PC_POINT_POINT_RECV
{
	PSBMSG_HEAD header; // C1:19:00
	WORD index;
	char account[11];
};

struct SDHP_PC_POINT_ITEM_BUY_RECV
{
	PSBMSG_HEAD header; // C1:19:01
	WORD index;
	char account[11];
	BYTE slot;
};

struct SDHP_PC_POINT_RECIEVE_POINT_RECV
{
	PSBMSG_HEAD header; // C1:19:02
	WORD index;
	char account[11];
	DWORD CallbackFunc;
	DWORD CallbackArg1;
	DWORD CallbackArg2;
};

struct SDHP_PC_POINT_ADD_POINT_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:19:30
	WORD index;
	char account[11];
	DWORD AddPcPoint;
	DWORD MaxPcPoint;
};

struct SDHP_PC_POINT_SUB_POINT_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:19:31
	WORD index;
	char account[11];
	DWORD SubPcPoint;
	DWORD MaxPcPoint;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_PC_POINT_POINT_SEND
{
	PSBMSG_HEAD header; // C1:19:00
	WORD index;
	char account[11];
	BYTE result;
	DWORD PcPoint;
};

struct SDHP_PC_POINT_ITEM_BUY_SEND
{
	PSBMSG_HEAD header; // C1:19:01
	WORD index;
	char account[11];
	BYTE result;
	BYTE slot;
	DWORD PcPoint;
};

struct SDHP_PC_POINT_RECIEVE_POINT_SEND
{
	PSBMSG_HEAD header; // C1:19:02
	WORD index;
	char account[11];
	DWORD CallbackFunc;
	DWORD CallbackArg1;
	DWORD CallbackArg2;
	DWORD PcPoint;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CPcPoint
{
public:
	CPcPoint();
	virtual ~CPcPoint();
	void GDPcPointPointRecv(SDHP_PC_POINT_POINT_RECV* lpMsg,int index);
	void GDPcPointItemBuyRecv(SDHP_PC_POINT_ITEM_BUY_RECV* lpMsg,int index);
	void GDPcPointRecievePointRecv(SDHP_PC_POINT_RECIEVE_POINT_RECV* lpMsg,int index);
	void GDPcPointAddPointSaveRecv(SDHP_PC_POINT_ADD_POINT_SAVE_RECV* lpMsg);
	void GDPcPointSubPointSaveRecv(SDHP_PC_POINT_SUB_POINT_SAVE_RECV* lpMsg);
};

extern CPcPoint gPcPoint;

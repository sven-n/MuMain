// PcPoint.h: interface for the CPcPoint class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "Shop.h"

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_PC_POINT_ITEM_BUY_RECV
{
	PSBMSG_HEAD header; // C1:D0:05
	BYTE slot;
};

struct PMSG_PC_POINT_OPEN_RECV
{
	PSBMSG_HEAD header; // C1:D0:06
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_PC_POINT_POINT_SEND
{
	PSBMSG_HEAD header; // C1:D0:04
	WORD MinPcPoint;
	WORD MaxPcPoint;
	BYTE PcPointType;
};

struct PMSG_PC_POINT_ITEM_BUY_SEND
{
	PSBMSG_HEAD header; // C1:D0:05
	BYTE result;
	BYTE ItemSlot;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_PC_POINT_OPEN_SEND
{
	PSBMSG_HEAD header; // C1:D0:06
	BYTE result;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_PC_POINT_POINT_RECV
{
	PSBMSG_HEAD header; // C1:19:00
	WORD index;
	char account[11];
	BYTE result;
	DWORD PcPoint;
};

struct SDHP_PC_POINT_ITEM_BUY_RECV
{
	PSBMSG_HEAD header; // C1:19:01
	WORD index;
	char account[11];
	BYTE result;
	BYTE slot;
	DWORD PcPoint;
};

struct SDHP_PC_POINT_RECIEVE_POINT_RECV
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
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_PC_POINT_POINT_SEND
{
	PSBMSG_HEAD header; // C1:19:00
	WORD index;
	char account[11];
};

struct SDHP_PC_POINT_ITEM_BUY_SEND
{
	PSBMSG_HEAD header; // C1:19:01
	WORD index;
	char account[11];
	BYTE slot;
};

struct SDHP_PC_POINT_RECIEVE_POINT_SEND
{
	PSBMSG_HEAD header; // C1:19:02
	WORD index;
	char account[11];
	DWORD CallbackFunc;
	DWORD CallbackArg1;
	DWORD CallbackArg2;
};

struct SDHP_PC_POINT_ADD_POINT_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:19:30
	WORD index;
	char account[11];
	DWORD AddPcPoint;
	DWORD MaxPcPoint;
};

struct SDHP_PC_POINT_SUB_POINT_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:19:31
	WORD index;
	char account[11];
	DWORD SubPcPoint;
	DWORD MaxPcPoint;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CPcPoint : public CShop
{
public:
	CPcPoint();
	virtual ~CPcPoint();
	void Load(char* path);
	void MainProc();
	void CGPcPointItemBuyRecv(PMSG_PC_POINT_ITEM_BUY_RECV* lpMsg,int aIndex);
	void CGPcPointOpenRecv(PMSG_PC_POINT_OPEN_RECV* lpMsg,int aIndex);
	void DGPcPointPointRecv(SDHP_PC_POINT_POINT_RECV* lpMsg);
	void DGPcPointItemBuyRecv(SDHP_PC_POINT_ITEM_BUY_RECV* lpMsg);
	void DGPcPointRecievePointRecv(SDHP_PC_POINT_RECIEVE_POINT_RECV* lpMsg);
	void GDPcPointPointSend(int aIndex);
	void GDPcPointRecievePointSend(int aIndex,DWORD CallbackFunc,DWORD CallbackArg1,DWORD CallbackArg2);
	void GDPcPointAddPointSaveSend(int aIndex,DWORD AddPcPoint);
	void GDPcPointSubPointSaveSend(int aIndex,DWORD SubPcPoint);
};

extern CPcPoint gPcPoint;

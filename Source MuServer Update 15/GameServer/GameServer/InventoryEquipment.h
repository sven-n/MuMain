// InventoryEquipment.h: interface for the CInventoryEquipment class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_INVENTORY_EQUIPMENT_RECV
{
	PSBMSG_HEAD header; // C1:BF:20
	BYTE slot;
	BYTE result;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_INVENTORY_EQUIPMENT_SEND
{
	PSBMSG_HEAD header; // C1:BF:20
	BYTE slot;
	BYTE result;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CInventoryEquipment
{
public:
	CInventoryEquipment();
	virtual ~CInventoryEquipment();
	void MainProc();
	bool CheckInventoryEquipment(LPOBJ lpObj,int ItemIndex);
	void InsertInventoryEquipment(LPOBJ lpObj);
	void CGInventoryEquipmentRecv(PMSG_INVENTORY_EQUIPMENT_RECV* lpMsg,int aIndex);
};

extern CInventoryEquipment gInventoryEquipment;

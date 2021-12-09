// InventoryEquipment.cpp: implementation of the CInventoryEquipment class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InventoryEquipment.h"
#include "EffectManager.h"
#include "ItemManager.h"
#include "Util.h"

CInventoryEquipment gInventoryEquipment;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInventoryEquipment::CInventoryEquipment() // OK
{

}

CInventoryEquipment::~CInventoryEquipment() // OK
{

}

void CInventoryEquipment::MainProc() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[n];

		for(int i=0;i < MAX_EFFECT_LIST;i++)
		{
			if(lpObj->Effect[i].IsEffect() != 0)
			{
				if(lpObj->Effect[i].m_index == EFFECT_HAWK_FIGURINE || lpObj->Effect[i].m_index == EFFECT_GOAT_FIGURINE || lpObj->Effect[i].m_index == EFFECT_OAK_CHARM || lpObj->Effect[i].m_index == EFFECT_MAPLE_CHARM || lpObj->Effect[i].m_index == EFFECT_GOLDEN_OAK_CHARM || lpObj->Effect[i].m_index == EFFECT_GOLDEN_MAPLE_CHARM || lpObj->Effect[i].m_index == EFFECT_WORN_HORSESHOE)
				{
					EFFECT_INFO* lpInfo = gEffectManager.GetInfo(lpObj->Effect[i].m_index);

					if(lpInfo != 0)
					{
						if(this->CheckInventoryEquipment(lpObj,lpInfo->ItemIndex) == 0)
						{
							gEffectManager.DelEffect(lpObj,lpInfo->Index);
						}
					}
				}
			}
		}
	}

	#endif
}

bool CInventoryEquipment::CheckInventoryEquipment(LPOBJ lpObj,int ItemIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=INVENTORY_WEAR_SIZE;n < INVENTORY_EXT4_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0)
		{
			if(lpObj->Inventory[n].m_Index == ItemIndex)
			{
				if(lpObj->Inventory[n].m_Durability == 254)
				{
					return 1;
				}
			}
		}
	}

	return 0;

	#else

	return 0;

	#endif
}

void CInventoryEquipment::InsertInventoryEquipment(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=INVENTORY_WEAR_SIZE;n < INVENTORY_EXT4_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0)
		{
			if(lpObj->Inventory[n].m_Index >= GET_ITEM(13,128) && lpObj->Inventory[n].m_Index <= GET_ITEM(13,134))
			{
				if(lpObj->Inventory[n].m_Durability == 254)
				{
					EFFECT_INFO* lpInfo = gEffectManager.GetInfoByItem(lpObj->Inventory[n].m_Index);

					if(lpInfo != 0)
					{
						if(gEffectManager.CheckEffectByGroup(lpObj,lpInfo->Group) == 0)
						{
							gEffectManager.AddEffect(lpObj,1,lpInfo->Index,0,0,0,0,0);
						}
					}
				}
			}
		}
	}

	#endif
}

void CInventoryEquipment::CGInventoryEquipmentRecv(PMSG_INVENTORY_EQUIPMENT_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_INVENTORY_EQUIPMENT_SEND pMsg;

	pMsg.header.set(0xBF,0x20,sizeof(pMsg));

	pMsg.slot = lpMsg->slot;

	pMsg.result = -1;

	if(INVENTORY_FULL_RANGE(lpMsg->slot) == 0 || INVENTORY_WEAR_RANGE(lpMsg->slot) != 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if((lpObj->Inventory[lpMsg->slot].m_Index < GET_ITEM(13,128) || lpObj->Inventory[lpMsg->slot].m_Index > GET_ITEM(13,134)) || lpObj->Inventory[lpMsg->slot].m_Durability == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpMsg->result == 254)
	{
		if(lpObj->Inventory[lpMsg->slot].m_Durability == 255)
		{
			EFFECT_INFO* lpInfo = gEffectManager.GetInfoByItem(lpObj->Inventory[lpMsg->slot].m_Index);

			if(lpInfo != 0)
			{
				lpObj->Inventory[lpMsg->slot].m_Durability = 254;
				gEffectManager.AddEffect(lpObj,1,lpInfo->Index,0,0,0,0,0);
				pMsg.result = -2;
			}
		}
	}

	if(lpMsg->result == 255)
	{
		if(lpObj->Inventory[lpMsg->slot].m_Durability == 254)
		{
			EFFECT_INFO* lpInfo = gEffectManager.GetInfoByItem(lpObj->Inventory[lpMsg->slot].m_Index);

			if(lpInfo != 0)
			{
				lpObj->Inventory[lpMsg->slot].m_Durability = 255;
				gEffectManager.DelEffect(lpObj,lpInfo->Index);
				pMsg.result = -1;
			}
		}
	}

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

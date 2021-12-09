// JewelMix.cpp: implementation of the CJewelMix class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JewelMix.h"
#include "CommandManager.h"
#include "DSProtocol.h"
#include "ItemManager.h"
#include "Message.h"
#include "Notice.h"
#include "Util.h"

CJewelMix gJewelMix;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJewelMix::CJewelMix() // OK
{

}

CJewelMix::~CJewelMix() // OK
{

}

int CJewelMix::GetJewelSimpleIndex(int type) // OK
{
	switch(type)
	{
		case 0:
			return GET_ITEM(14,13);
		case 1:
			return GET_ITEM(14,14);
		case 2:
			return GET_ITEM(14,16);
		case 3:
			return GET_ITEM(14,22);
		case 4:
			return GET_ITEM(14,31);
		case 5:
			return GET_ITEM(14,41);
		case 6:
			return GET_ITEM(14,42);
		case 7:
			return GET_ITEM(12,15);
		case 8:
			return GET_ITEM(14,43);
		case 9:
			return GET_ITEM(14,44);
	}

	return -1;
}

int CJewelMix::GetJewelBundleIndex(int type) // OK
{
	switch(type)
	{
		case 0:
			return GET_ITEM(12,30);
		case 1:
			return GET_ITEM(12,31);
		case 2:
			return GET_ITEM(12,136);
		case 3:
			return GET_ITEM(12,137);
		case 4:
			return GET_ITEM(12,138);
		case 5:
			return GET_ITEM(12,139);
		case 6:
			return GET_ITEM(12,140);
		case 7:
			return GET_ITEM(12,141);
		case 8:
			return GET_ITEM(12,142);
		case 9:
			return GET_ITEM(12,143);
	}

	return -1;
}

void CJewelMix::CGJewelMixRecv(PMSG_JEWEL_MIX_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}
	
	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_COMMON)
	{
		return;
	}

	if(lpObj->ChaosLock != 0)
	{
		this->GCJewelMixResultSend(aIndex,0);
		return;
	}

	if(lpMsg->type < 0 || lpMsg->type > 9)
	{
		this->GCJewelMixResultSend(aIndex,2);
		return;
	}

	if(lpMsg->level < 0 || lpMsg->level > 2)
	{
		this->GCJewelMixResultSend(aIndex,3);
		return;
	}

	lpObj->ChaosLock = 1;

	int count = gItemManager.GetInventoryItemCount(lpObj,this->GetJewelSimpleIndex(lpMsg->type),0);
	
	int JewelCount = (lpMsg->level+1)*10;

	if(count < JewelCount)
	{
		lpObj->ChaosLock = 0;
		this->GCJewelMixResultSend(aIndex,4);
		return;
	}

	int MixMoney = (lpMsg->level+1)*500000;

	if(lpObj->Money < ((DWORD)MixMoney))
	{
		lpObj->ChaosLock = 0;
		this->GCJewelMixResultSend(aIndex,5);
		return;
	}

	lpObj->Money -= MixMoney;

	GCMoneySend(aIndex,lpObj->Money);

	gItemManager.DeleteInventoryItemCount(lpObj,this->GetJewelSimpleIndex(lpMsg->type),0,JewelCount);

	GDCreateItemSend(aIndex,0xEB,0,0,this->GetJewelBundleIndex(lpMsg->type),lpMsg->level,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

	this->GCJewelMixResultSend(aIndex,1);

	lpObj->ChaosLock = 0;
}

void CJewelMix::CGJewelUnMixRecv(PMSG_JEWEL_UNMIX_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}
	
	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_COMMON)
	{
		return;
	}

	if(lpObj->ChaosLock != 0)
	{
		this->GCJewelUnMixResultSend(aIndex,0);
		return;
	}

	if(lpMsg->type < 0 || lpMsg->type > 9)
	{
		this->GCJewelUnMixResultSend(aIndex,2);
		return;
	}

	if(lpMsg->level < 0 || lpMsg->level > 2)
	{
		this->GCJewelUnMixResultSend(aIndex,3);
		return;
	}

	if(INVENTORY_FULL_RANGE(lpMsg->slot) == 0)
	{
		this->GCJewelUnMixResultSend(aIndex,5);
		return;
	}

	lpObj->ChaosLock = 1;
	
	if(lpObj->Inventory[lpMsg->slot].IsItem() == 0)
	{
		lpObj->ChaosLock = 0;
		this->GCJewelUnMixResultSend(aIndex,4);
		return;
	}

	if(lpObj->Inventory[lpMsg->slot].m_Level != lpMsg->level)
	{
		lpObj->ChaosLock = 0;
		this->GCJewelUnMixResultSend(aIndex,3);
		return;
	}

	if(lpObj->Inventory[lpMsg->slot].m_Index != this->GetJewelBundleIndex(lpMsg->type))
	{
		lpObj->ChaosLock = 0;
		this->GCJewelUnMixResultSend(aIndex,6);
		return;
	}

	int count = gItemManager.GetInventoryEmptySlotCount(lpObj);

	int JewelCount = (lpMsg->level+1)*10;

	if(count < JewelCount)
	{
		lpObj->ChaosLock = 0;
		this->GCJewelUnMixResultSend(aIndex,7);
		return;
	}

	int MixMoney = 1000000;

	if(lpObj->Money < ((DWORD)MixMoney))
	{
		lpObj->ChaosLock = 0;
		this->GCJewelUnMixResultSend(aIndex,8);
		return;
	}

	lpObj->Money -= MixMoney;

	GCMoneySend(aIndex,lpObj->Money);

	gItemManager.InventoryDelItem(aIndex,lpMsg->slot);

	gItemManager.GCItemDeleteSend(aIndex,lpMsg->slot,1);

	for(int n=0;n < JewelCount;n++)
	{
		GDCreateItemSend(aIndex,0xEB,0,0,this->GetJewelSimpleIndex(lpMsg->type),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
	}

	this->GCJewelUnMixResultSend(aIndex,1);

	lpObj->ChaosLock = 0;
}

void CJewelMix::GCJewelMixResultSend(int aIndex,BYTE result) // OK
{
	PMSG_JEWEL_MIX_RESULT_SEND pMsg;

	pMsg.header.set(0xBC,0x00,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CJewelMix::GCJewelUnMixResultSend(int aIndex,BYTE result) // OK
{
	PMSG_JEWEL_MIX_RESULT_SEND pMsg;

	pMsg.header.set(0xBC,0x01,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

bool CJewelMix::CommandJewelMix(LPOBJ lpObj,char* arg) // OK
{
	if(gObjIsConnectedGP(lpObj->Index) == 0)
	{
		return 0;
	}
	
	if(lpObj->Interface.use != 0)
	{
		return 0;
	}

	if(lpObj->ChaosLock != 0)
	{
		this->GCJewelMixResultSend(lpObj->Index,0);
		return 0;
	}

	char tipo[10] = {0};
	gCommandManager.GetString(arg,tipo,sizeof(tipo),0);

	int qtd = gCommandManager.GetNumber(arg,1);

	int type;

    if (_stricmp(tipo,"bless") == 0)
    {
		type = 0;
	}
    else if (_stricmp(tipo,"soul") == 0)
    {
		type = 1;
	}
    else if (_stricmp(tipo,"life") == 0)
    {
		type = 2;
	}
    else if (_stricmp(tipo,"creation") == 0)
    {
		type = 3;
	}
    else if (_stricmp(tipo,"guardian") == 0)
    {
		type = 4;
	}
    else if (_stricmp(tipo,"gem") == 0)
    {
		type = 5;
	}
    else if (_stricmp(tipo,"harmony") == 0)
    {
		type = 6;
	}
    else if (_stricmp(tipo,"chaos") == 0)
    {
		type = 7;
	}
    else if (_stricmp(tipo,"lowstone") == 0)
    {
		type = 8;
	}
    else if (_stricmp(tipo,"highstone") == 0)
    {
		type = 9;
	}
	else
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(820));
		return 0;
	}

	if(type < 0 || type > 9)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(822));
		return 0;
	}

	int level;
	if(qtd == 10)
	{
		level = 0;
	}
	else if (qtd == 20)
	{
		level = 1;
	}
	else if (qtd == 30)
	{
		level = 2;
	}
	else
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(820));
		return 0;
	}

	if(level < 0 || level > 2)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(822));
		return 0;
	}

	lpObj->ChaosLock = 1;

	int count = gItemManager.GetInventoryItemCount(lpObj,this->GetJewelSimpleIndex(type),0);
	
	int JewelCount = (level+1)*10;

	if(count < JewelCount)
	{
		lpObj->ChaosLock = 0;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(822));
		return 0;
	}

	gItemManager.DeleteInventoryItemCount(lpObj,this->GetJewelSimpleIndex(type),0,JewelCount);

	GDCreateItemSend(lpObj->Index,0xEB,0,0,this->GetJewelBundleIndex(type),level,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

	lpObj->ChaosLock = 0;

	gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(823));

	return 1;
}

bool CJewelMix::CommandJewelUnMix(LPOBJ lpObj,char* arg) // OK
{
	if(gObjIsConnectedGP(lpObj->Index) == 0)
	{
		return 0;
	}
	
	if(lpObj->Interface.use != 0)
	{
		return 0;
	}

	if(lpObj->ChaosLock != 0)
	{
		this->GCJewelUnMixResultSend(lpObj->Index,0);
		return 0;
	}

	char tipo[10] = {0};
	gCommandManager.GetString(arg,tipo,sizeof(tipo),0);

	int qtd = gCommandManager.GetNumber(arg,1);

	int type;

    if (_stricmp(tipo,"bless") == 0)
    {
		type = 0;
	}
    else if (_stricmp(tipo,"soul") == 0)
    {
		type = 1;
	}
    else if (_stricmp(tipo,"life") == 0)
    {
		type = 2;
	}
    else if (_stricmp(tipo,"creation") == 0)
    {
		type = 3;
	}
    else if (_stricmp(tipo,"guardian") == 0)
    {
		type = 4;
	}
    else if (_stricmp(tipo,"gem") == 0)
    {
		type = 5;
	}
    else if (_stricmp(tipo,"harmony") == 0)
    {
		type = 6;
	}
    else if (_stricmp(tipo,"chaos") == 0)
    {
		type = 7;
	}
    else if (_stricmp(tipo,"lowstone") == 0)
    {
		type = 8;
	}
    else if (_stricmp(tipo,"highstone") == 0)
    {
		type = 9;
	}
	else
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(821));
		return 0;
	}

	if(type < 0 || type > 9)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(822));
		return 0;
	}

	int level;
	if(qtd == 10)
	{
		level = 0;
	}
	else if (qtd == 20)
	{
		level = 1;
	}
	else if (qtd == 30)
	{
		level = 2;
	}
	else
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(821));
		return 0;
	}

	if(level < 0 || level > 2)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(822));
		return 0;
	}

	int slot = -1;
	for(int n=0;n < INVENTORY_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->Inventory[n].m_Index == this->GetJewelBundleIndex(type) && (lpObj->Inventory[n].m_Level == level))
		{
			slot = n;
			break;
		}
	}

	if(INVENTORY_FULL_RANGE(slot) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(822));
		return 0;
	}

	lpObj->ChaosLock = 1;
	
	if(slot < 0)
	{
		lpObj->ChaosLock = 0;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(822));
		return 0;
	}

	int count = gItemManager.GetInventoryEmptySlotCount(lpObj);

	int JewelCount = (level+1)*10;

	if(count < JewelCount)
	{
		lpObj->ChaosLock = 0;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(824));
		return 0;
	}

	gItemManager.InventoryDelItem(lpObj->Index,slot);

	gItemManager.GCItemDeleteSend(lpObj->Index,slot,1);

	for(int n=0;n < JewelCount;n++)
	{
		GDCreateItemSend(lpObj->Index,0xEB,0,0,this->GetJewelSimpleIndex(type),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
	}

	lpObj->ChaosLock = 0;

	gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(823));

	return 1;
}
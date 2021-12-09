// CustomPick.cpp: implementation of the CCustomPick class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "CustomPick.h"
#include "GensSystem.h"
#include "Log.h"
#include "Map.h"
#include "MapManager.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "ServerInfo.h"
#include "Util.h"
#include "Viewport.h"

CCustomPick gCustomPick;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomPick::CCustomPick() // OK
{
	this->m_CustomPickInfo.clear();
}

CCustomPick::~CCustomPick() // OK
{

}

void CCustomPick::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->m_CustomPickInfo.clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			CUSTOMPICK_INFO info;

			info.Index = lpMemScript->GetNumber();

			strcpy_s(info.Name,lpMemScript->GetAsString());

			info.Cat = lpMemScript->GetAsNumber();

			info.Item = lpMemScript->GetAsNumber();

			this->m_CustomPickInfo.insert(std::pair<int,CUSTOMPICK_INFO>(info.Index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CCustomPick::GetInfo(int index,CUSTOMPICK_INFO* lpInfo) // OK
{
	std::map<int,CUSTOMPICK_INFO>::iterator it = this->m_CustomPickInfo.find(index);

	if(it == this->m_CustomPickInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}
}

bool CCustomPick::GetInfoByName(char* message,CUSTOMPICK_INFO* lpInfo) // OK
{
	char command[32] = {0};

	memset(command,0,sizeof(command));

	gCommandManager.GetString(message,command,sizeof(command),0);


	for(std::map<int,CUSTOMPICK_INFO>::iterator it=this->m_CustomPickInfo.begin();it != this->m_CustomPickInfo.end();it++)
	{
		if(_stricmp(it->second.Name,command) == 0)
		{
			(*lpInfo) = it->second;
			return 1;
		}
	}

	return 0;
}

void CCustomPick::ItemGet(LPOBJ lpObj) // OK
{
	if(gObjIsConnectedGP(lpObj->Index) == 0)
	{
		return;
	}

	if(lpObj->DieRegen != 0)
	{
		return;
	}

	if(lpObj->Interface.use != 0 && lpObj->Interface.type != INTERFACE_SHOP)
	{
		return;
	}

	if(lpObj->Transaction == 1)
	{
		return;
	}

	if(lpObj->Type != OBJECT_USER)
	{
		return;
	}

	int index = -1;

	for(int n=0;n < MAX_MAP_ITEM;n++)
	{
		if(gMap[lpObj->Map].m_Item[n].m_Live == 0)
		{
			continue;
		}

		if(gMap[lpObj->Map].m_Item[n].m_State != OBJECT_CREATE && gMap[lpObj->Map].m_Item[n].m_State != OBJECT_PLAYING)
		{
			continue;
		}

		if (lpObj->PickupExc == 1 || lpObj->PickupSocket == 1 || lpObj->PickupSetItem == 1)
		{
			if(gViewport.CheckViewportObjectPosition(lpObj->Index,lpObj->Map,gMap[lpObj->Map].m_Item[n].m_X,gMap[lpObj->Map].m_Item[n].m_Y,gMapManager.GetMapViewRange(lpObj->Map)) != 0)
			{
				index = n;
			}

			if (index < 0)
			{
				continue;
			}

			if(MAP_ITEM_RANGE(index) == 0)
			{
				continue;
			}

			if(MAP_RANGE(lpObj->Map) == 0)
			{
				continue;
			}

			if(gMap[lpObj->Map].CheckItemGive(lpObj->Index,index) == 0)
			{
				continue;
			}

			CMapItem* lpItem = &gMap[lpObj->Map].m_Item[index];

			if(lpItem->IsEventItem() != 0)
			{
				continue;
			}

			if(lpObj->PickupExc == 1 &&	lpItem->IsExcItem() != 0 || lpObj->PickupSocket == 1 &&	lpItem->IsSocketItem() != 0 || lpObj->PickupSetItem == 1 &&	lpItem->IsSetItem() != 0)
			{
				PMSG_ITEM_GET_SEND pMsg;

				pMsg.header.set(0x22,sizeof(pMsg));

				pMsg.result = 0xFF;

				memset(pMsg.ItemInfo,0,sizeof(pMsg.ItemInfo)); 

				gObjFixInventoryPointer(lpObj->Index);

				if(lpItem->m_Index == GET_ITEM(14,15)) // Money
				{

					gMap[lpObj->Map].ItemGive(lpObj->Index,index);

					if(gObjCheckMaxMoney(lpObj->Index,lpItem->m_BuyMoney) == 0)
					{
						lpObj->Money = MAX_MONEY;
					}
					else
					{
						lpObj->Money += lpItem->m_BuyMoney;
					}

					pMsg.result = 0xFE;

					pMsg.ItemInfo[0] = SET_NUMBERHB(SET_NUMBERHW(lpObj->Money));
					pMsg.ItemInfo[1] = SET_NUMBERLB(SET_NUMBERHW(lpObj->Money));
					pMsg.ItemInfo[2] = SET_NUMBERHB(SET_NUMBERLW(lpObj->Money));
					pMsg.ItemInfo[3] = SET_NUMBERLB(SET_NUMBERLW(lpObj->Money));

					DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
					continue;
				}

				CItem item = (*lpItem);

				if(gItemManager.InventoryInsertItemStack(lpObj,&item) != 0)
				{
					gMap[lpObj->Map].ItemGive(lpObj->Index,index);

					pMsg.result = 0xFD;

					gItemManager.ItemByteConvert(pMsg.ItemInfo,item);

					DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
					continue;
				}

				pMsg.result = gItemManager.InventoryInsertItem(lpObj->Index,item);

				if(pMsg.result == 0xFF)
				{
					DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
					continue;
				}

				gMap[lpObj->Map].ItemGive(lpObj->Index,index);

				gItemManager.ItemByteConvert(pMsg.ItemInfo,item);

				DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

				//LogAdd(LOG_BLACK,"%d",index);

				GCPartyItemInfoSend(lpObj->Index,&item);
			}

		}

		//Custom Pick
		for (int i = 0; i < MAX_CUSTOMPICK; i++)
		{
			if (lpObj->Pickup[i] == -1)
			{
				continue;
			}

			if (lpObj->Pickup[i] != gMap[lpObj->Map].m_Item[n].m_Index)
			{
				continue;
			}

			if(gViewport.CheckViewportObjectPosition(lpObj->Index,lpObj->Map,gMap[lpObj->Map].m_Item[n].m_X,gMap[lpObj->Map].m_Item[n].m_Y,gMapManager.GetMapViewRange(lpObj->Map)) != 0)
			{
				index = n;
			}

			if (index < 0)
			{
				continue;
			}

			if(MAP_ITEM_RANGE(index) == 0)
			{
				continue;
			}

			if(MAP_RANGE(lpObj->Map) == 0)
			{
				continue;
			}

			if(gMap[lpObj->Map].CheckItemGive(lpObj->Index,index) == 0)
			{
				continue;
			}

			CMapItem* lpItem = &gMap[lpObj->Map].m_Item[index];

			if(lpItem->IsEventItem() != 0)
			{
				continue;
			}

			PMSG_ITEM_GET_SEND pMsg;

			pMsg.header.set(0x22,sizeof(pMsg));

			pMsg.result = 0xFF;

			memset(pMsg.ItemInfo,0,sizeof(pMsg.ItemInfo)); 

			gObjFixInventoryPointer(lpObj->Index);

			if(lpItem->m_Index == GET_ITEM(14,15)) // Money
			{

				gMap[lpObj->Map].ItemGive(lpObj->Index,index);

				if(gObjCheckMaxMoney(lpObj->Index,lpItem->m_BuyMoney) == 0)
				{
					lpObj->Money = MAX_MONEY;
				}
				else
				{
					lpObj->Money += lpItem->m_BuyMoney;
				}

				pMsg.result = 0xFE;

				pMsg.ItemInfo[0] = SET_NUMBERHB(SET_NUMBERHW(lpObj->Money));
				pMsg.ItemInfo[1] = SET_NUMBERLB(SET_NUMBERHW(lpObj->Money));
				pMsg.ItemInfo[2] = SET_NUMBERHB(SET_NUMBERLW(lpObj->Money));
				pMsg.ItemInfo[3] = SET_NUMBERLB(SET_NUMBERLW(lpObj->Money));

				DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
				continue;
			}

			CItem item = (*lpItem);

			if(gItemManager.InventoryInsertItemStack(lpObj,&item) != 0)
			{
				gMap[lpObj->Map].ItemGive(lpObj->Index,index);

				pMsg.result = 0xFD;

				gItemManager.ItemByteConvert(pMsg.ItemInfo,item);

				DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
				continue;
			}

			pMsg.result = gItemManager.InventoryInsertItem(lpObj->Index,item);

			if(pMsg.result == 0xFF)
			{
				DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
				continue;
			}

			gMap[lpObj->Map].ItemGive(lpObj->Index,index);

			gItemManager.ItemByteConvert(pMsg.ItemInfo,item);

			DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

			//LogAdd(LOG_BLACK,"%d",index);

			GCPartyItemInfoSend(lpObj->Index,&item);
		}
	}
}


void CCustomPick::PickProc(LPOBJ lpObj) // OK
{

	if (lpObj->PickupEnable == 0)
	{
		return;
	}

	if (gMapManager.GetMapCustomPick(lpObj->Map) == 0)
	{
		this->OnPickClose(lpObj);
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(754));
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(36));
		return;
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0)
	{
		this->OnPickClose(lpObj);
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(754));
		return;
	}

	if(lpObj->Interface.use != 0 || lpObj->TradeDuel != 0 || lpObj->Teleport != 0 || lpObj->DieRegen != 0 || lpObj->PShopOpen != 0)
	{
		this->OnPickClose(lpObj);
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(754));
		return;
	}


	this->ItemGet(lpObj);

}

bool CCustomPick::CommandPick(LPOBJ lpObj,char* arg)   
{
#if GAMESERVER_CLIENTE_UPDATE >= 5

	if(lpObj->Interface.use != 0 ||	lpObj->TradeDuel != 0 || lpObj->Teleport != 0 || lpObj->DieRegen != 0 || lpObj->PShopOpen != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(659));
		return 0;
	}

	if (gMapManager.GetMapCustomPick(lpObj->Map) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(36));
		return 0;
	}

	char mode[32] = {0};

	gCommandManager.GetString(arg,mode,sizeof(mode),0);

	if(strcmp(mode,"exc") == 0)
	{
		if (lpObj->PickupExc == 1)
		{
			lpObj->PickupExc = 0;
		    gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(753),mode);

			gLog.Output(LOG_COMMAND,"[CustomPick][%s][%s] - (Disable: %s)",lpObj->Account,lpObj->Name,mode);
			return 0;
		}
		else
		{
			lpObj->PickupExc = 1;
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(752),mode);

			lpObj->PickupEnable = 1;

			gLog.Output(LOG_COMMAND,"[CustomPick][%s][%s] - (Enable: %s)",lpObj->Account,lpObj->Name,mode);
			return 1;
		}
	}

	if(strcmp(mode,"socket") == 0)
	{
		if (lpObj->PickupSocket == 1)
		{
			lpObj->PickupSocket = 0;
		    gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(753),mode);

			gLog.Output(LOG_COMMAND,"[CustomPick][%s][%s] - (Disable: %s)",lpObj->Account,lpObj->Name,mode);
			return 0;
		}
		else
		{
			lpObj->PickupSocket = 1;
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(752),mode);

			lpObj->PickupEnable = 1;

			gLog.Output(LOG_COMMAND,"[CustomPick][%s][%s] - (Enable: %s)",lpObj->Account,lpObj->Name,mode);
			return 1;
		}
	}

	if(strcmp(mode,"setitem") == 0)
	{
		if (lpObj->PickupSetItem == 1)
		{
			lpObj->PickupSetItem = 0;
		    gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(753),mode);

			gLog.Output(LOG_COMMAND,"[CustomPick][%s][%s] - (Disable: %s)",lpObj->Account,lpObj->Name,mode);
			return 0;
		}
		else
		{
			lpObj->PickupSetItem = 1;
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(752),mode);

			lpObj->PickupEnable = 1;

			gLog.Output(LOG_COMMAND,"[CustomPick][%s][%s] - (Enable: %s)",lpObj->Account,lpObj->Name,mode);
			return 1;
		}
	}

	if(strcmp(mode,"") != 0)
	{

		CUSTOMPICK_INFO CustomPickInfo;

		if(this->GetInfoByName(mode,&CustomPickInfo) == 0)
		{
			return 0;
		}

		if (lpObj->Pickup[CustomPickInfo.Index] == ((CustomPickInfo.Cat*512)+CustomPickInfo.Item))
		{
			lpObj->Pickup[CustomPickInfo.Index] = -1;
		    gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(753),mode);

			gLog.Output(LOG_COMMAND,"[CustomPick][%s][%s] - (Disable: %s)",lpObj->Account,lpObj->Name,mode);
			return 0;
		}
		else
		{
			lpObj->Pickup[CustomPickInfo.Index] = ((CustomPickInfo.Cat*512)+CustomPickInfo.Item);
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(752),mode);

			lpObj->PickupEnable = 1;

			gLog.Output(LOG_COMMAND,"[CustomPick][%s][%s] - (Enable: %s)",lpObj->Account,lpObj->Name,mode);
			return 1;
		}

	}
	return 1;

	
#endif

}

void CCustomPick::OnPickClose(LPOBJ lpObj) // OK
{
	lpObj->PickupEnable = 0;
	lpObj->PickupExc	= 0;
	lpObj->PickupSocket = 0;
	lpObj->PickupSetItem = 0;

	for (int i = 0; i < MAX_CUSTOMPICK; i++)
	{
		lpObj->Pickup[i] = -1;
	}
}
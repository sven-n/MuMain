// ItemBag.cpp: implementation of the CItemBag class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemBag.h"
#include "DSProtocol.h"
#include "ItemOptionRate.h"
#include "ItemExcellentOptionRate.h"
#include "ItemManager.h"
#include "JewelOfHarmonyOption.h"
#include "Map.h"
#include "MemScript.h"
#include "Protocol.h"
#include "ServerInfo.h"
#include "SetItemType.h"
#include "SocketItemOption.h"
#include "SocketItemType.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemBag::CItemBag() // OK
{
	this->Init();
}

CItemBag::~CItemBag() // OK
{

}

void CItemBag::Init() // OK
{
	memset(this->m_EventName,0,sizeof(this->m_EventName));

	this->m_DropZen = 0;
	this->m_ItemDropRate = 0;
	this->m_ItemDropCount = 0;
	this->m_SetItemDropRate = 0;
	this->m_ItemDropType = 0;
	this->m_SendFirework = 0;
	this->m_count = 0;
}

void CItemBag::Load(char* path) // OK
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

	this->Init();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					strcpy_s(this->m_EventName,lpMemScript->GetString());

					this->m_DropZen = lpMemScript->GetAsNumber();

					this->m_ItemDropRate = lpMemScript->GetAsNumber();

					this->m_ItemDropCount = lpMemScript->GetAsNumber();

					this->m_SetItemDropRate = lpMemScript->GetAsNumber();

					this->m_ItemDropType = lpMemScript->GetAsNumber();

					this->m_SendFirework = lpMemScript->GetAsNumber();
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_BAG_INFO info;

					memset(&info,0,sizeof(info));

					info.Index = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(),lpMemScript->GetAsNumber()));

					info.MinLevel = lpMemScript->GetAsNumber();

					info.MaxLevel = lpMemScript->GetAsNumber();

					info.Option1 = lpMemScript->GetAsNumber();

					info.Option2 = lpMemScript->GetAsNumber();

					info.Option3 = lpMemScript->GetAsNumber();

					info.NewOption = lpMemScript->GetAsNumber();

					info.SetOption = 0;

					info.SocketOption = 0;

					this->SetInfo(info);
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_BAG_INFO info;

					memset(&info,0,sizeof(info));

					info.Index = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(),lpMemScript->GetAsNumber()));

					info.MinLevel = lpMemScript->GetAsNumber();

					info.MaxLevel = lpMemScript->GetAsNumber();

					info.Option1 = lpMemScript->GetAsNumber();

					info.Option2 = lpMemScript->GetAsNumber();

					info.Option3 = lpMemScript->GetAsNumber();

					info.NewOption = lpMemScript->GetAsNumber();

					info.SetOption = lpMemScript->GetAsNumber();

					info.SocketOption = lpMemScript->GetAsNumber();

					this->SetInfo(info);
				}
				else
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	this->m_ItemBagEx.Load(path);

	delete lpMemScript;
}

void CItemBag::SetInfo(ITEM_BAG_INFO info) // OK
{
	if(this->m_count < 0 || this->m_count >= MAX_BAG_ITEM)
	{
		return;
	}

	this->m_BagInfo[this->m_count++] = info;
}

ITEM_BAG_INFO* CItemBag::GetInfo(int index) // OK
{
	if(index < 0 || index >= this->m_count)
	{
		return 0;
	}

	return &this->m_BagInfo[index];
}

int CItemBag::GetItemLevel(ITEM_BAG_INFO* lpInfo) // OK
{
	if(lpInfo->MinLevel >= lpInfo->MaxLevel)
	{
		return lpInfo->MinLevel;
	}

	return (lpInfo->MinLevel+(GetLargeRand()%((lpInfo->MaxLevel-lpInfo->MinLevel)+1)));
}

int CItemBag::GetItemNewOption(ITEM_BAG_INFO* lpInfo) // OK
{
	int count = 0;

	int NewOption = 0;

	while(true)
	{
		if(count >= lpInfo->NewOption || count >= MAX_EXC_OPTION)
		{
			break;
		}

		//int value = 1 << (GetLargeRand()%MAX_EXC_OPTION);

		BYTE ExcOption;

		if(gItemExOptionRate.GetExcellentOptionByRate(lpInfo->Index,NewOption,&ExcOption) == 0)
		{
			break;
		}

		if((NewOption & ExcOption) == 0)
		{
			NewOption |= ExcOption;
			count++;
		}
	}

	return NewOption;
}

int CItemBag::GetItemSetOption(ITEM_BAG_INFO* lpInfo) // OK
{
	if(lpInfo->SetOption == 0)
	{
		return 0;
	}

	int SetOption = 0;

	for(int n=0;n < MAX_SET_ITEM_OPTION_INDEX;n++)
	{
		if(gSetItemType.GetSetItemOptionIndex(lpInfo->Index,n) <= 0)
		{
			continue;
		}

		SetOption = ((SetOption==0)?(n+1):(((GetLargeRand()%2)==0)?(n+1):SetOption));
	}

	if(SetOption == 0)
	{
		return 0;
	}

	if((GetLargeRand()%100) < 80)
	{
		SetOption |= 4;
	}
	else
	{
		SetOption |= 8;
	}

	return SetOption;
}

bool::CItemBag::GetRandomItemDropLocation(int map,int* ox,int* oy,int tx,int ty,int count) // OK
{
	int x = (*ox);
	int y = (*oy);

	tx = ((tx<1)?1:tx);
	ty = ((ty<1)?1:ty);

	for(int n=0;n < count;n++)
	{
		(*ox) = ((GetLargeRand()%(tx+1))*((GetLargeRand()%2==0)?-1:1))+x;
		(*oy) = ((GetLargeRand()%(ty+1))*((GetLargeRand()%2==0)?-1:1))+y;

		if(gMap[map].CheckAttr((*ox),(*oy),4) == 0 && gMap[map].CheckAttr((*ox),(*oy),8) == 0)
		{
			return 1;
		}
	}

	return 0;
}

bool CItemBag::GetItem(LPOBJ lpObj,CItem* lpItem) // OK
{
	if(this->m_ItemBagEx.m_ItemBagInfo.empty() == 0)
	{
		return this->m_ItemBagEx.GetItem(lpObj,lpItem);
	}

	if(this->m_count == 0)
	{
		return 0;
	}

	ITEM_BAG_INFO* lpInfo = this->GetInfo(GetLargeRand()%this->m_count);

	if(lpInfo == 0)
	{
		return 0;
	}

	int level = this->GetItemLevel(lpInfo);
	int Option1 = lpInfo->Option1;
	int Option2 = lpInfo->Option2;
	int Option3 = lpInfo->Option3;
	int NewOption = this->GetItemNewOption(lpInfo);
	int SetOption = this->GetItemSetOption(lpInfo);

	if(this->m_ItemDropType != 0)
	{
		if(Option1 != 0)
		{
			Option1 = GetLargeRand()%2;
		}

		if(Option2 != 0)
		{
			Option2 = GetLargeRand()%2;
		}

		Option3 = 1+(GetLargeRand()%gServerInfo.m_MaxItemOption);

		if((GetLargeRand()%100) < (Option3*3))
		{
			Option3 = (gServerInfo.m_MaxItemOption+1)-Option3;
		}
		else
		{
			Option3 = 0;
		}

		if(NewOption != 0)
		{
			Option1 = 1;
		}
	}

	BYTE SocketOption[MAX_SOCKET_OPTION] = {SOCKET_ITEM_OPTION_NONE,SOCKET_ITEM_OPTION_NONE,SOCKET_ITEM_OPTION_NONE,SOCKET_ITEM_OPTION_NONE,SOCKET_ITEM_OPTION_NONE};

	if(gSocketItemType.CheckSocketItemType(lpInfo->Index) != 0)
	{
		if(lpInfo->SocketOption == 0)
		{
			gItemOptionRate.GetItemOption6(0,&SocketOption[0]);
			gItemOptionRate.MakeSocketOption(lpInfo->Index,SocketOption[0],&SocketOption[0]);
		}
		else
		{
			for(int n=0;n < lpInfo->SocketOption;n++)
			{
				SocketOption[n] = 0xFE;
			}
		}
	}

	lpItem->m_Level = level;

	lpItem->Convert(lpInfo->Index,Option1,Option2,Option3,NewOption,SetOption,0,0,SocketOption,0xFF);
	return 1;
}

bool CItemBag::DropItem(LPOBJ lpObj,int map,int x,int y) // OK
{
	if(this->m_ItemBagEx.m_ItemBagInfo.empty() == 0)
	{
		return this->m_ItemBagEx.DropItem(lpObj,map,x,y);
	}

	for(int n=0;n < this->m_ItemDropCount;n++)
	{
		int px = x;
		int py = y;

		if(this->m_ItemDropCount > 1 || (gMap[map].CheckAttr(px,py,4) != 0 || gMap[map].CheckAttr(px,py,8) != 0))
		{
			if(this->GetRandomItemDropLocation(map,&px,&py,2,2,10) == 0)
			{
				px = lpObj->X;
				py = lpObj->Y;
			}
		}

		if((GetLargeRand()%100) >= this->m_ItemDropRate)
		{
			gMap[map].MoneyItemDrop(this->m_DropZen,px,py);
			continue;
		}

		if((GetLargeRand()%10000) < this->m_SetItemDropRate)
		{
			gSetItemType.MakeRandomSetItem(lpObj->Index,map,px,py);
			continue;
		}

		CItem item;

		if(this->GetItem(lpObj,&item) != 0)
		{
			GDCreateItemSend(lpObj->Index,map,px,py,item.m_Index,(BYTE)item.m_Level,0,item.m_Option1,item.m_Option2,item.m_Option3,lpObj->Index,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus,0);
			continue;
		}
	}

	if(this->m_SendFirework != 0)
	{
		GCFireworksSend(lpObj,lpObj->X,lpObj->Y);
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////
// NewUIItemMng.cpp: implementation of the CNewUIItemMng class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIItemMng.h"
#include "CSItemOption.h"
#include "GIPetManager.h"
#include "ZzzInfomation.h"
#include "SocketSystem.h"

using namespace SEASON3B;

SEASON3B::CNewUIItemMng::CNewUIItemMng() 
{
	m_dwAlternate = 0;
	m_dwAvailableKeyStream = 0x80000000;
	m_UpdateTimer.SetTimer(1000);
}

SEASON3B::CNewUIItemMng::~CNewUIItemMng() 
{ 
	DeleteAllItems(); 
}

ITEM* SEASON3B::CNewUIItemMng::CreateItem(BYTE* pbyItemPacket)
{
	size_t ItemPacketSize = 5;
	ItemPacketSize++;
	ItemPacketSize++;

	if(IsBadReadPtr(pbyItemPacket, ItemPacketSize))
		return NULL;

	WORD wType = ExtractItemType(pbyItemPacket);
	BYTE byOption380 = 0, byOptionHarmony = 0;
	
	byOption380 = pbyItemPacket[5];
	byOptionHarmony = pbyItemPacket[6];

	BYTE bySocketOption[5] = { pbyItemPacket[7], pbyItemPacket[8], pbyItemPacket[9], pbyItemPacket[10], pbyItemPacket[11] };
	
	return CNewUIItemMng::CreateItem(wType/MAX_ITEM_INDEX, wType%MAX_ITEM_INDEX, pbyItemPacket[1], pbyItemPacket[2], pbyItemPacket[3], pbyItemPacket[4], byOption380, byOptionHarmony, bySocketOption);
}

ITEM* SEASON3B::CNewUIItemMng::CreateItem(BYTE byType, BYTE bySubType, BYTE byLevel /* = 0 */, BYTE byDurability /* = 255 */,BYTE byOption1 /* = 0 */, BYTE byOptionEx /* = 0 */, BYTE byOption380 /* = 0 */,BYTE byOptionHarmony /* = 0 */	, BYTE * pbySocketOptions /*= NULL*/)
{
	ITEM* pNewItem = new ITEM;
	memset(pNewItem, 0, sizeof(ITEM));

	WORD wType = byType * MAX_ITEM_INDEX + bySubType;
	pNewItem->Key = GenerateItemKey();
	pNewItem->Type = wType;
	pNewItem->Durability = byDurability;
	pNewItem->Option1 = byOption1;
	pNewItem->ExtOption = byOptionEx;
	if( (((byOption380 & 0x08) << 4) >> 7) > 0)
		pNewItem->option_380 = true;
	else
		pNewItem->option_380 = false;
	pNewItem->Jewel_Of_Harmony_Option = (byOptionHarmony & 0xf0) >> 4;
	pNewItem->Jewel_Of_Harmony_OptionLevel = byOptionHarmony & 0x0f;

	if (pbySocketOptions == NULL)
	{
		pNewItem->SocketCount = 0;
		assert(!"여기로 오는 경우에 소켓 옵션 관련 확인 필요");
	}
	else
	{
		pNewItem->SocketCount = MAX_SOCKETS;

		for (int i = 0; i < MAX_SOCKETS; ++i)
		{
			pNewItem->bySocketOption[i] = pbySocketOptions[i];
		}

		for (int i = 0; i < MAX_SOCKETS; ++i)
		{
			if (pbySocketOptions[i] == 0xFF)
			{
				pNewItem->SocketCount = i;
				break;
			}
			else if (pbySocketOptions[i] == 0xFE)
			{
				pNewItem->SocketSeedID[i] = SOCKET_EMPTY;
			}
			else
			{
				pNewItem->SocketSeedID[i] = pbySocketOptions[i] % SEASON4A::MAX_SOCKET_OPTION;
				pNewItem->SocketSphereLv[i] = int(pbySocketOptions[i] / SEASON4A::MAX_SOCKET_OPTION) + 1;
			}
		}
		
  		if (g_SocketItemMgr.IsSocketItem(pNewItem))
		{
			pNewItem->SocketSeedSetOption = byOptionHarmony;
			pNewItem->Jewel_Of_Harmony_Option = 0;
			pNewItem->Jewel_Of_Harmony_OptionLevel = 0;
		}
		else
		{
			pNewItem->SocketSeedSetOption = SOCKET_EMPTY;
		}
	}

	pNewItem->byColorState = ITEM_COLOR_NORMAL;

	pNewItem->RefCount = 1;
	
	if( ((byOption380&0x02) >> 1) > 0 )
	{
		pNewItem->bPeriodItem = true;
	}
	else 
	{
		pNewItem->bPeriodItem = false;
	}

	if( ((byOption380&0x04) >> 2) > 0 )
	{
		pNewItem->bExpiredPeriod = true;
	}
	else 
	{
		pNewItem->bExpiredPeriod = false;
	}

	SetItemAttr(pNewItem, byLevel, byOption1, byOptionEx);

	m_listItem.push_back(pNewItem);
	return pNewItem;
}

ITEM* SEASON3B::CNewUIItemMng::CreateItem(ITEM* pItem)
{
	pItem->RefCount++;
	return pItem;
}

ITEM* SEASON3B::CNewUIItemMng::DuplicateItem(ITEM* pItem)
{
	ITEM* pNewItem = new ITEM;
	memcpy(pNewItem, pItem, sizeof(ITEM));
	pNewItem->Key = GenerateItemKey();
	pNewItem->RefCount = 1;
	m_listItem.push_back(pNewItem);
	return pNewItem;
}

void SEASON3B::CNewUIItemMng::DeleteItem(ITEM* pItem)
{
	if(pItem == NULL)
		return;
	
	if(--pItem->RefCount <= 0)
	{
		type_list_item::iterator li = m_listItem.begin();
		for(; li != m_listItem.end(); li++)
		{
			if((*li) == pItem)
			{
				SAFE_DELETE(*li);
				m_listItem.erase(li);
				break;
			}
		}
	}
}

void SEASON3B::CNewUIItemMng::DeleteDuplicatedItem(ITEM* pItem)
{
	if(pItem == NULL)
		return;

	if(--pItem->RefCount <= 0)
	{
		DeleteItem( pItem );
	}
}

void SEASON3B::CNewUIItemMng::DeleteAllItems()
{
	type_list_item::iterator li = m_listItem.begin();
	for(; li != m_listItem.end(); li++)
	{
		SAFE_DELETE(*li);
	}
	m_listItem.clear();

	m_dwAlternate = 0;
	m_dwAvailableKeyStream = 0x80000000;
}

bool SEASON3B::CNewUIItemMng::IsEmpty()
{ 
	return m_listItem.empty(); 
}

void SEASON3B::CNewUIItemMng::Update()
{
	m_UpdateTimer.UpdateTime();
	if(m_UpdateTimer.IsTime())
	{
		type_list_item::iterator li = m_listItem.begin();
		for(; li != m_listItem.end(); )
		{
			if((*li)->RefCount <=0 )
			{
				delete (*li);
				li = m_listItem.erase(li);
			}
			else
				li++;
		}
	}
}

DWORD SEASON3B::CNewUIItemMng::GenerateItemKey()
{
	DWORD dwAvailableItemKey = FindAvailableKeyIndex(m_dwAvailableKeyStream);
	if(dwAvailableItemKey >= 0x8F000000)
	{
		m_dwAvailableKeyStream = 0;
		m_dwAlternate++;
		dwAvailableItemKey = FindAvailableKeyIndex(m_dwAvailableKeyStream);
	}
	return m_dwAvailableKeyStream = dwAvailableItemKey;
}

DWORD SEASON3B::CNewUIItemMng::FindAvailableKeyIndex(DWORD dwSeed)
{
	if(m_dwAlternate > 0)
	{
		type_list_item::iterator li = m_listItem.begin();
		for(; li != m_listItem.end(); li++)
		{
			ITEM* pItem = (*li);
			if(pItem->Key == dwSeed+1)
				return FindAvailableKeyIndex(dwSeed+1);
		}
	}
	return dwSeed+1;
}

WORD SEASON3B::CNewUIItemMng::ExtractItemType(BYTE* pbyItemPacket)
{
	return pbyItemPacket[0] + (pbyItemPacket[3]&128)*2 + (pbyItemPacket[5]&240)*32;
}

void SEASON3B::CNewUIItemMng::SetItemAttr(ITEM* pItem, BYTE byLevel, BYTE byOption1, BYTE byOptionEx)
{
	ItemConvert(pItem, byLevel, byOption1, byOptionEx);
}

//////////////////////////////////////////////////////////////////////////
//  
//  UIGuardsMan.cpp
//  
//  내  용 : 근위병 인터페이스
//  
//  날  짜 : 2004년 11월 12일
//  
//  작성자 : 강 병 국
//  
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "wsclientinline.h"
#include "UIGuildInfo.h"
#include "UIGuardsMan.h"
#include "NewUIMyInventory.h"

CUIGuardsMan g_GuardsMan;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIGuardsMan::CUIGuardsMan()
{
	m_dwRegMark		= 0;
	m_eRegStatus	= REG_STATUS_NONE;
}

CUIGuardsMan::~CUIGuardsMan()
{
}

bool CUIGuardsMan::IsSufficentDeclareLevel()
{
	if( Hero->GuildStatus != G_MASTER )
		return false;

	if( CharacterAttribute->Level >= BC_REQ_LEVEL )
	{
		return true;
	}
	else
	{
		return false;
	}
}

DWORD CUIGuardsMan::GetMyMarkCount()
{
	DWORD dwResult = 0;

	SEASON3B::CNewUIInventoryCtrl * pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
	ITEM * pItem = NULL;
#ifdef _VS2008PORTING
	for (int i = 0; i < (int)pNewInventoryCtrl->GetNumberOfItems(); ++i)
#else // _VS2008PORTING
	for (int i = 0; i < pNewInventoryCtrl->GetNumberOfItems(); ++i)
#endif // _VS2008PORTING
	{
		pItem = pNewInventoryCtrl->GetItem(i);
		int nItemLevel = (pItem->Level>>3)&15;
		if( pItem->Type == ITEM_POTION+21 && nItemLevel == 3 )	// 성주의표식
        {
	        dwResult += pItem->Durability;
        }
	}

	return dwResult;
}

int CUIGuardsMan::GetMyMarkSlotIndex()
{
	SEASON3B::CNewUIInventoryCtrl * pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
	ITEM * pItem = NULL;
#ifdef _VS2008PORTING
	for (int i = 0; i < (int)pNewInventoryCtrl->GetNumberOfItems(); ++i)
#else // _VS2008PORTING	
	for (int i = 0; i < pNewInventoryCtrl->GetNumberOfItems(); ++i)
#endif // _VS2008PORTING
	{
		pItem = pNewInventoryCtrl->GetItem(i);
		int nItemLevel = (pItem->Level>>3)&15;
		if( pItem->Type == ITEM_POTION+21 && nItemLevel == 3 )	// 성주의표식
        {
	        return pItem->y * COLUMN_INVENTORY + pItem->x;
        }
	}

	return -1;
}

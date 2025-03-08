//////////////////////////////////////////////////////////////////////////
//  UIGuardsMan.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"


#include "UIGuildInfo.h"
#include "UIGuardsMan.h"
#include "NewUIMyInventory.h"
#include "NewUISystem.h"

CUIGuardsMan g_GuardsMan;

CUIGuardsMan::CUIGuardsMan()
{
    m_dwRegMark = 0;
    m_eRegStatus = REG_STATUS_NONE;
}

CUIGuardsMan::~CUIGuardsMan()
{
}

bool CUIGuardsMan::IsSufficentDeclareLevel()
{
    if (Hero->GuildStatus != G_MASTER)
        return false;

    if (CharacterAttribute->Level >= BC_REQ_LEVEL)
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

    SEASON3B::CNewUIInventoryCtrl* pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
    ITEM* pItem = NULL;

    for (int i = 0; i < (int)pNewInventoryCtrl->GetNumberOfItems(); ++i)
    {
        pItem = pNewInventoryCtrl->GetItem(i);
        int nItemLevel = pItem->Level;
        if (pItem->Type == ITEM_POTION + 21 && nItemLevel == 3)
        {
            dwResult += pItem->Durability;
        }
    }

    return dwResult;
}

int CUIGuardsMan::GetMyMarkSlotIndex()
{
    SEASON3B::CNewUIInventoryCtrl* pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
    ITEM* pItem = NULL;

    for (int i = 0; i < (int)pNewInventoryCtrl->GetNumberOfItems(); ++i)
    {
        pItem = pNewInventoryCtrl->GetItem(i);
        int nItemLevel = pItem->Level;
        if (pItem->Type == ITEM_POTION + 21 && nItemLevel == 3)
        {
            return pItem->y * COLUMN_INVENTORY + pItem->x;
        }
    }

    return -1;
}
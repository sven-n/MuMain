// NewUIInventoryActionController.cpp
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIInventoryActionController.h"
#include "NewUIMyInventory.h"
#include "NewUIInventoryCtrl.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "ZzzInventory.h"
#include "UIJewelHarmony.h"
#include "CSItemOption.h"
#include "MapManager.h"
#include "SocketSystem.h"
#include "MonkSystem.h"
#include "CharacterManager.h"
#include "DSPlaySound.h"
#include "ZzzInterface.h"
#include "UIManager.h"
#include "ChangeRingManager.h"
#include "PortalMgr.h"
#include "CSQuest.h"

namespace SEASON3B
{

CNewUIInventoryActionController::CNewUIInventoryActionController()
    : m_pContext(nullptr)
{
}

void CNewUIInventoryActionController::SetContext(IInventoryActionContext* pContext)
{
    m_pContext = pContext;
}

bool CNewUIInventoryActionController::HandleInventoryActions(CNewUIInventoryCtrl* targetControl) const
{
    if (m_pContext == nullptr || targetControl == nullptr)
    {
        return false;
    }

    if (CNewUIInventoryCtrl::GetPickedItem() && IsRelease(VK_LBUTTON))
    {
        return HandlePickedItemPlacement(targetControl);
    }

    if (m_pContext->GetRepairMode() == REPAIR_MODE_OFF && IsPress(VK_RBUTTON))
    {
        return HandleRightClick(targetControl);
    }

    if (m_pContext->GetRepairMode() == REPAIR_MODE_ON && IsPress(VK_LBUTTON))
    {
        return HandleRepairClick(targetControl);
    }

    return false;
}

bool CNewUIInventoryActionController::HandlePickedItemPlacement(CNewUIInventoryCtrl* targetControl) const
{
    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem == nullptr)
    {
        return false;
    }

    ITEM* pPickItem = pPickedItem->GetItem();
    if (pPickItem == nullptr)
    {
        return true;
    }

    const int iSourceIndex = pPickedItem->GetSourceLinealPos();
    const int iTargetIndex = pPickedItem->GetTargetLinealPos(targetControl);

    const bool bFromInventorySystem =
        (pPickedItem->GetOwnerInventory() == targetControl)
        || (g_pMyInventoryExt != nullptr && g_pMyInventoryExt->GetOwnerOf(pPickedItem) != nullptr);

    if (bFromInventorySystem)
    {
        if (ApplyJewels(targetControl, pPickedItem, pPickItem, iSourceIndex, iTargetIndex))
        {
            return true;
        }

        if (iTargetIndex != -1 && TryStackItems(targetControl, pPickItem, iSourceIndex, iTargetIndex))
        {
            return true;
        }
    }

    if (TryMoveItem(targetControl, pPickedItem, pPickItem, iSourceIndex, iTargetIndex))
    {
        return true;
    }

    return false;
}

bool CNewUIInventoryActionController::TryApplyJewel(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
{
    return ApplyJewels(targetControl, pPickedItem, pPickItem, iSourceIndex, iTargetIndex);
}

bool CNewUIInventoryActionController::TryStackItem(CNewUIInventoryCtrl* targetControl, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
{
    return TryStackItems(targetControl, pPickItem, iSourceIndex, iTargetIndex);
}

bool CNewUIInventoryActionController::TryMoveItem(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
{
    if (iTargetIndex < 0 || !targetControl->CanMove(iTargetIndex, pPickItem))
    {
        return false;
    }

    const auto sourceStorageType = pPickedItem->GetSourceStorageType();
    const auto targetStorageType = targetControl->GetStorageType();

    if (iTargetIndex != iSourceIndex)
    {
        return SendRequestEquipmentItem(sourceStorageType, iSourceIndex,
            pPickItem, targetStorageType, iTargetIndex);
    }

    CNewUIInventoryCtrl::BackupPickedItem();
    return false;
}

bool CNewUIInventoryActionController::HandleRepairClick(CNewUIInventoryCtrl* targetControl) const
{
    return RepairItemAtMousePoint(targetControl);
}

bool CNewUIInventoryActionController::HandleRightClick(CNewUIInventoryCtrl* targetControl) const
{
    m_pContext->ResetMouseRButton();

    if (g_pNewUISystem->IsVisible(INTERFACE_STORAGE))
    {
        return HandleStorageAutoMove(targetControl);
    }

    if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) && g_pNewUISystem->IsVisible(INTERFACE_INVENTORY))
    {
        return HandleSellToNPC(targetControl);
    }

    if (g_pNewUISystem->IsVisible(INTERFACE_INVENTORY)
        && !g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP)
        && !g_pNewUISystem->IsVisible(INTERFACE_TRADE)
        && !g_pNewUISystem->IsVisible(INTERFACE_DEVILSQUARE)
        && !g_pNewUISystem->IsVisible(INTERFACE_BLOODCASTLE)
        && !g_pNewUISystem->IsVisible(INTERFACE_LUCKYITEMWND)
        && !g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY)
        && !g_pNewUISystem->IsVisible(INTERFACE_MYSHOP_INVENTORY))
    {
        return HandleInventoryRightClickActions(targetControl);
    }

    return false;
}

bool CNewUIInventoryActionController::HandleStorageAutoMove(CNewUIInventoryCtrl* targetControl) const
{
    if (g_pStorageInventory->ProcessMyInvenItemAutoMove(targetControl))
    {
        return true;
    }

    if (g_pNewUISystem->IsVisible(INTERFACE_STORAGE_EXT))
    {
        return g_pStorageInventoryExt->ProcessMyInvenItemAutoMove(targetControl);
    }

    return false;
}

bool CNewUIInventoryActionController::HandleSellToNPC(CNewUIInventoryCtrl* targetControl) const
{
    if (CNewUIInventoryCtrl::GetPickedItem())
    {
        return false;
    }

    if (!targetControl->CheckPtInRect(MouseX, MouseY))
    {
        return false;
    }

    if (targetControl->GetStorageType() != STORAGE_TYPE::INVENTORY)
    {
        return false;
    }

    ITEM* pItem = targetControl->FindItemAtPt(MouseX, MouseY);
    if (pItem == nullptr)
    {
        return false;
    }

    if (g_pNPCShop->IsSellingItem())
    {
        return false;
    }

    if (IsSellingBan(pItem))
    {
        g_pSystemLogBox->AddText(GlobalText[668], TYPE_ERROR_MESSAGE);
        return true;
    }

    const int iIndex = targetControl->GetIndexByItem(pItem);
    if (iIndex < 0)
    {
        return false;
    }

    if (!CNewUIInventoryCtrl::CreatePickedItem(targetControl, pItem))
    {
        return false;
    }

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem == nullptr)
    {
        return false;
    }

    targetControl->RemoveItem(pItem);
    pPickedItem->HidePickedItem();

    const int sourceIndex = pPickedItem->GetSourceLinealPos();
    if (sourceIndex < MAX_EQUIPMENT_INDEX || sourceIndex >= MAX_MY_INVENTORY_EX_INDEX)
    {
        CNewUIInventoryCtrl::BackupPickedItem();
        return false;
    }

    if (IsHighValueItem(pItem))
    {
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CHighValueItemCheckMsgBoxLayout));
        return true;
    }

    SocketClient->ToGameServer()->SendSellItemToNpcRequest(sourceIndex);
    g_pNPCShop->SetSellingItem(true);
    return true;
}

bool CNewUIInventoryActionController::HandleInventoryRightClickActions(CNewUIInventoryCtrl* targetControl) const
{
    if (g_pNewUISystem->IsVisible(INTERFACE_INVENTORY_EXT))
    {
        return TryTransferBetweenInventorySections(targetControl);
    }

    ITEM* pItem = targetControl->FindItemAtPt(MouseX, MouseY);
    if (pItem == nullptr)
    {
        return false;
    }

    const int iIndex = targetControl->GetIndexByItem(pItem);

    if (iIndex >= 0 && TryConsumeItem(targetControl, pItem, iIndex))
    {
        return true;
    }

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
    if (g_pMyInventory->IsInvenItem(pItem->Type))
    {
#ifdef LJH_FIX_APP_SHUTDOWN_WEQUIPPING_INVENITEM_WITH_CLICKING_MOUSELBTN
        if (MouseLButton || MouseLButtonPop || MouseLButtonPush)
        {
            return false;
        }
#endif
        if (pItem->Durability == 0)
        {
            return false;
        }

        int iChangeInvenItemStatus = 0;
        (pItem->Durability == 255) ? iChangeInvenItemStatus = 254 : iChangeInvenItemStatus = 255;
        SendRequestEquippingInventoryItem(iIndex, iChangeInvenItemStatus);
        return true;
    }
#endif

    if (!EquipmentItem)
    {
        if (TryEquipItem(targetControl, pItem, iIndex))
        {
            return true;
        }
    }

    if (TryDropItem(targetControl, pItem))
    {
        return true;
    }

    return false;
}

int CNewUIInventoryActionController::FindAlternateEquipSlot(int nOriginalSlot, ITEM* pItem) const
{
    if (nOriginalSlot == EQUIPMENT_WEAPON_RIGHT)
    {
        const auto baseClass = gCharacterManager.GetBaseClass(Hero->Class);
        if (baseClass == CLASS_KNIGHT || baseClass == CLASS_DARK || baseClass == CLASS_RAGEFIGHTER)
        {
            const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
            if (!pItemAttr->TwoHand)
            {
                return EQUIPMENT_WEAPON_LEFT;
            }
        }
    }
    else if (nOriginalSlot == EQUIPMENT_RING_RIGHT)
    {
        return EQUIPMENT_RING_LEFT;
    }

    return -1;
}

bool CNewUIInventoryActionController::IsSlotOccupied(int nSlot) const
{
    if (nSlot < 0 || nSlot >= MAX_EQUIPMENT_INDEX)
    {
        return true;
    }

    const ITEM* pEquipment = &CharacterMachine->Equipment[nSlot];
    return (pEquipment->Type != -1);
}

bool CNewUIInventoryActionController::TryEquipItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem, int iSrcIndex) const
{
    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
    int nDstIndex = pItemAttr->m_byItemSlot;

    if (nDstIndex < 0 || nDstIndex >= MAX_EQUIPMENT_INDEX)
    {
        return false;
    }

    if (!m_pContext->IsEquipable(nDstIndex, pItem))
    {
        return true;
    }

    if (IsSlotOccupied(nDstIndex))
    {
        const int nAltSlot = FindAlternateEquipSlot(nDstIndex, pItem);

        if (nAltSlot != -1 && !IsSlotOccupied(nAltSlot))
        {
            nDstIndex = nAltSlot;
        }
        else
        {
            return true;
        }
    }

    if (!m_pContext->IsEquipable(nDstIndex, pItem))
    {
        return true;
    }

    if (!CNewUIInventoryCtrl::CreatePickedItem(nullptr, pItem))
    {
        return false;
    }

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem == nullptr)
    {
        return false;
    }

    targetControl->RemoveItem(pItem);
    pPickedItem->HidePickedItem();

    SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSrcIndex, pItem, STORAGE_TYPE::INVENTORY, nDstIndex);
    return true;
}

bool CNewUIInventoryActionController::TryDropItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem) const
{
    if (Hero->Dead != 0)
    {
        return false;
    }

    if (IsHighValueItem(pItem))
    {
        g_pSystemLogBox->AddText(GlobalText[269], TYPE_ERROR_MESSAGE);
        return true;
    }

    if (IsDropBan(pItem))
    {
        g_pSystemLogBox->AddText(GlobalText[1915], TYPE_ERROR_MESSAGE);
        return true;
    }

    if (!CNewUIInventoryCtrl::CreatePickedItem(targetControl, pItem))
    {
        return false;
    }

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem == nullptr)
    {
        return false;
    }

    targetControl->RemoveItem(pItem);
    pPickedItem->HidePickedItem();

    const int tx = Hero->PositionX;
    const int ty = Hero->PositionY;
    const int sourceIndex = pPickedItem->GetSourceLinealPos();

    SocketClient->ToGameServer()->SendDropItemRequest(tx, ty, sourceIndex);
    SendDropItem = sourceIndex;

    return true;
}

bool CNewUIInventoryActionController::RepairItemAtMousePoint(CNewUIInventoryCtrl* targetControl) const
{
    ITEM* pItem = targetControl->FindItemAtPt(MouseX, MouseY);
    if (pItem == nullptr)
    {
        return true;
    }

    if (IsRepairBan(pItem))
    {
        return true;
    }

    const int iIndex = targetControl->GetIndex(pItem->x, pItem->y);

    if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) && g_pNPCShop->IsRepairShop())
    {
        SocketClient->ToGameServer()->SendRepairItemRequest(iIndex, 0);
    }
    else
    {
        SocketClient->ToGameServer()->SendRepairItemRequest(iIndex, 1);
    }

    return true;
}

bool CNewUIInventoryActionController::ApplyJewels(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
{
    const bool bIsJewelType =
        pPickItem->Type == ITEM_JEWEL_OF_BLESS
        || pPickItem->Type == ITEM_JEWEL_OF_SOUL
        || pPickItem->Type == ITEM_JEWEL_OF_LIFE
        || pPickItem->Type == ITEM_JEWEL_OF_HARMONY
        || pPickItem->Type == ITEM_LOWER_REFINE_STONE
        || pPickItem->Type == ITEM_HIGHER_REFINE_STONE
        || pPickItem->Type == ITEM_POTION + 160
        || pPickItem->Type == ITEM_POTION + 161;

    if (!bIsJewelType)
    {
        return false;
    }

    ITEM* pItem = targetControl->FindItem(iTargetIndex);
    if (!pItem)
    {
        return false;
    }

    const int iType       = pItem->Type;
    const int iLevel      = pItem->Level;
    const int iDurability = pItem->Durability;

    bool bSuccess = true;

    if (iType > ITEM_WINGS_OF_DARKNESS
        && iType != ITEM_CAPE_OF_LORD
        && !(iType >= ITEM_WING_OF_STORM && iType <= ITEM_WING_OF_DIMENSION)
        && !(ITEM_WING + 130 <= iType && iType <= ITEM_WING + 134)
        && !(iType >= ITEM_CAPE_OF_FIGHTER && iType <= ITEM_CAPE_OF_OVERRULE)
        && (iType != ITEM_WING + 135))
    {
        bSuccess = false;
    }

    if (iType == ITEM_BOLT || iType == ITEM_ARROWS)
    {
        bSuccess = false;
    }

    if ((pPickItem->Type == ITEM_JEWEL_OF_BLESS && iLevel >= 6)
        || (pPickItem->Type == ITEM_JEWEL_OF_SOUL && iLevel >= 9))
    {
        bSuccess = false;
    }

    if (pPickItem->Type == ITEM_JEWEL_OF_BLESS
        && iType == ITEM_HORN_OF_FENRIR
        && iDurability != 255)
    {
        CFenrirRepairMsgBox* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CFenrirRepairMsgBoxLayout), &pMsgBox);
        pMsgBox->SetSourceIndex(iSourceIndex);

        const int iIndex = targetControl->GetIndex(pItem->x, pItem->y);
        pMsgBox->SetTargetIndex(iIndex);

        pPickedItem->HidePickedItem();
        return true;
    }

    if (pPickItem->Type == ITEM_JEWEL_OF_HARMONY)
    {
        if (g_SocketItemMgr.IsSocketItem(pItem))
        {
            bSuccess = false;
        }
        else if (pItem->Jewel_Of_Harmony_Option != 0)
        {
            bSuccess = false;
        }
        else
        {
            const StrengthenItem strengthitem =
                g_pUIJewelHarmonyinfo->GetItemType(static_cast<int>(pItem->Type));

            if (strengthitem == SI_None)
            {
                bSuccess = false;
            }
        }
    }

    if (pPickItem->Type == ITEM_LOWER_REFINE_STONE
        || pPickItem->Type == ITEM_HIGHER_REFINE_STONE)
    {
        if (g_SocketItemMgr.IsSocketItem(pItem))
        {
            bSuccess = false;
        }
        else if (pItem->Jewel_Of_Harmony_Option == 0)
        {
            bSuccess = false;
        }
    }

    if (Check_LuckyItem(pItem->Type))
    {
        bSuccess = false;
        if (pPickItem->Type == ITEM_POTION + 161)
        {
            if (pItem->Jewel_Of_Harmony_Option == 0) bSuccess = true;
        }
        else if (pPickItem->Type == ITEM_POTION + 160)
        {
            if (pItem->Durability > 0)               bSuccess = true;
        }
    }

    if (bSuccess)
    {
        const int targetIndex = targetControl->GetIndexByItem(pItem);
        SendRequestUse(iSourceIndex, targetIndex);
        PlayBuffer(SOUND_GET_ITEM01);
        return true;
    }

    return false;
}

bool CNewUIInventoryActionController::TryStackItems(CNewUIInventoryCtrl* targetControl, ITEM* pPickItem, int iSourceIndex, int iTargetIndex) const
{
    if (ITEM* pItem = targetControl->FindItem(iTargetIndex))
    {
        if (targetControl->AreItemsStackable(pPickItem, pItem))
        {
            SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex,
                pPickItem, STORAGE_TYPE::INVENTORY, iTargetIndex);
            return true;
        }
    }

    return false;
}

bool CNewUIInventoryActionController::TryConsumeItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem, int iIndex) const
{
    if (pItem == nullptr)
    {
        return false;
    }

    if (pItem->Type == ITEM_TOWN_PORTAL_SCROLL)
    {
        SendRequestUse(iIndex, 0);
        return true;
    }

    const auto isApple  = pItem->Type == ITEM_APPLE;
    const auto isPotion =
        (pItem->Type >= ITEM_APPLE && pItem->Type <= ITEM_ALE)
        || (pItem->Type >= ITEM_SMALL_SHIELD_POTION && pItem->Type <= ITEM_LARGE_COMPLEX_POTION);

    if (isApple || isPotion
        || (pItem->Type == ITEM_POTION + 20 && pItem->Level == 0)
        || (pItem->Type >= ITEM_JACK_OLANTERN_BLESSINGS && pItem->Type <= ITEM_JACK_OLANTERN_DRINK)
        || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 14)
        || (pItem->Type >= ITEM_POTION + 70 && pItem->Type <= ITEM_POTION + 71)
        || (pItem->Type >= ITEM_POTION + 72 && pItem->Type <= ITEM_POTION + 77)
        || pItem->Type == ITEM_HELPER + 60
        || pItem->Type == ITEM_POTION + 94
        || (pItem->Type >= ITEM_CHERRY_BLOSSOM_WINE && pItem->Type <= ITEM_CHERRY_BLOSSOM_FLOWER_PETAL)
        || (pItem->Type >= ITEM_POTION + 97 && pItem->Type <= ITEM_POTION + 98)
        || pItem->Type == ITEM_HELPER + 81
        || pItem->Type == ITEM_HELPER + 82
        || pItem->Type == ITEM_POTION + 133)
    {
        SendRequestUse(iIndex, 0);
        if (isApple)
        {
            PlayBuffer(SOUND_EAT_APPLE01);
        }
        else if (isPotion)
        {
            PlayBuffer(SOUND_DRINK01);
        }

        return true;
    }

    if (pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82)
    {
        std::list<eBuffState> secretPotionbufflist;
        secretPotionbufflist.push_back(eBuff_SecretPotion1);
        secretPotionbufflist.push_back(eBuff_SecretPotion2);
        secretPotionbufflist.push_back(eBuff_SecretPotion3);
        secretPotionbufflist.push_back(eBuff_SecretPotion4);
        secretPotionbufflist.push_back(eBuff_SecretPotion5);

        if (g_isCharacterBufflist((&Hero->Object), secretPotionbufflist) == eBuffNone)
        {
            SendRequestUse(iIndex, 0);
            return true;
        }

        CreateOkMessageBox(GlobalText[2530], RGBA(255, 30, 0, 255));
        return false;
    }

    if ((pItem->Type >= ITEM_HELPER + 54 && pItem->Type <= ITEM_HELPER + 57)
        || (pItem->Type == ITEM_HELPER + 58
            && gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD))
    {
        WORD point[5] = { 0, };
        point[0] = CharacterAttribute->Strength  + CharacterAttribute->AddStrength;
        point[1] = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        point[2] = CharacterAttribute->Vitality  + CharacterAttribute->AddVitality;
        point[3] = CharacterAttribute->Energy    + CharacterAttribute->AddEnergy;
        point[4] = CharacterAttribute->Charisma  + CharacterAttribute->AddCharisma;

        const unsigned char nStat[MAX_CLASS][5] =
        {
            18, 18, 15, 30,  0,
            28, 20, 25, 10,  0,
            22, 25, 20, 15,  0,
            26, 26, 26, 26,  0,
            26, 20, 20, 15, 25,
            21, 21, 18, 23,  0,
            32, 27, 25, 20,  0,
        };

        const int attributeType   = pItem->Type - (ITEM_HELPER + 54);
        const int characterClass  = gCharacterManager.GetBaseClass(Hero->Class);
        point[attributeType]     -= nStat[characterClass][attributeType];

        if (point[attributeType] < (pItem->Durability * 10))
        {
            g_pMyInventory->SetStandbyItemKey(pItem->Key);
            CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUsePartChargeFruitMsgBoxLayout));
            return false;
        }

        SendRequestUse(iIndex, 0);
        return true;
    }

    if (pItem->Type == ITEM_HELPER + 58
        && gCharacterManager.GetBaseClass(Hero->Class) != CLASS_DARK_LORD)
    {
        CreateOkMessageBox(GlobalText[1905]);
        return true;
    }

    if (pItem->Type == ITEM_ARMOR_OF_GUARDSMAN)
    {
        if (IsUnitedMarketPlace())
        {
            wchar_t szOutputText[512];
            mu_swprintf(szOutputText, L"%ls %ls", GlobalText[3014], GlobalText[3015]);
            CreateOkMessageBox(szOutputText);
            return true;
        }

        if (Hero->SafeZone == false)
        {
            CreateOkMessageBox(GlobalText[2330]);
            return false;
        }

        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::ChaosCastle, pItem->Level);
        g_pMyInventory->SetStandbyItemKey(pItem->Key);
        return true;
    }

    if (pItem->Type == ITEM_HELPER + 46)
    {
        const BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_DEVILSQUARE);
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::DevilSquare, byPossibleLevel);
        return false;
    }

    if (pItem->Type == ITEM_HELPER + 47)
    {
        const BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_BLOODCASTLE);
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::BloodCastle, byPossibleLevel);
        return false;
    }

    if (pItem->Type == ITEM_HELPER + 48)
    {
        if (Hero->SafeZone || gMapManager.InHellas())
        {
            g_pSystemLogBox->AddText(GlobalText[1238], TYPE_ERROR_MESSAGE);
            return false;
        }

        SendRequestUse(iIndex, 0);
        return true;
    }

    if (pItem->Type == ITEM_HELPER + 61)
    {
        const BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_CURSEDTEMPLE);
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::CursedTemple, byPossibleLevel);
        return true;
    }

    if (pItem->Type == ITEM_HELPER + 121)
    {
        if (Hero->SafeZone == false)
        {
            CreateOkMessageBox(GlobalText[2330]);
            return false;
        }

        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::ChaosCastle, pItem->Level);
        g_pMyInventory->SetStandbyItemKey(pItem->Key);
        return true;
    }

    if (pItem->Type == ITEM_SCROLL_OF_BLOOD)
    {
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::CursedTemple, pItem->Level);
        return true;
    }

    if (pItem->Type == ITEM_DEVILS_INVITATION)
    {
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::DevilSquare, pItem->Level);
        return true;
    }

    if (pItem->Type == ITEM_INVISIBILITY_CLOAK)
    {
        if (pItem->Level == 0)
        {
            g_pSystemLogBox->AddText(GlobalText[2089], TYPE_ERROR_MESSAGE);
        }
        else
        {
            SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(MiniGameType::BloodCastle, pItem->Level - 1);
        }

        return true;
    }

    if ((pItem->Type >= ITEM_SCROLL_OF_POISON && pItem->Type < ITEM_ETC + MAX_ITEM_INDEX)
        || (pItem->Type >= ITEM_ORB_OF_TWISTING_SLASH && pItem->Type <= ITEM_ORB_OF_GREATER_FORTITUDE)
        || (pItem->Type >= ITEM_ORB_OF_FIRE_SLASH && pItem->Type <= ITEM_ORB_OF_DEATH_STAB)
        || (pItem->Type == ITEM_WING + 20)
        || (pItem->Type >= ITEM_SCROLL_OF_FIREBURST && pItem->Type <= ITEM_SCROLL_OF_ELECTRIC_SPARK)
        || (pItem->Type == ITEM_SCROLL_OF_FIRE_SCREAM)
        || (pItem->Type == ITEM_CRYSTAL_OF_DESTRUCTION)
        || (pItem->Type == ITEM_CRYSTAL_OF_FLAME_STRIKE)
        || (pItem->Type == ITEM_CRYSTAL_OF_RECOVERY)
        || (pItem->Type == ITEM_CRYSTAL_OF_MULTI_SHOT)
        || (pItem->Type == ITEM_SCROLL_OF_CHAOTIC_DISEIER)
        || (pItem->Type == ITEM_SCROLL_OF_GIGANTIC_STORM)
        || (pItem->Type == ITEM_SCROLL_OF_WIZARDRY_ENHANCE))
    {
        bool bReadBookGem = true;

        if (pItem->Type == ITEM_SCROLL_OF_NOVA
            || pItem->Type == ITEM_SCROLL_OF_WIZARDRY_ENHANCE
            || pItem->Type == ITEM_CRYSTAL_OF_MULTI_SHOT
            || pItem->Type == ITEM_CRYSTAL_OF_RECOVERY
            || pItem->Type == ITEM_CRYSTAL_OF_DESTRUCTION)
        {
            if (g_csQuest.getQuestState2(QUEST_CHANGE_UP_3) != QUEST_END)
            {
                bReadBookGem = false;
            }
        }

        if (pItem->Type == ITEM_SCROLL_OF_CHAOTIC_DISEIER)
        {
            if (CharacterAttribute->Level < 220)
            {
                bReadBookGem = false;
            }
        }

        if (bReadBookGem)
        {
            const WORD wStrength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
            const WORD wEnergy   = CharacterAttribute->Energy   + CharacterAttribute->AddEnergy;

            if (CharacterAttribute->Level >= ItemAttribute[pItem->Type].RequireLevel
                && wEnergy   >= pItem->RequireEnergy
                && wStrength >= pItem->RequireStrength)
            {
                SendRequestUse(iIndex, 0);
            }

            return true;
        }

        return false;
    }

    if (pItem->Type == ITEM_FRUITS)
    {
        if (CharacterAttribute->Level < 10)
        {
            CreateOkMessageBox(GlobalText[749]);
            return true;
        }

        bool bEquipmentEmpty = true;
        for (int i = 0; i < MAX_EQUIPMENT; i++)
        {
            if (CharacterMachine->Equipment[i].Type != -1)
            {
                bEquipmentEmpty = false;
                break;
            }
        }

        if (!bEquipmentEmpty)
        {
            CreateOkMessageBox(GlobalText[1909]);
            return true;
        }

        if (pItem->Level == 4) // Command Fruit
        {
            if (gCharacterManager.GetBaseClass(CharacterAttribute->Class) != CLASS_DARK_LORD)
            {
                CreateOkMessageBox(GlobalText[1905]);
                return true;
            }
        }

        g_pMyInventory->SetStandbyItemKey(pItem->Key);
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseFruitMsgBoxLayout));
        return true;
    }

    if (pItem->Type == ITEM_LIFE_STONE_ITEM)
    {
        bool bUse = false;
        switch (pItem->Level)
        {
        case 0: bUse = true; break;
        case 1: if (Hero->GuildStatus != G_MASTER) bUse = true; break;
        }

        if (bUse)
        {
            SendRequestUse(iIndex, 0);
            return true;
        }

        return false;
    }

    if (pItem->Type == ITEM_HELPER + 69)
    {
        if (g_PortalMgr.IsRevivePositionSaved())
        {
            if (g_PortalMgr.IsPortalUsable())
            {
                g_pMyInventory->SetStandbyItemKey(pItem->Key);
                CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseReviveCharmMsgBoxLayout));
            }
            else
            {
                CreateOkMessageBox(GlobalText[2608]);
            }
        }

        return false;
    }

    if (pItem->Type == ITEM_HELPER + 70)
    {
        if (g_PortalMgr.IsPortalUsable())
        {
            if (pItem->Durability == 2)
            {
                if (g_PortalMgr.IsPortalPositionSaved())
                {
                    CreateOkMessageBox(GlobalText[2610]);
                }
                else
                {
                    g_pMyInventory->SetStandbyItemKey(pItem->Key);
                    CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUsePortalCharmMsgBoxLayout));
                }
            }
            else if (pItem->Durability == 1)
            {
                g_pMyInventory->SetStandbyItemKey(pItem->Key);
                CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CReturnPortalCharmMsgBoxLayout));
            }
        }
        else
        {
            CreateOkMessageBox(GlobalText[2608]);
        }

        return false;
    }

    if (pItem->Type == ITEM_HELPER + 66)
    {
        g_pMyInventory->SetStandbyItemKey(pItem->Key);
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseSantaInvitationMsgBoxLayout));
    }

    return false;
}

bool CNewUIInventoryActionController::TryTransferBetweenInventorySections(CNewUIInventoryCtrl* sourceControl) const
{
    if (sourceControl == nullptr || g_pMyInventoryExt == nullptr)
    {
        return false;
    }

    ITEM* pItem = sourceControl->FindItemAtPt(MouseX, MouseY);
    if (pItem == nullptr)
    {
        return false;
    }

    const int sourceIndex = sourceControl->GetIndexByItem(pItem);
    if (sourceIndex < 0)
    {
        return false;
    }

    const ITEM_ATTRIBUTE* itemAttribute = &ItemAttribute[pItem->Type];
    int destinationIndex = -1;

    if (sourceControl == g_pMyInventory->GetInventoryCtrl())
    {
        destinationIndex = g_pMyInventoryExt->FindEmptySlot(
            itemAttribute->Width, itemAttribute->Height);
    }
    else
    {
        destinationIndex = m_pContext->FindEmptySlot(pItem);
    }

    if (destinationIndex == -1 || destinationIndex == sourceIndex)
    {
        return true;
    }

    if (!CNewUIInventoryCtrl::CreatePickedItem(sourceControl, pItem))
    {
        return false;
    }

    sourceControl->RemoveItem(pItem);

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem == nullptr || pPickedItem->GetItem() == nullptr)
    {
        CNewUIInventoryCtrl::BackupPickedItem();
        return false;
    }

    pPickedItem->HidePickedItem();

    if (!SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, sourceIndex,
        pPickedItem->GetItem(), STORAGE_TYPE::INVENTORY, destinationIndex))
    {
        CNewUIInventoryCtrl::BackupPickedItem();
        return false;
    }

    return true;
}

} // namespace SEASON3B
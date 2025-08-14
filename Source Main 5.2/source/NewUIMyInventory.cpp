// NewUIMyInventory.cpp: implementation of the CNewUIMyInventory class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMyInventory.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "GOBoid.h"
#include "ZzzEffect.h"
#include "GIPetManager.h"
#include "CSParts.h"
#include "UIJewelHarmony.h"
#include "CDirection.h"
#include "ZzzInventory.h"
#include "ZzzLodTerrain.h"
#include "CSQuest.h"
#include "UIGuildInfo.h"
#include "UIManager.h"
#include "CSItemOption.h"
#include "MapManager.h"
#include "w_PetProcess.h"
#include "SocketSystem.h"
#include "PortalMgr.h"
#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#include "Event.h"
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#include "ChangeRingManager.h"
#include "MonkSystem.h"
#include "CharacterManager.h"
#include "DSPlaySound.h"
#include "ZzzInterface.h"

using namespace SEASON3B;

CNewUIMyInventory::CNewUIMyInventory()
{
    m_pNewUIMng = nullptr;
    m_pNewUI3DRenderMng = nullptr;
    m_pNewInventoryCtrl = nullptr;
    m_Pos.x = m_Pos.y = 0;

    memset(&m_EquipmentSlots, 0, sizeof(EQUIPMENT_ITEM) * MAX_EQUIPMENT_INDEX);
    m_iPointedSlot = -1;

    m_MyShopMode = MYSHOP_MODE_OPEN;
    m_RepairMode = REPAIR_MODE_OFF;
    m_dwStandbyItemKey = 0;

    m_bRepairEnableLevel = false;
    m_bMyShopOpen = false;
}

CNewUIMyInventory::~CNewUIMyInventory()
{
    Release();
}

bool CNewUIMyInventory::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
    if (nullptr == pNewUIMng || nullptr == pNewUI3DRenderMng || nullptr == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_INVENTORY, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

    m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::INVENTORY, m_pNewUI3DRenderMng, g_pNewItemMng, this, x + 15, y + 200, 8, 8, MAX_EQUIPMENT))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    SetPos(x, y);
    LoadImages();
    SetEquipmentSlotInfo();
    SetButtonInfo();
    Show(false);
    return true;
}

void CNewUIMyInventory::Release()
{
    if (m_pNewUI3DRenderMng)
        m_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);

    UnequipAllItems();
    DeleteAllItems();

    UnloadImages();

    SAFE_DELETE(m_pNewInventoryCtrl);

    if (m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->Remove3DRenderObj(this);
        m_pNewUI3DRenderMng = nullptr;
    }
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }
}

bool CNewUIMyInventory::EquipItem(int iIndex, std::span<const BYTE> pbyItemPacket)
{
    if (iIndex < 0 || iIndex >= MAX_EQUIPMENT_INDEX || !g_pNewItemMng || !CharacterMachine)
    {
        return false;
    }

    ITEM* pTargetItemSlot = &CharacterMachine->Equipment[iIndex];
    if (pTargetItemSlot->Type > 0)
    {
        UnequipItem(iIndex);
    }

    ITEM* pTempItem = g_pNewItemMng->CreateItem(pbyItemPacket);

    if (nullptr == pTempItem)
    //if(NULL == pTempItem || false == IsEquipable(iIndex, pTempItem))
    {
        return false;
    }

    if (pTempItem->Type == ITEM_DARK_HORSE_ITEM)
    {
        SocketClient->ToGameServer()->SendPetInfoRequest(PET_TYPE_DARK_HORSE, 0, iIndex);
    }

    if (pTempItem->Type == ITEM_DARK_RAVEN_ITEM)
    {
        CreatePetDarkSpirit(Hero);
        SocketClient->ToGameServer()->SendPetInfoRequest(PET_TYPE_DARK_SPIRIT, 0, iIndex);
    }

    pTempItem->lineal_pos = iIndex;
    pTempItem->ex_src_type = ITEM_EX_SRC_EQUIPMENT;
    memcpy(pTargetItemSlot, pTempItem, sizeof(ITEM));
    g_pNewItemMng->DeleteItem(pTempItem);

    CreateEquippingEffect(pTargetItemSlot);
    
    return true;
}
void CNewUIMyInventory::UnequipItem(int iIndex)
{
    if (iIndex >= 0 && iIndex < MAX_EQUIPMENT_INDEX && g_pNewItemMng && CharacterMachine)
    {
        ITEM* pEquippedItem = &CharacterMachine->Equipment[iIndex];

        if (pEquippedItem && pEquippedItem->Type != -1)
        {
            if (pEquippedItem->Type == ITEM_DARK_HORSE_ITEM)
            {
                Hero->InitPetInfo(PET_TYPE_DARK_HORSE);
            }
            else if (pEquippedItem->Type == ITEM_DARK_RAVEN_ITEM)
            {
                DeletePet(Hero);
                Hero->InitPetInfo(PET_TYPE_DARK_SPIRIT);
            }

            if (pEquippedItem->Type != ITEM_DARK_RAVEN_ITEM)
                DeleteEquippingEffectBug(pEquippedItem);

            pEquippedItem->Type = -1;
            pEquippedItem->Level = 0;
            pEquippedItem->Number = -1;
            pEquippedItem->ExcellentFlags = 0;
            pEquippedItem->Durability = 0;
            pEquippedItem->AncientDiscriminator = 0;
            pEquippedItem->AncientBonusOption = 0;
            pEquippedItem->SocketCount = 0;
            for (int i = 0; i < MAX_SOCKETS; ++i)
            {
                pEquippedItem->SocketSeedID[i] = SOCKET_EMPTY;
                pEquippedItem->SocketSphereLv[i] = 0;
            }
            pEquippedItem->SocketSeedSetOption = 0;
            DeleteEquippingEffect();
        }
    }
}
void CNewUIMyInventory::UnequipAllItems()
{
    if (CharacterMachine)
    {
        for (int i = 0; i < MAX_EQUIPMENT_INDEX; i++)
        {
            UnequipItem(i);
        }
    }
}

bool CNewUIMyInventory::IsEquipable(int iIndex, ITEM* pItem)
{
    if (pItem == nullptr)
        return false;

    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
    bool bEquipable = false;
    if (pItemAttr->RequireClass[gCharacterManager.GetBaseClass(Hero->Class)])
        bEquipable = true;

    else if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK && pItemAttr->RequireClass[CLASS_WIZARD]
        && pItemAttr->RequireClass[CLASS_KNIGHT])
        bEquipable = true;

    const BYTE byFirstClass = gCharacterManager.GetBaseClass(Hero->Class);
    const BYTE byStepClass = gCharacterManager.GetStepClass(Hero->Class);
    if (pItemAttr->RequireClass[byFirstClass] > byStepClass)
    {
        return false;
    }

    if (bEquipable == false)
        return false;

    bEquipable = false;
    if (pItemAttr->m_byItemSlot == iIndex)
        bEquipable = true;

    else if (pItemAttr->m_byItemSlot == EQUIPMENT_WEAPON_RIGHT && iIndex == EQUIPMENT_WEAPON_LEFT)
    {
        if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK
            || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
        {
            if (!pItemAttr->TwoHand)
                bEquipable = true;
#ifdef PBG_FIX_EQUIP_TWOHANDSWORD
            else
            {
                bEquipable = false;
                return false;
            }
#endif //PBG_FIX_EQUIP_TWOHANDSWORD
        }
        else if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_SUMMONER &&
            !(pItem->Type >= ITEM_STAFF && pItem->Type <= ITEM_STAFF + MAX_ITEM_INDEX))
            bEquipable = true;
    }
    else if (pItemAttr->m_byItemSlot == EQUIPMENT_RING_RIGHT && iIndex == EQUIPMENT_RING_LEFT)
        bEquipable = true;

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
    {
        //ITEM *r = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
        const ITEM* l = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
        if (iIndex == EQUIPMENT_WEAPON_RIGHT && l->Type != ITEM_BOLT
            && (l->Type >= ITEM_BOW && l->Type < ITEM_BOW + MAX_ITEM_INDEX))
        {
            if (pItem->Type != ITEM_ARROWS)
                bEquipable = false;
        }
    }

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
    {
        if (iIndex == EQUIPMENT_GLOVES)
            bEquipable = false;
        else if (pItemAttr->m_byItemSlot == EQUIPMENT_WEAPON_RIGHT)
            bEquipable = g_CMonkSystem.RageEquipmentWeapon(iIndex, pItem->Type);
    }

    if (bEquipable == false)
        return false;

    const WORD wStrength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
    const WORD wDexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
    const WORD wEnergy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
    const WORD wVitality = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
    const WORD wCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
    const WORD wLevel = CharacterAttribute->Level;

    const int iItemLevel = pItem->Level;

    int iDecNeedStrength = 0, iDecNeedDex = 0;

    extern JewelHarmonyInfo* g_pUIJewelHarmonyinfo;
    if (iItemLevel >= pItem->Jewel_Of_Harmony_OptionLevel)
    {
        StrengthenCapability SC;
        g_pUIJewelHarmonyinfo->GetStrengthenCapability(&SC, pItem, 0);

        if (SC.SI_isNB)
        {
            iDecNeedStrength = SC.SI_NB.SI_force;
            iDecNeedDex = SC.SI_NB.SI_activity;
        }
    }
    if (pItem->SocketCount > 0)
    {
        for (int i = 0; i < pItem->SocketCount; ++i)
        {
            if (pItem->SocketSeedID[i] == 38)
            {
                const int iReqStrengthDown = g_SocketItemMgr.GetSocketOptionValue(pItem, i);
                iDecNeedStrength += iReqStrengthDown;
            }
            else if (pItem->SocketSeedID[i] == 39)
            {
                const int iReqDexterityDown = g_SocketItemMgr.GetSocketOptionValue(pItem, i);
                iDecNeedDex += iReqDexterityDown;
            }
        }
    }

    if (pItem->RequireStrength - iDecNeedStrength > wStrength)
        return false;
    if (pItem->RequireDexterity - iDecNeedDex > wDexterity)
        return false;
    if (pItem->RequireEnergy > wEnergy)
        return false;
    if (pItem->RequireVitality > wVitality)
        return false;
    if (pItem->RequireCharisma > wCharisma)
        return false;
    if (pItem->RequireLevel > wLevel)
        return false;

    if (pItem->Type == ITEM_DARK_RAVEN_ITEM)
    {
        const auto pPetInfo = GetPetInfo(pItem);
        if (pPetInfo->m_dwPetType == PET_TYPE_NONE)
        {
            return false;
        }

        const auto requiredCharisma = (185 + (pPetInfo->m_wLevel * 15));
        if (requiredCharisma > wCharisma)
        {
            return false;
        }
    }

    if (gMapManager.WorldActive == WD_7ATLANSE && (pItem->Type >= ITEM_HORN_OF_UNIRIA && pItem->Type <= ITEM_HORN_OF_DINORANT))
    {
        return false;
    }
    if (pItem->Type == ITEM_HORN_OF_UNIRIA && gMapManager.WorldActive == WD_10HEAVEN)
    {
        return false;
    }
    if (pItem->Type == ITEM_HORN_OF_UNIRIA && g_Direction.m_CKanturu.IsMayaScene())
    {
        return false;
    }
    if (gMapManager.InChaosCastle() || (Get_State_Only_Elf()
        && g_isCharacterBuff((&Hero->Object), eBuff_CrywolfHeroContracted)))
    {
        if ((pItem->Type >= ITEM_HORN_OF_UNIRIA && pItem->Type <= ITEM_DARK_RAVEN_ITEM) || pItem->Type == ITEM_HORN_OF_FENRIR)
            return false;
    }
    else if ((pItem->Type >= ITEM_HORN_OF_UNIRIA && pItem->Type <= ITEM_DARK_HORSE_ITEM || pItem->Type == ITEM_HORN_OF_FENRIR)
        && Hero->Object.CurrentAction >= PLAYER_SIT1 && Hero->Object.CurrentAction <= PLAYER_SIT_FEMALE2)
    {
        return false;
    }

    return bEquipable;
}

bool CNewUIMyInventory::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket) const
{
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
    }

    return false;
}
void CNewUIMyInventory::DeleteItem(int iIndex) const
{
    if (m_pNewInventoryCtrl)
    {
        ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);
        if (pItem != nullptr)
        {
            m_pNewInventoryCtrl->RemoveItem(pItem);
        }
        else
        {
            CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
            if (pPickedItem)
            {
                if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
                {
                    if (pPickedItem->GetSourceLinealPos() == iIndex)
                    {
                        CNewUIInventoryCtrl::DeletePickedItem();
                    }
                }
            }
        }
    }
}
void CNewUIMyInventory::DeleteAllItems() const
{
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->RemoveAllItems();
}

void CNewUIMyInventory::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    SetEquipmentSlotInfo();

    m_pNewInventoryCtrl->SetPos(x + 15, y + 200);
    m_BtnExit.SetPos(m_Pos.x + 13, m_Pos.y + 391);
    m_BtnRepair.SetPos(m_Pos.x + 50, m_Pos.y + 391);
    m_BtnMyShop.SetPos(m_Pos.x + 87, m_Pos.y + 391);
    m_BtnExpand.SetPos(m_Pos.x + 87 + 37, m_Pos.y + 391);
}

const POINT& CNewUIMyInventory::GetPos() const
{
    return m_Pos;
}

CNewUIMyInventory::REPAIR_MODE CNewUIMyInventory::GetRepairMode() const
{
    return m_RepairMode;
}

void CNewUIMyInventory::SetRepairMode(bool bRepair)
{
    if (bRepair)
    {
        m_RepairMode = REPAIR_MODE_ON;
        if (m_pNewInventoryCtrl)
        {
            m_pNewInventoryCtrl->SetRepairMode(true);
        }
    }
    else
    {
        m_RepairMode = REPAIR_MODE_OFF;
        if (m_pNewInventoryCtrl)
        {
            m_pNewInventoryCtrl->SetRepairMode(false);
        }
    }
}

bool CNewUIMyInventory::UpdateMouseEvent()
{
    if (m_pNewInventoryCtrl && !m_pNewInventoryCtrl->UpdateMouseEvent())
        return false;

    if (true == EquipmentWindowProcess())
        return false;
    if (true == InventoryProcess())
        return false;

    if (true == BtnProcess())
        return false;

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem && IsPress(VK_LBUTTON) && CheckMouseIn(0, 0, GetScreenWidth(), 429))
    {
        if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) == true
            || g_pNewUISystem->IsVisible(INTERFACE_TRADE) == true
            || g_pNewUISystem->IsVisible(INTERFACE_DEVILSQUARE) == true
            || g_pNewUISystem->IsVisible(INTERFACE_BLOODCASTLE) == true
            || g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY) == true
            || g_pNewUISystem->IsVisible(INTERFACE_STORAGE) == true
            || g_pNewUISystem->IsVisible(INTERFACE_MYSHOP_INVENTORY) == true
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) == true
            || g_pNewUISystem->IsVisible(INTERFACE_PURCHASESHOP_INVENTORY) == true)
        {
            ResetMouseLButton();
            return false;
        }

        ITEM* pItemObj = pPickedItem->GetItem();
        if (pItemObj && pItemObj->Jewel_Of_Harmony_Option != 0)
        {
            g_pSystemLogBox->AddText(GlobalText[2217], TYPE_ERROR_MESSAGE);

            ResetMouseLButton();
            return false;
        }
        if (pItemObj && IsHighValueItem(pItemObj) == true)
        {
            g_pSystemLogBox->AddText(GlobalText[269], TYPE_ERROR_MESSAGE);
            CNewUIInventoryCtrl::BackupPickedItem();

            ResetMouseLButton();
            return false;
        }
        if (pItemObj && IsDropBan(pItemObj))
        {
            g_pSystemLogBox->AddText(GlobalText[1915], TYPE_ERROR_MESSAGE);
            CNewUIInventoryCtrl::BackupPickedItem();

            ResetMouseLButton();
            return false;
        }
        if (pItemObj && pItemObj->Type == ITEM_LOST_MAP && gMapManager.IsCursedTemple() == true)
        {
            ResetMouseLButton();
            return false;
        }
        RenderTerrain(true);
        if (RenderTerrainTile(SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, 1.f, 1, true))
        {
            const int iSourceIndex = pPickedItem->GetSourceLinealPos();
            const int tx = (int)(CollisionPosition[0] / TERRAIN_SCALE);
            const int ty = (int)(CollisionPosition[1] / TERRAIN_SCALE);
            if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
            {
                if (Hero->Dead == 0)
                {
                    SocketClient->ToGameServer()->SendDropItemRequest(tx, ty, iSourceIndex);
                    SendDropItem = iSourceIndex;
                }
            }
            else if (pItemObj && pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
            {
                SocketClient->ToGameServer()->SendDropItemRequest(tx, ty, iSourceIndex);
                SendDropItem = iSourceIndex;
            }
            MouseUpdateTime = 0;
            MouseUpdateTimeMax = 6;

            ResetMouseLButton();
            return false;
        }
    }

    g_csItemOption.SetViewOptionList(false);

    if (CheckMouseIn(m_Pos.x, m_Pos.y + 20, INVENTORY_WIDTH * 0.5f, 15) == true)
    {
        g_csItemOption.SetViewOptionList(true);
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
    {
        if (IsPress(VK_RBUTTON))
        {
            ResetMouseRButton();
            return false;
        }

        if (IsNone(VK_LBUTTON) == false)
        {
            return false;
        }
    }

    return true;
}

bool CNewUIMyInventory::UpdateKeyEvent()
{
    if (!g_pNewUISystem->IsVisible(INTERFACE_INVENTORY))
    {
        return true;
    }

    if (IsPress(VK_ESCAPE) == true)
    {
        if (g_pNPCShop->IsSellingItem() == false)
        {
            g_pNewUISystem->Hide(INTERFACE_INVENTORY);
            PlayBuffer(SOUND_CLICK01);
        }
        return false;
    }

    if (IsPress('L') == true)
    {
        if (m_bRepairEnableLevel == true && g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) == false
            && g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY) == false
            && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) == false
            )
        {
            ToggleRepairMode();

            return false;
        }
    }

    if (CanOpenMyShopInterface() == true && IsPress('S'))
    {
        if (m_bMyShopOpen)
        {
            if (m_MyShopMode == MYSHOP_MODE_OPEN)
            {
                ChangeMyShopButtonStateClose();
            }
            else if (m_MyShopMode == MYSHOP_MODE_CLOSE)
            {
                ChangeMyShopButtonStateOpen();
            }
            g_pNewUISystem->Toggle(INTERFACE_MYSHOP_INVENTORY);
            PlayBuffer(SOUND_CLICK01);
        }
        return false;
    }

    if (IsPress('K'))
    {
        g_pNewUISystem->Toggle(INTERFACE_INVENTORY_EXT);
        PlayBuffer(SOUND_CLICK01);

        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT) == false)
    {
        return true;
    }

    if (IsRepeat(VK_CONTROL))
    {
        int iHotKey = -1;
        if (IsPress('Q'))
        {
            iHotKey = HOTKEY_Q;
        }
        else if (IsPress('W'))
        {
            iHotKey = HOTKEY_W;
        }
        else if (IsPress('E'))
        {
            iHotKey = HOTKEY_E;
        }
        else if (IsPress('R'))
        {
            iHotKey = HOTKEY_R;
        }

        if (iHotKey != -1)
        {
            const ITEM* pItem = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);
            if (pItem == nullptr)
            {
                return false;
            }

            if (CanRegisterItemHotKey(pItem->Type) == true)
            {
                const int iItemLevel = pItem->Level;
                g_pMainFrame->SetItemHotKey(iHotKey, pItem->Type, iItemLevel);
                return false;
            }
        }
    }

    return true;
}

bool CNewUIMyInventory::Update()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
    {
        return false;
    }

    if (IsVisible())
    {
        m_iPointedSlot = -1;
        for (int i = 0; i < MAX_EQUIPMENT_INDEX; i++)
        {
            if (CheckMouseIn(m_EquipmentSlots[i].x + 1, m_EquipmentSlots[i].y,
                m_EquipmentSlots[i].width - 4, m_EquipmentSlots[i].height - 4))
            {
                m_iPointedSlot = i;
                break;
            }
        }
    }
    return true;
}

bool CNewUIMyInventory::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderInventoryDetails();
    RenderSetOption();
    RenderSocketOption();
    RenderButtons();

    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->Render();

    RenderEquippedItem();
    DisableAlphaBlend();
    return true;
}

void CNewUIMyInventory::RenderSetOption()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    if (g_csItemOption.IsAncientSetEquipped())
    {
        g_pRenderText->SetTextColor(255, 204, 25, 255);
    }
    else
    {
        g_pRenderText->SetTextColor(100, 100, 100, 255);
    }

    wchar_t strText[128];
    swprintf(strText, L"[%s]", GlobalText[989]);
    g_pRenderText->RenderText(m_Pos.x + INVENTORY_WIDTH * 0.2f, m_Pos.y + 25, strText, INVENTORY_WIDTH * 0.3f, 0, RT3_SORT_CENTER);

    if (g_csItemOption.IsViewOptionList() == true)
    {
        m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, -1, ITEM_SET_OPTION);
    }
}

void CNewUIMyInventory::RenderSocketOption()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    if (g_SocketItemMgr.IsSocketSetOptionEnabled())
    {
        g_pRenderText->SetTextColor(255, 204, 25, 255);
    }
    else
    {
        g_pRenderText->SetTextColor(100, 100, 100, 255);
    }

    wchar_t strText[128];
    swprintf(strText, L"[%s]", GlobalText[2651]);
    g_pRenderText->RenderText(m_Pos.x + INVENTORY_WIDTH * 0.5f, m_Pos.y + 25, strText, INVENTORY_WIDTH * 0.3f, 0, RT3_SORT_CENTER);

    if (CheckMouseIn(m_Pos.x + INVENTORY_WIDTH * 0.5f, m_Pos.y + 20, INVENTORY_WIDTH * 0.5f, 15) == true)
    {
        m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, -1, ITEM_SOCKET_SET_OPTION);
    }
}

void CNewUIMyInventory::Render3D()
{
    for (int i = 0; i < MAX_EQUIPMENT_INDEX; i++)
    {
        const ITEM* pEquippedItem = &CharacterMachine->Equipment[i];
        if (pEquippedItem->Type >= 0)
        {
            float y = 0.f;
            if (i == EQUIPMENT_ARMOR)
            {
                y = m_EquipmentSlots[i].y - 10.f;
            }
            else
            {
                y = m_EquipmentSlots[i].y;
            }

            glColor4f(1.f, 1.f, 1.f, 1.f);
            RenderItem3D(
                m_EquipmentSlots[i].x + 1,
                y,
                m_EquipmentSlots[i].width - 4,
                m_EquipmentSlots[i].height - 4,
                pEquippedItem->Type,
                pEquippedItem->Level,
                pEquippedItem->ExcellentFlags,
                pEquippedItem->AncientDiscriminator,
                false);
        }
    }
}

bool CNewUIMyInventory::IsVisible() const
{
    return CNewUIObj::IsVisible();
}

void CNewUIMyInventory::OpenningProcess()
{
    SetRepairMode(false);

    m_MyShopMode = MYSHOP_MODE_OPEN;
    ChangeMyShopButtonStateOpen();

    const WORD wLevel = CharacterAttribute->Level;

    if (wLevel >= 50)
    {
        m_bRepairEnableLevel = true;
    }
    else
    {
        m_bRepairEnableLevel = false;
    }

    if (wLevel >= 6)
    {
        m_bMyShopOpen = true;
    }
    else
    {
        m_bMyShopOpen = false;
    }

    if (g_QuestMng.IsIndexInCurQuestIndexList(0x1000F))
    {
        if (g_QuestMng.IsEPRequestRewardState(0x1000F))
        {
            SocketClient->ToGameServer()->SendQuestClientActionRequest(1, 0x0F);
            g_QuestMng.SetEPRequestRewardState(0x1000F, false);
        }
    }
}

void CNewUIMyInventory::ClosingProcess()
{
    m_pNewInventoryCtrl->BackupPickedItem();
    RepairEnable = 0;
    SetRepairMode(false);
}

float CNewUIMyInventory::GetLayerDepth()
{
    return 4.2f;
}

CNewUIInventoryCtrl* CNewUIMyInventory::GetInventoryCtrl() const
{
    return m_pNewInventoryCtrl;
}

ITEM* CNewUIMyInventory::FindItem(int iLinealPos) const
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItem(iLinealPos);
    return nullptr;
}

ITEM* CNewUIMyInventory::FindItemByKey(DWORD dwKey) const
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItemByKey(dwKey);
    return nullptr;
}

int CNewUIMyInventory::FindItemIndex(short int siType, int iLevel) const
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItemIndex(siType, iLevel);
    return -1;
}

int CNewUIMyInventory::FindItemReverseIndex(short sType, int iLevel) const
{
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->FindItemReverseIndex(sType, iLevel);
    }

    return -1;
}

int CNewUIMyInventory::FindEmptySlot(IN int cx, IN int cy) const
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindEmptySlot(cx, cy);
    return -1;
}

int CNewUIMyInventory::FindEmptySlot(ITEM* pItem) const
{
    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->FindEmptySlot(pItemAttr->Width, pItemAttr->Height);
    }

    return -1;
}

void CNewUIMyInventory::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    if (pClass)
    {
        auto* pMyInventory = (CNewUIMyInventory*)(pClass);

        if (dwParamB == ITEM_SET_OPTION)
        {
            g_csItemOption.RenderSetOptionList(pMyInventory->GetPos().x, pMyInventory->GetPos().y);
        }
        else if (dwParamB == ITEM_SOCKET_SET_OPTION)
        {
            g_SocketItemMgr.RenderToolTipForSocketSetOption(pMyInventory->GetPos().x, pMyInventory->GetPos().y);
        }
        else
        {
            pMyInventory->RenderItemToolTip(dwParamA);
        }
    }
}

void CNewUIMyInventory::SetStandbyItemKey(DWORD dwItemKey)
{
    m_dwStandbyItemKey = dwItemKey;
}

DWORD CNewUIMyInventory::GetStandbyItemKey() const
{
    return m_dwStandbyItemKey;
}

int CNewUIMyInventory::GetStandbyItemIndex() const
{
    if (ITEM* pItem = GetStandbyItem())
    {
        return m_pNewInventoryCtrl->GetIndexByItem(pItem);
    }
    return -1;
}

ITEM* CNewUIMyInventory::GetStandbyItem() const
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItemByKey(m_dwStandbyItemKey);
    return nullptr;
}

void CNewUIMyInventory::CreateEquippingEffect(ITEM* pItem)
{
    SetCharacterClass(Hero);
    OBJECT* pHeroObject = &Hero->Object;
    if (false == gMapManager.InChaosCastle())
    {
        switch (pItem->Type)
        {
        case ITEM_HELPER:
            CreateMount(MODEL_HELPER, pHeroObject->Position, pHeroObject);
            break;
        case ITEM_HORN_OF_UNIRIA:
            CreateMount(MODEL_UNICON, pHeroObject->Position, pHeroObject);
            if (!Hero->SafeZone)
                CreateEffect(BITMAP_MAGIC + 1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
            break;
        case ITEM_HORN_OF_DINORANT:
            CreateMount(MODEL_PEGASUS, pHeroObject->Position, pHeroObject);
            if (!Hero->SafeZone)
                CreateEffect(BITMAP_MAGIC + 1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
            break;
        case ITEM_DARK_HORSE_ITEM:
            CreateMount(MODEL_DARK_HORSE, pHeroObject->Position, pHeroObject);
            if (!Hero->SafeZone)
                CreateEffect(BITMAP_MAGIC + 1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
            break;
        case ITEM_HORN_OF_FENRIR:
            Hero->Helper.ExcellentFlags = pItem->ExcellentFlags;
            if (pItem->ExcellentFlags == 0x01)
            {
                CreateMount(MODEL_FENRIR_BLACK, pHeroObject->Position, pHeroObject);
            }
            else if (pItem->ExcellentFlags == 0x02)
            {
                CreateMount(MODEL_FENRIR_BLUE, pHeroObject->Position, pHeroObject);
            }
            else if (pItem->ExcellentFlags == 0x04)
            {
                CreateMount(MODEL_FENRIR_GOLD, pHeroObject->Position, pHeroObject);
            }
            else
            {
                CreateMount(MODEL_FENRIR_RED, pHeroObject->Position, pHeroObject);
            }

            if (!Hero->SafeZone)
            {
                CreateEffect(BITMAP_MAGIC + 1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
            }
            break;
        case ITEM_DEMON:
            ThePetProcess().CreatePet(pItem->Type, MODEL_DEMON, pHeroObject->Position, Hero);
            break;
        case ITEM_SPIRIT_OF_GUARDIAN:
            ThePetProcess().CreatePet(pItem->Type, MODEL_SPIRIT_OF_GUARDIAN, pHeroObject->Position, Hero);
            break;
        case ITEM_PET_RUDOLF:
            ThePetProcess().CreatePet(pItem->Type, MODEL_PET_RUDOLF, pHeroObject->Position, Hero);
            break;
        case ITEM_PET_PANDA:
            ThePetProcess().CreatePet(pItem->Type, MODEL_PET_PANDA, pHeroObject->Position, Hero);
            break;
        case ITEM_PET_UNICORN:
            ThePetProcess().CreatePet(pItem->Type, MODEL_PET_UNICORN, pHeroObject->Position, Hero);
            break;
        case ITEM_PET_SKELETON:
            ThePetProcess().CreatePet(pItem->Type, MODEL_PET_SKELETON, pHeroObject->Position, Hero);
            break;
        }
    }
    if (Hero->EtcPart <= 0 || Hero->EtcPart > 3)
    {
        if (pItem->Type == ITEM_WIZARDS_RING && pItem->Level == 3)
        {
            DeleteParts(Hero);
            Hero->EtcPart = PARTS_LION;
        }
    }
    if (pItem->Type == ITEM_WING_OF_RUIN || pItem->Type == ITEM_CAPE_OF_LORD ||
        pItem->Type == ITEM_WING + 130 ||
        (pItem->Type >= ITEM_CAPE_OF_FIGHTER && pItem->Type <= ITEM_CAPE_OF_OVERRULE) ||
        (pItem->Type == ITEM_WING + 135) ||
        pItem->Type == ITEM_CAPE_OF_EMPEROR)
    {
        DeleteCloth(Hero, &Hero->Object);
    }
}

void CNewUIMyInventory::DeleteEquippingEffectBug(ITEM* pItem)
{
    if (ThePetProcess().IsPet(pItem->Type) == true)
    {
        ThePetProcess().DeletePet(Hero, pItem->Type);
    }

    switch (pItem->Type)
    {
    case ITEM_CAPE_OF_LORD:
    case ITEM_WING_OF_RUIN:
    case ITEM_CAPE_OF_EMPEROR:
    case ITEM_WING + 130:
    case ITEM_CAPE_OF_FIGHTER:
    case ITEM_CAPE_OF_OVERRULE:
    case ITEM_WING + 135:
        DeleteCloth(Hero, &Hero->Object);
        return;
    }

    if (IsMount(pItem) == true)
    {
        DeleteMount(&Hero->Object);
    }
}

void CNewUIMyInventory::DeleteEquippingEffect()
{
    if (Hero->EtcPart < PARTS_ATTACK_TEAM_MARK)
    {
        DeleteParts(Hero);
        if (Hero->EtcPart > 3)
        {
            Hero->EtcPart = 0;
        }
    }

    SetCharacterClass(Hero);
}

void CNewUIMyInventory::SetEquipmentSlotInfo()
{
    m_EquipmentSlots[EQUIPMENT_HELPER].x = m_Pos.x + 15;
    m_EquipmentSlots[EQUIPMENT_HELPER].y = m_Pos.y + 44;
    m_EquipmentSlots[EQUIPMENT_HELPER].width = 46;
    m_EquipmentSlots[EQUIPMENT_HELPER].height = 46;
    m_EquipmentSlots[EQUIPMENT_HELPER].dwBgImage = IMAGE_INVENTORY_ITEM_FAIRY;

    m_EquipmentSlots[EQUIPMENT_HELM].x = m_Pos.x + 75;
    m_EquipmentSlots[EQUIPMENT_HELM].y = m_Pos.y + 44;
    m_EquipmentSlots[EQUIPMENT_HELM].width = 46;
    m_EquipmentSlots[EQUIPMENT_HELM].height = 46;
    m_EquipmentSlots[EQUIPMENT_HELM].dwBgImage = IMAGE_INVENTORY_ITEM_HELM;

    m_EquipmentSlots[EQUIPMENT_WING].x = m_Pos.x + 120;
    m_EquipmentSlots[EQUIPMENT_WING].y = m_Pos.y + 44;
    m_EquipmentSlots[EQUIPMENT_WING].width = 61;
    m_EquipmentSlots[EQUIPMENT_WING].height = 46;
    m_EquipmentSlots[EQUIPMENT_WING].dwBgImage = IMAGE_INVENTORY_ITEM_WING;

    m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].x = m_Pos.x + 135;
    m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].y = m_Pos.y + 87;
    m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].width = 46;
    m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].height = 66;
    m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].dwBgImage = IMAGE_INVENTORY_ITEM_LEFT;

    m_EquipmentSlots[EQUIPMENT_ARMOR].x = m_Pos.x + 75;
    m_EquipmentSlots[EQUIPMENT_ARMOR].y = m_Pos.y + 87;
    m_EquipmentSlots[EQUIPMENT_ARMOR].width = 46;
    m_EquipmentSlots[EQUIPMENT_ARMOR].height = 66;
    m_EquipmentSlots[EQUIPMENT_ARMOR].dwBgImage = IMAGE_INVENTORY_ITEM_ARMOR;

    m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].x = m_Pos.x + 15;
    m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].y = m_Pos.y + 87;
    m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].width = 46;
    m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].height = 66;
    m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].dwBgImage = IMAGE_INVENTORY_ITEM_RIGHT;

    m_EquipmentSlots[EQUIPMENT_GLOVES].x = m_Pos.x + 15;
    m_EquipmentSlots[EQUIPMENT_GLOVES].y = m_Pos.y + 150;
    m_EquipmentSlots[EQUIPMENT_GLOVES].width = 46;
    m_EquipmentSlots[EQUIPMENT_GLOVES].height = 46;
    m_EquipmentSlots[EQUIPMENT_GLOVES].dwBgImage = IMAGE_INVENTORY_ITEM_GLOVES;

    m_EquipmentSlots[EQUIPMENT_PANTS].x = m_Pos.x + 75;
    m_EquipmentSlots[EQUIPMENT_PANTS].y = m_Pos.y + 150;
    m_EquipmentSlots[EQUIPMENT_PANTS].width = 46;
    m_EquipmentSlots[EQUIPMENT_PANTS].height = 46;
    m_EquipmentSlots[EQUIPMENT_PANTS].dwBgImage = IMAGE_INVENTORY_ITEM_PANTS;

    m_EquipmentSlots[EQUIPMENT_BOOTS].x = m_Pos.x + 135;
    m_EquipmentSlots[EQUIPMENT_BOOTS].y = m_Pos.y + 150;
    m_EquipmentSlots[EQUIPMENT_BOOTS].width = 46;
    m_EquipmentSlots[EQUIPMENT_BOOTS].height = 46;
    m_EquipmentSlots[EQUIPMENT_BOOTS].dwBgImage = IMAGE_INVENTORY_ITEM_BOOT;

    m_EquipmentSlots[EQUIPMENT_RING_LEFT].x = m_Pos.x + 114;
    m_EquipmentSlots[EQUIPMENT_RING_LEFT].y = m_Pos.y + 150;
    m_EquipmentSlots[EQUIPMENT_RING_LEFT].width = 28;
    m_EquipmentSlots[EQUIPMENT_RING_LEFT].height = 28;
    m_EquipmentSlots[EQUIPMENT_RING_LEFT].dwBgImage = IMAGE_INVENTORY_ITEM_RING;

    m_EquipmentSlots[EQUIPMENT_AMULET].x = m_Pos.x + 54;
    m_EquipmentSlots[EQUIPMENT_AMULET].y = m_Pos.y + 87;
    m_EquipmentSlots[EQUIPMENT_AMULET].width = 28;
    m_EquipmentSlots[EQUIPMENT_AMULET].height = 28;
    m_EquipmentSlots[EQUIPMENT_AMULET].dwBgImage = IMAGE_INVENTORY_ITEM_NECKLACE;

    m_EquipmentSlots[EQUIPMENT_RING_RIGHT].x = m_Pos.x + 54;
    m_EquipmentSlots[EQUIPMENT_RING_RIGHT].y = m_Pos.y + 150;
    m_EquipmentSlots[EQUIPMENT_RING_RIGHT].width = 28;
    m_EquipmentSlots[EQUIPMENT_RING_RIGHT].height = 28;
    m_EquipmentSlots[EQUIPMENT_RING_RIGHT].dwBgImage = IMAGE_INVENTORY_ITEM_RING;
}

void CNewUIMyInventory::SetButtonInfo()
{
    m_BtnExit.ChangeButtonImgState(true, IMAGE_INVENTORY_EXIT_BTN, false);
    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 391, 36, 29);
    m_BtnExit.ChangeToolTipText(GlobalText[225], true);

    m_BtnRepair.ChangeButtonImgState(true, IMAGE_INVENTORY_REPAIR_BTN, false);
    m_BtnRepair.ChangeButtonInfo(m_Pos.x + 50, m_Pos.y + 391, 36, 29);
    m_BtnRepair.ChangeToolTipText(GlobalText[233], true);

    m_BtnMyShop.ChangeButtonImgState(true, IMAGE_INVENTORY_MYSHOP_OPEN_BTN, false);
    m_BtnMyShop.ChangeButtonInfo(m_Pos.x + 87, m_Pos.y + 391, 36, 29);
    m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);

    m_BtnExpand.ChangeButtonImgState(true, IMAGE_INVENTORY_EXPAND_BTN, false);
    m_BtnExpand.ChangeButtonInfo(m_Pos.x + 87 + 37, m_Pos.y + 391, 36, 29);
    m_BtnExpand.ChangeToolTipText(GlobalText[3322], true);
}

void CNewUIMyInventory::LoadImages() const
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_INVENTORY_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_INVENTORY_BACK_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back04.tga", IMAGE_INVENTORY_BACK_TOP2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_INVENTORY_BACK_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_INVENTORY_BACK_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_INVENTORY_BACK_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_boots.tga", IMAGE_INVENTORY_ITEM_BOOT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_cap.tga", IMAGE_INVENTORY_ITEM_HELM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_fairy.tga", IMAGE_INVENTORY_ITEM_FAIRY, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_wing.tga", IMAGE_INVENTORY_ITEM_WING, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_weapon(L).tga", IMAGE_INVENTORY_ITEM_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_weapon(R).tga", IMAGE_INVENTORY_ITEM_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_upper.tga", IMAGE_INVENTORY_ITEM_ARMOR, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_gloves.tga", IMAGE_INVENTORY_ITEM_GLOVES, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_lower.tga", IMAGE_INVENTORY_ITEM_PANTS, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_ring.tga", IMAGE_INVENTORY_ITEM_RING, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_necklace.tga", IMAGE_INVENTORY_ITEM_NECKLACE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_money.tga", IMAGE_INVENTORY_MONEY, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_INVENTORY_EXIT_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_repair_00.tga", IMAGE_INVENTORY_REPAIR_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_expansion_btn.tga", IMAGE_INVENTORY_EXPAND_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_openshop.tga", IMAGE_INVENTORY_MYSHOP_OPEN_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_closeshop.tga", IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, GL_LINEAR);
}

void CNewUIMyInventory::UnloadImages()
{
    DeleteBitmap(IMAGE_INVENTORY_MYSHOP_CLOSE_BTN);
    DeleteBitmap(IMAGE_INVENTORY_MYSHOP_OPEN_BTN);
    DeleteBitmap(IMAGE_INVENTORY_REPAIR_BTN);
    DeleteBitmap(IMAGE_INVENTORY_EXIT_BTN);
    DeleteBitmap(IMAGE_INVENTORY_MONEY);
    DeleteBitmap(IMAGE_INVENTORY_ITEM_NECKLACE);
    DeleteBitmap(IMAGE_INVENTORY_ITEM_RING);
    DeleteBitmap(IMAGE_INVENTORY_ITEM_PANTS);
    DeleteBitmap(IMAGE_INVENTORY_ITEM_GLOVES);
    DeleteBitmap(IMAGE_INVENTORY_ITEM_ARMOR);
    DeleteBitmap(IMAGE_INVENTORY_ITEM_RIGHT);
    DeleteBitmap(IMAGE_INVENTORY_ITEM_LEFT);
    DeleteBitmap(IMAGE_INVENTORY_ITEM_WING);
    DeleteBitmap(IMAGE_INVENTORY_ITEM_FAIRY);
    DeleteBitmap(IMAGE_INVENTORY_ITEM_HELM);
    DeleteBitmap(IMAGE_INVENTORY_ITEM_BOOT);
    DeleteBitmap(IMAGE_INVENTORY_BACK_BOTTOM);
    DeleteBitmap(IMAGE_INVENTORY_BACK_RIGHT);
    DeleteBitmap(IMAGE_INVENTORY_BACK_LEFT);
    DeleteBitmap(IMAGE_INVENTORY_BACK_TOP2);
    DeleteBitmap(IMAGE_INVENTORY_BACK_TOP);
    DeleteBitmap(IMAGE_INVENTORY_BACK);
    DeleteBitmap(IMAGE_INVENTORY_EXPAND_BTN);
}

void CNewUIMyInventory::RenderFrame() const
{
    const auto x = static_cast<float>(m_Pos.x);
    const auto y = static_cast<float>(m_Pos.y);

    RenderImage(IMAGE_INVENTORY_BACK, x, y, INVENTORY_WIDTH, INVENTORY_HEIGHT);
    RenderImage(IMAGE_INVENTORY_BACK_TOP2, x, y, 190.f, 64.f);
    RenderImage(IMAGE_INVENTORY_BACK_LEFT, x, y + 64, 21.f, 320.f);
    RenderImage(IMAGE_INVENTORY_BACK_RIGHT, x + INVENTORY_WIDTH - 21, y + 64, 21.f, 320.f);
    RenderImage(IMAGE_INVENTORY_BACK_BOTTOM, x, y + INVENTORY_HEIGHT - 45, 190.f, 45.f);
}

void CNewUIMyInventory::RenderEquippedItem()
{
    for (int i = 0; i < MAX_EQUIPMENT_INDEX; i++)
    {
        if (i == EQUIPMENT_HELM)
        {
            if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK)
            {
                continue;
            }
        }
        if ((i == EQUIPMENT_GLOVES) && (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER))
            continue;

        EnableAlphaTest();

        RenderImage(m_EquipmentSlots[i].dwBgImage, m_EquipmentSlots[i].x, m_EquipmentSlots[i].y,
            m_EquipmentSlots[i].width, m_EquipmentSlots[i].height);
        DisableAlphaBlend();

        ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[i];
        if (pEquipmentItemSlot->Type != -1)
        {
            ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pEquipmentItemSlot->Type];
            const int iLevel = pEquipmentItemSlot->Level;
            const int iMaxDurability = CalcMaxDurability(pEquipmentItemSlot, pItemAttr, iLevel);

            // 용사/전사의반지 예외처리
            if (i == EQUIPMENT_RING_LEFT || i == EQUIPMENT_RING_RIGHT)
            {
                if (pEquipmentItemSlot->Type == ITEM_WIZARDS_RING && iLevel == 1
                    || iLevel == 2)
                {
                    continue;
                }
            }

            if ((pEquipmentItemSlot->bPeriodItem == true) && (pEquipmentItemSlot->bExpiredPeriod == false))
                continue;

            if (pEquipmentItemSlot->Durability <= 0)
                glColor4f(1.f, 0.f, 0.f, 0.25f);
            else if (pEquipmentItemSlot->Durability <= (iMaxDurability * 0.2f))
                glColor4f(1.f, 0.15f, 0.f, 0.25f);
            else if (pEquipmentItemSlot->Durability <= (iMaxDurability * 0.3f))
                glColor4f(1.f, 0.5f, 0.f, 0.25f);
            else if (pEquipmentItemSlot->Durability <= (iMaxDurability * 0.5f))
                glColor4f(1.f, 1.f, 0.f, 0.25f);
            else if (IsEquipable(i, pEquipmentItemSlot) == false)
                glColor4f(1.f, 0.f, 0.f, 0.25f);
            else
            {
                continue;
            }

            EnableAlphaTest();
            RenderColor(m_EquipmentSlots[i].x + 1, m_EquipmentSlots[i].y, m_EquipmentSlots[i].width - 4, m_EquipmentSlots[i].height - 4);
            EndRenderColor();
        }
    }

    if (CNewUIInventoryCtrl::GetPickedItem() && m_iPointedSlot != -1)
    {
        ITEM* pItemObj = CNewUIInventoryCtrl::GetPickedItem()->GetItem();
        const ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[m_iPointedSlot];
        if (pItemObj && (pEquipmentItemSlot->Type != -1 || false == IsEquipable(m_iPointedSlot, pItemObj))
            && !((gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER) && (m_iPointedSlot == EQUIPMENT_GLOVES)))
        {
            glColor4f(0.9f, 0.1f, 0.1f, 0.4f);
            EnableAlphaTest();
            RenderColor(m_EquipmentSlots[m_iPointedSlot].x + 1, m_EquipmentSlots[m_iPointedSlot].y,
                m_EquipmentSlots[m_iPointedSlot].width - 4, m_EquipmentSlots[m_iPointedSlot].height - 4);
            EndRenderColor();
        }
    }

    if (m_iPointedSlot != -1 && m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, m_iPointedSlot, 0);
    }
}

void CNewUIMyInventory::RenderButtons()
{
    EnableAlphaTest();

    if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) == false
        && g_pNewUISystem->IsVisible(INTERFACE_TRADE) == false
        && g_pNewUISystem->IsVisible(INTERFACE_DEVILSQUARE) == false
        && g_pNewUISystem->IsVisible(INTERFACE_BLOODCASTLE) == false
        && g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY) == false
        && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) == false
        && g_pNewUISystem->IsVisible(INTERFACE_STORAGE) == false)
    {
        if (m_bRepairEnableLevel == true)
        {
            m_BtnRepair.Render();
        }
        if (m_bMyShopOpen == true)
        {
            m_BtnMyShop.Render();
        }
    }
    m_BtnExit.Render();
    m_BtnExpand.Render();

    DisableAlphaBlend();
}

void CNewUIMyInventory::RenderInventoryDetails() const
{
    EnableAlphaTest();

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, GlobalText[223], INVENTORY_WIDTH, 0, RT3_SORT_CENTER);

    RenderImage(IMAGE_INVENTORY_MONEY, m_Pos.x + 11, m_Pos.y + 364, 170.f, 26.f);

    const DWORD dwZen = CharacterMachine->Gold;

    wchar_t Text[256] = { 0, };
    ConvertGold(dwZen, Text);

    g_pRenderText->SetTextColor(getGoldColor(dwZen));
    g_pRenderText->RenderText((int)m_Pos.x + 50, (int)m_Pos.y + 371, Text);

    g_pRenderText->SetFont(g_hFont);

    DisableAlphaBlend();
}

bool CNewUIMyInventory::EquipmentWindowProcess()
{
    if (m_iPointedSlot != -1 && IsRelease(VK_LBUTTON))
    {
        if (CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem())
        {
            ITEM* pItemObj = pPickedItem->GetItem();
            const int iSourceIndex = pPickedItem->GetSourceLinealPos();
            const int iTargetIndex = m_iPointedSlot;
            if (pItemObj->bPeriodItem && pItemObj->bExpiredPeriod)
            {
                g_pSystemLogBox->AddText(GlobalText[2285], SEASON3B::TYPE_ERROR_MESSAGE);
                CNewUIInventoryCtrl::BackupPickedItem();

                ResetMouseLButton();
                return false;
            }

            ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[iTargetIndex];
            if (pEquipmentItemSlot && pEquipmentItemSlot->Type != -1)
            {
                return true;
            }

            if (g_ChangeRingMgr->CheckChangeRing(pPickedItem->GetItem()->Type))
            {
                ITEM* pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
                ITEM* pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

                if (g_ChangeRingMgr->CheckChangeRing(pItemRingLeft->Type) || g_ChangeRingMgr->CheckChangeRing(pItemRingRight->Type))
                {
                    g_pSystemLogBox->AddText(GlobalText[2285], TYPE_ERROR_MESSAGE);
                    CNewUIInventoryCtrl::BackupPickedItem();

                    ResetMouseLButton();
                    return false;
                }
            }

            if (IsEquipable(iTargetIndex, pItemObj))
            {
                const STORAGE_TYPE sourceType = pPickedItem->GetSourceStorageType();

                if (sourceType == STORAGE_TYPE::INVENTORY && iSourceIndex == iTargetIndex)
                {
                    CNewUIInventoryCtrl::BackupPickedItem();
                }
                else
                {
                    SendRequestEquipmentItem(sourceType, iSourceIndex, pItemObj, STORAGE_TYPE::INVENTORY, iTargetIndex);
                    return true;
                }
            }
        }
        else // pPickedItem == NULL
        {
            if (GetRepairMode() == REPAIR_MODE_ON)
            {
                ITEM* pEquippedItem = &CharacterMachine->Equipment[m_iPointedSlot];

                if (pEquippedItem == NULL)
                {
                    return true;
                }

                if (IsRepairBan(pEquippedItem) == true)
                {
                    return true;
                }

                if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) && g_pNPCShop->IsRepairShop())
                {
                    SocketClient->ToGameServer()->SendRepairItemRequest(m_iPointedSlot, 0);
                }
                else if (m_bRepairEnableLevel == true)
                {
                    SocketClient->ToGameServer()->SendRepairItemRequest(m_iPointedSlot, 1);
                }

                return true;
            }

            ITEM* pEquippedItem = &CharacterMachine->Equipment[m_iPointedSlot];
            if (pEquippedItem->Type >= 0)
            {
                if (gMapManager.WorldActive == WD_10HEAVEN)
                {
                    const ITEM* pEquippedPetItem = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
                    bool bPicked = true;

                    if (m_iPointedSlot == EQUIPMENT_HELPER || m_iPointedSlot == EQUIPMENT_WING)
                    {
                        if (((m_iPointedSlot == EQUIPMENT_HELPER) && !gCharacterManager.IsEquipedWing()))
                        {
                            bPicked = false;
                        }
                        else if (((m_iPointedSlot == EQUIPMENT_WING) && !((pEquippedPetItem->Type == ITEM_HORN_OF_DINORANT) || (pEquippedPetItem->Type == ITEM_DARK_HORSE_ITEM) || (pEquippedPetItem->Type == ITEM_HORN_OF_FENRIR)))
                            )
                        {
                            bPicked = false;
                        }
                    }

                    if (bPicked == true)
                    {
                        if (CNewUIInventoryCtrl::CreatePickedItem(nullptr, pEquippedItem))
                        {
                            UnequipItem(m_iPointedSlot);
                        }
                    }
                }
                else
                {
                    if (CNewUIInventoryCtrl::CreatePickedItem(nullptr, pEquippedItem))
                    {
                        UnequipItem(m_iPointedSlot);
                    }
                }
            }
        }
    }

    if (IsRelease(VK_RBUTTON))
    {
        const CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

        const int iSourceIndex = m_iPointedSlot;
        if (GetRepairMode() != REPAIR_MODE_ON && EquipmentItem == false
            && pPickedItem == nullptr
            && iSourceIndex != -1)
        {
            ResetMouseRButton();

            ITEM* pEquippedItem = &CharacterMachine->Equipment[iSourceIndex];

            if (pEquippedItem->Type >= 0)
            {
                const int emptySlotIndex = FindEmptySlot(pEquippedItem);

                if (emptySlotIndex != -1)
                {
                    // This code looks tricky... it simulates a pick up and click on the inventory slot.
                    // God knows what happens, when this request to the server goes wrong.
                    if (CNewUIInventoryCtrl::CreatePickedItem(nullptr, pEquippedItem))
                    {
                        CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
                        UnequipItem(iSourceIndex);
                        pPickedItem->HidePickedItem();
                    }

                    SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex, pEquippedItem, STORAGE_TYPE::INVENTORY, emptySlotIndex);
                    return true;
                }
            }
        }
    }

    return false;
}

bool CNewUIMyInventory::RepairItemAtMousePoint(CNewUIInventoryCtrl* targetControl)
{
    ITEM* pItem = targetControl->FindItemAtPt(MouseX, MouseY);
    if (pItem == nullptr)
    {
        return true;
    }

    if (IsRepairBan(pItem) == true)
    {
        return true;
    }

    int iIndex = targetControl->GetIndex(pItem->x, pItem->y);

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

bool CNewUIMyInventory::ApplyJewels(CNewUIInventoryCtrl* targetControl, CNewUIPickedItem* pPickedItem, ITEM* pPickItem, const int iSourceIndex, const int iTargetIndex)
{
    auto pItem = targetControl->FindItem(iTargetIndex);
    if (!pItem)
    {
        return false;
    }

    const int	iType = pItem->Type;
    const int	iLevel = pItem->Level;
    const int	iDurability = pItem->Durability;

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

    if ((pPickItem->Type == ITEM_JEWEL_OF_BLESS && iLevel >= 6) || (pPickItem->Type == ITEM_JEWEL_OF_SOUL && iLevel >= 9))
    {
        bSuccess = false;
    }

    if (pPickItem->Type == ITEM_JEWEL_OF_BLESS && iType == ITEM_HORN_OF_FENRIR && iDurability != 255)
    {
        CFenrirRepairMsgBox* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CFenrirRepairMsgBoxLayout), &pMsgBox);
        pMsgBox->SetSourceIndex(iSourceIndex);

        auto iIndex = targetControl->GetIndex(pItem->x, pItem->y);
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
        else
            if (pItem->Jewel_Of_Harmony_Option != 0)
            {
                bSuccess = false;
            }
            else
            {
                const StrengthenItem strengthitem = g_pUIJewelHarmonyinfo->GetItemType(static_cast<int>(pItem->Type));

                if (strengthitem == SI_None)
                {
                    bSuccess = false;
                }
            }
    }

    if (pPickItem->Type == ITEM_LOWER_REFINE_STONE || pPickItem->Type == ITEM_HIGHER_REFINE_STONE)
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
            if (pItem->Jewel_Of_Harmony_Option == 0)	bSuccess = true;
        }
        else if (pPickItem->Type == ITEM_POTION + 160)
        {
            if (pItem->Durability > 0)					bSuccess = true;
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

bool CNewUIMyInventory::TryStackItems(CNewUIInventoryCtrl* targetControl, ITEM* pPickItem, const int iSourceIndex, const int iTargetIndex)
{
    if (ITEM* pItem = targetControl->FindItem(iTargetIndex))
    {
        if (targetControl->AreItemsStackable(pPickItem, pItem))
        {
            SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex, pPickItem, STORAGE_TYPE::INVENTORY, iTargetIndex);
            return true;
        }
    }

    return false;
}

bool CNewUIMyInventory::TryConsumeItem(CNewUIInventoryCtrl* targetControl, ITEM* pItem, const int iIndex)
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

    const auto isApple = pItem->Type == ITEM_APPLE;
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
        || pItem->Type == ITEM_POTION + 133
        )
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

    if ((pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82))
    {
        std::list<eBuffState> secretPotionbufflist;
        secretPotionbufflist.push_back(eBuff_SecretPotion1);
        secretPotionbufflist.push_back(eBuff_SecretPotion2);
        secretPotionbufflist.push_back(eBuff_SecretPotion3);
        secretPotionbufflist.push_back(eBuff_SecretPotion4);
        secretPotionbufflist.push_back(eBuff_SecretPotion5);

        if (g_isCharacterBufflist((&Hero->Object), secretPotionbufflist) == eBuffNone) {
            SendRequestUse(iIndex, 0);
            return true;
        }

        CreateOkMessageBox(GlobalText[2530], RGBA(255, 30, 0, 255));
        return false;
    }

    if ((pItem->Type >= ITEM_HELPER + 54 && pItem->Type <= ITEM_HELPER + 57) || (pItem->Type == ITEM_HELPER + 58 && gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD))
    {
        bool result = true;
        WORD point[5] = { 0, };

        point[0] = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
        point[1] = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        point[2] = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
        point[3] = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
        point[4] = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;

        wchar_t nStat[MAX_CLASS][5] =
        {
            18, 18, 15, 30,	0,
            28, 20, 25, 10,	0,
            22, 25, 20, 15,	0,
            26, 26, 26, 26,	0,
            26, 20, 20, 15, 25,
            21, 21, 18, 23,	0,
            32, 27, 25, 20, 0,
        };

        const auto attributeType = pItem->Type - (ITEM_HELPER + 54);
        const int characterClass = gCharacterManager.GetBaseClass(Hero->Class);
        point[attributeType] -= nStat[characterClass][attributeType];
        if (point[attributeType] < (pItem->Durability * 10))
        {
            g_pMyInventory->SetStandbyItemKey(pItem->Key);
            CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUsePartChargeFruitMsgBoxLayout));
            return false;
        }

        SendRequestUse(iIndex, 0);
        return true;
    }

    if (pItem->Type == ITEM_HELPER + 58 && gCharacterManager.GetBaseClass(Hero->Class) != CLASS_DARK_LORD)
    {
        CreateOkMessageBox(GlobalText[1905]);
        return true;
    }

    if (pItem->Type == ITEM_ARMOR_OF_GUARDSMAN)
    {
        if (IsUnitedMarketPlace())
        {
            wchar_t	szOutputText[512];
            swprintf(szOutputText, L"%s %s", GlobalText[3014], GlobalText[3015]);

            CreateOkMessageBox(szOutputText);
            return true;
        }

        if (Hero->SafeZone == false)
        {
            CreateOkMessageBox(GlobalText[2330]);
            return false;
        }

        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(4, pItem->Level);
        g_pMyInventory->SetStandbyItemKey(pItem->Key);
        return true;
    }

    if (pItem->Type == ITEM_HELPER + 46)
    {
        const BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_DEVILSQUARE);
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(1, byPossibleLevel);
        return false;
    }

    if (pItem->Type == ITEM_HELPER + 47)
    {
        const BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_BLOODCASTLE);
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(2, byPossibleLevel);
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
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(5, byPossibleLevel);
        return true;
    }

    if (pItem->Type == ITEM_HELPER + 121)
    {
        if (Hero->SafeZone == false)
        {
            CreateOkMessageBox(GlobalText[2330]);
            return false;
        }

        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(4, pItem->Level);
        g_pMyInventory->SetStandbyItemKey(pItem->Key);
        return true;
    }

    if (pItem->Type == ITEM_SCROLL_OF_BLOOD)
    {
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(5, pItem->Level);
        return true;
    }

    if (pItem->Type == ITEM_DEVILS_INVITATION)
    {
        SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(1, pItem->Level);
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
            SocketClient->ToGameServer()->SendMiniGameOpeningStateRequest(2, pItem->Level - 1);
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
        || (pItem->Type == ITEM_SCROLL_OF_WIZARDRY_ENHANCE)
        )
    {
        bool bReadBookGem = true;

        if ((pItem->Type == ITEM_SCROLL_OF_NOVA)
            || (pItem->Type == ITEM_SCROLL_OF_WIZARDRY_ENHANCE)
            || pItem->Type == ITEM_CRYSTAL_OF_MULTI_SHOT
            || (pItem->Type == ITEM_CRYSTAL_OF_RECOVERY)
            || (pItem->Type == ITEM_CRYSTAL_OF_DESTRUCTION)
            )
        {
            if (g_csQuest.getQuestState2(QUEST_CHANGE_UP_3) != QUEST_END)

                bReadBookGem = false;
        }
        if (pItem->Type == ITEM_SCROLL_OF_CHAOTIC_DISEIER)
        {
            const int Level = CharacterAttribute->Level;
            if (Level < 220)
                bReadBookGem = false;
        }
        if (bReadBookGem)
        {
            WORD Strength, Energy;
            Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
            Energy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;

            if (CharacterAttribute->Level >= ItemAttribute[pItem->Type].RequireLevel &&
                Energy >= pItem->RequireEnergy &&
                Strength >= pItem->RequireStrength)
            {
                SendRequestUse(iIndex, 0);
            }

            return true;
        }

        return false;
    }

    if (pItem->Type == ITEM_FRUITS)
    {
        const int Level = CharacterAttribute->Level;

        if (Level >= 10)
        {
            bool bEquipmentEmpty = true;
            for (int i = 0; i < MAX_EQUIPMENT; i++)
            {
                if (CharacterMachine->Equipment[i].Type != -1)
                {
                    bEquipmentEmpty = false;
                }
            }
            const auto Class = CharacterAttribute->Class;

            if (bEquipmentEmpty == true)
            {
                if (pItem->Level == 4) // Command Fruit
                {
                    if (gCharacterManager.GetBaseClass(Class) != CLASS_DARK_LORD)
                    {
                        CreateOkMessageBox(GlobalText[1905]);
                        return true;
                    }
                }
                g_pMyInventory->SetStandbyItemKey(pItem->Key);
                CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseFruitMsgBoxLayout));
                return true;
            }

            CreateOkMessageBox(GlobalText[1909]);
            return true;
        }

        CreateOkMessageBox(GlobalText[749]);
        return true;
    }

    if (pItem->Type == ITEM_LIFE_STONE_ITEM)
    {
        bool bUse = false;
        switch (pItem->Level)
        {
        case 0:
            bUse = true;
            break;
        case 1:
            if (Hero->GuildStatus != G_MASTER)
                bUse = true;
            break;
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

// TODO: This whole logic (and possibly others) should be moved into a 'controller' class or similar.
bool CNewUIMyInventory::HandleInventoryActions(CNewUIInventoryCtrl* targetControl)
{
    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem && IsRelease(VK_LBUTTON))
    {
        ITEM* pPickItem = pPickedItem->GetItem();
        if (pPickItem == nullptr)
        {
            return true;
        }

        const int iSourceIndex = pPickedItem->GetSourceLinealPos();
        const int iTargetIndex = pPickedItem->GetTargetLinealPos(targetControl);

        if (pPickedItem->GetOwnerInventory() == targetControl
            || g_pMyInventoryExt->GetOwnerOf(pPickedItem)) // Movement between Inventory (and within extensions)
        {
            // Apply Jewels:
            if ((pPickItem->Type == ITEM_JEWEL_OF_BLESS
                || pPickItem->Type == ITEM_JEWEL_OF_SOUL
                || pPickItem->Type == ITEM_JEWEL_OF_LIFE
                || pPickItem->Type == ITEM_JEWEL_OF_HARMONY
                || pPickItem->Type == ITEM_LOWER_REFINE_STONE
                || pPickItem->Type == ITEM_HIGHER_REFINE_STONE
                || pPickItem->Type == ITEM_POTION + 160
                || pPickItem->Type == ITEM_POTION + 161
                ) && ApplyJewels(targetControl, pPickedItem, pPickItem, iSourceIndex, iTargetIndex))
            {
                return true;
            }

            if (iTargetIndex != -1 && TryStackItems(targetControl, pPickItem, iSourceIndex, iTargetIndex))
            {
                return true;
            }
        }

        // If nothing of above applied, we try to move the item.
        if (iTargetIndex >= 0 && targetControl->CanMove(iTargetIndex, pPickItem))
        {
            const auto sourceStorageType = pPickedItem->GetSourceStorageType();
            const auto targetStorageType = targetControl->GetStorageType();
            if (iTargetIndex != iSourceIndex)
            {
                return SendRequestEquipmentItem(sourceStorageType, iSourceIndex,
                    pPickItem, targetStorageType, iTargetIndex);
            }
            else
            {
                CNewUIInventoryCtrl::BackupPickedItem();
            }
        }
    }
    else if (g_pMyInventory->GetRepairMode() == REPAIR_MODE_OFF && IsPress(VK_RBUTTON))
    {
        // handle right click (item usage etc.)
        g_pMyInventory->ResetMouseRButton();

        if (g_pNewUISystem->IsVisible(INTERFACE_STORAGE))
        {
            return g_pStorageInventory->ProcessMyInvenItemAutoMove();
        }

        if (g_pNewUISystem->IsVisible(INTERFACE_INVENTORY)
            && !g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP)
            && !g_pNewUISystem->IsVisible(INTERFACE_TRADE)
            && !g_pNewUISystem->IsVisible(INTERFACE_DEVILSQUARE)
            && !g_pNewUISystem->IsVisible(INTERFACE_BLOODCASTLE)
            && !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
            && !g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY))
        {
            ITEM* pItem = targetControl->FindItemAtPt(MouseX, MouseY);
            if (!pItem)
            {
                return false;
            }

            const int iIndex = targetControl->GetIndexByItem(pItem);
            if (iIndex >= 0 && TryConsumeItem(targetControl, pItem, iIndex))
            {
                return true;
            }
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
            else if (g_pMyInventory->IsInvenItem(pItem->Type))
            {
#ifdef LJH_FIX_APP_SHUTDOWN_WEQUIPPING_INVENITEM_WITH_CLICKING_MOUSELBTN
                if (MouseLButton || MouseLButtonPop || MouseLButtonPush)
                    return false;
#endif //LJH_FIX_APP_SHUTDOWN_WEQUIPPING_INVENITEM_WITH_CLICKING_MOUSELBTN
                if (pItem->Durability == 0)
                    return false;

                int iChangeInvenItemStatus = 0;
                (pItem->Durability == 255) ? iChangeInvenItemStatus = 254 : iChangeInvenItemStatus = 255;

                SendRequestEquippingInventoryItem(iIndex, iChangeInvenItemStatus);
            }
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
            //-- Equip item
            if (!EquipmentItem)
            {
                const int iSrcIndex = iIndex;

                const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];

                int nDstIndex = pItemAttr->m_byItemSlot;

                if (nDstIndex >= 0 && nDstIndex < MAX_EQUIPMENT_INDEX)
                {
                    const ITEM* pEquipment = &CharacterMachine->Equipment[nDstIndex];

                    if (pEquipment && pEquipment->Type != -1)
                    {
                        if (nDstIndex == EQUIPMENT_WEAPON_RIGHT)
                        {
                            if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK
                                || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
                            {
                                if (!pItemAttr->TwoHand)
                                {
                                    nDstIndex = EQUIPMENT_WEAPON_LEFT;
                                    pEquipment = &CharacterMachine->Equipment[nDstIndex];

                                    if (!(pEquipment && pEquipment->Type != -1))
                                    {
                                        goto LABEL_32;
                                    }
                                }
                            }
                        }
                        else if (nDstIndex == EQUIPMENT_RING_RIGHT)
                        {
                            nDstIndex = EQUIPMENT_RING_LEFT;
                            pEquipment = &CharacterMachine->Equipment[nDstIndex];

                            if (!(pEquipment && pEquipment->Type != -1))
                            {
                                goto LABEL_32;
                            }
                        }

                        return true;
                    }
                LABEL_32: // wtf...
                    if (g_pMyInventory->IsEquipable(nDstIndex, pItem))
                    {
                        if (CNewUIInventoryCtrl::CreatePickedItem(nullptr, pItem))
                        {
                            targetControl->RemoveItem(pItem);

                            const auto pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
                            pPickedItem->HidePickedItem();
                        }

                        SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSrcIndex, pItem, STORAGE_TYPE::INVENTORY, nDstIndex);
                    }
                }

                return true;
            }
        }

        return false;
    }

    if (g_pMyInventory->GetRepairMode() == REPAIR_MODE_ON && IsPress(VK_LBUTTON))
    {
        // Repair stuff

        return RepairItemAtMousePoint(targetControl);
    }

    return false;
}

bool CNewUIMyInventory::InventoryProcess() const
{
    if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT) == false)
    {
        return false;
    }

    if (m_pNewInventoryCtrl == nullptr)
    {
        return false;
    }

    return HandleInventoryActions(m_pNewInventoryCtrl);
}

bool CNewUIMyInventory::BtnProcess()
{
    const POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(INTERFACE_INVENTORY);
        return true;
    }
    if (m_BtnExit.UpdateMouseEvent())
    {
        if (g_pNewUISystem->IsVisible(INTERFACE_MYSHOP_INVENTORY))
        {
            g_pNewUISystem->Hide(INTERFACE_MYSHOP_INVENTORY);
        }
        g_pNewUISystem->Hide(INTERFACE_INVENTORY);
        return true;
    }

    if (m_BtnExpand.UpdateMouseEvent())
    {
        g_pNewUISystem->Toggle(INTERFACE_INVENTORY_EXT);
        return true;
    }

    if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) == false
        && g_pNewUISystem->IsVisible(INTERFACE_TRADE) == false
        && g_pNewUISystem->IsVisible(INTERFACE_DEVILSQUARE) == false
        && g_pNewUISystem->IsVisible(INTERFACE_BLOODCASTLE) == false
        && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) == false
        && g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY) == false
        && g_pNewUISystem->IsVisible(INTERFACE_STORAGE) == false)
    {
        if (m_bRepairEnableLevel == true && m_BtnRepair.UpdateMouseEvent() == true)
        {
            ToggleRepairMode();
            return true;
        }

        if (m_bMyShopOpen == true && m_BtnMyShop.UpdateMouseEvent() == true)
        {
            if (m_MyShopMode == MYSHOP_MODE_OPEN)
            {
                ChangeMyShopButtonStateClose();
                g_pNewUISystem->Show(INTERFACE_MYSHOP_INVENTORY);
            }
            else if (m_MyShopMode == MYSHOP_MODE_CLOSE)
            {
                ChangeMyShopButtonStateOpen();
                g_pNewUISystem->Hide(INTERFACE_MYSHOP_INVENTORY);
                g_pNewUISystem->Hide(INTERFACE_PURCHASESHOP_INVENTORY);
            }

            return true;
        }
    }

    return false;
}

void CNewUIMyInventory::RenderItemToolTip(int iSlotIndex) const
{
    if (m_iPointedSlot != -1)
    {
        ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[iSlotIndex];
        if (pEquipmentItemSlot->Type != -1)
        {
            const int iTargetX = m_EquipmentSlots[iSlotIndex].x + m_EquipmentSlots[iSlotIndex].width / 2;
            const int iTargetY = m_EquipmentSlots[iSlotIndex].y + m_EquipmentSlots[iSlotIndex].height / 2;

            pEquipmentItemSlot->bySelectedSlotIndex = iSlotIndex;

            if (m_RepairMode == REPAIR_MODE_OFF)
            {
                RenderItemInfo(iTargetX, iTargetY, pEquipmentItemSlot, false);
            }
            else
            {
                RenderRepairInfo(iTargetX, iTargetY, pEquipmentItemSlot, false);
            }
        }
    }
}

bool CNewUIMyInventory::CanRegisterItemHotKey(int iType)
{
    switch (iType)
    {
    case ITEM_APPLE:
    case ITEM_SMALL_HEALING_POTION:
    case ITEM_MEDIUM_HEALING_POTION:
    case ITEM_LARGE_HEALING_POTION:
    case ITEM_SMALL_MANA_POTION:
    case ITEM_MEDIUM_MANA_POTION:
    case ITEM_LARGE_MANA_POTION:
    case ITEM_SIEGE_POTION:
    case ITEM_ANTIDOTE:
    case ITEM_ALE:
    case ITEM_TOWN_PORTAL_SCROLL:
    case ITEM_POTION + 20:
    case ITEM_SMALL_SHIELD_POTION:
    case ITEM_MEDIUM_SHIELD_POTION:
    case ITEM_LARGE_SHIELD_POTION:
    case ITEM_SMALL_COMPLEX_POTION:
    case ITEM_MEDIUM_COMPLEX_POTION:
    case ITEM_LARGE_COMPLEX_POTION:
    case ITEM_JACK_OLANTERN_BLESSINGS:
    case ITEM_JACK_OLANTERN_WRATH:
    case ITEM_JACK_OLANTERN_CRY:
    case ITEM_JACK_OLANTERN_FOOD:
    case ITEM_JACK_OLANTERN_DRINK:
    case ITEM_POTION + 70:
    case ITEM_POTION + 71:
    case ITEM_POTION + 78:
    case ITEM_POTION + 79:
    case ITEM_POTION + 80:
    case ITEM_POTION + 81:
    case ITEM_POTION + 82:
    case ITEM_POTION + 94:
    case ITEM_CHERRY_BLOSSOM_WINE:
    case ITEM_CHERRY_BLOSSOM_RICE_CAKE:
    case ITEM_CHERRY_BLOSSOM_FLOWER_PETAL:
    case ITEM_POTION + 133:
        return true;
    }

    return false;
}

bool CNewUIMyInventory::CanOpenMyShopInterface()
{
    if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP)
        || g_pNewUISystem->IsVisible(INTERFACE_STORAGE)
        || g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
        || g_pNewUISystem->IsVisible(INTERFACE_TRADE)
        || gMapManager.IsCursedTemple()
        )
    {
        return false;
    }
    return true;
}

bool CNewUIMyInventory::IsRepairEnableLevel() const
{
    return m_bRepairEnableLevel;
}

void CNewUIMyInventory::SetRepairEnableLevel(bool bOver)
{
    m_bRepairEnableLevel = bOver;
}

void CNewUIMyInventory::ChangeMyShopButtonStateOpen()
{
    m_MyShopMode = MYSHOP_MODE_OPEN;
    m_BtnMyShop.UnRegisterButtonState();
    m_BtnMyShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_INVENTORY_MYSHOP_OPEN_BTN, 0);
    m_BtnMyShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_INVENTORY_MYSHOP_OPEN_BTN, 1);
    m_BtnMyShop.ChangeImgIndex(IMAGE_INVENTORY_MYSHOP_OPEN_BTN, 0);
    m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);
}

void CNewUIMyInventory::ChangeMyShopButtonStateClose()
{
    m_MyShopMode = MYSHOP_MODE_CLOSE;
    m_BtnMyShop.UnRegisterButtonState();
    m_BtnMyShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, 0);
    m_BtnMyShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, 1);
    m_BtnMyShop.ChangeImgIndex(IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, 0);
    m_BtnMyShop.ChangeToolTipText(GlobalText[1127], true);
}

void CNewUIMyInventory::LockMyShopButtonOpen()
{
    m_BtnMyShop.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
    m_BtnMyShop.ChangeTextColor(RGBA(100, 100, 100, 255));
    m_BtnMyShop.Lock();
    // 1125 "개인상점열기(S)"
    m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);
}

void CNewUIMyInventory::UnlockMyShopButtonOpen()
{
    m_BtnMyShop.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_BtnMyShop.ChangeTextColor(RGBA(255, 255, 255, 255));
    m_BtnMyShop.UnLock();
    // 1125 "개인상점열기(S)"
    m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);
}

void CNewUIMyInventory::ToggleRepairMode()
{
    //. 토글 수리모드
    if (m_RepairMode == REPAIR_MODE_OFF)
    {
        SetRepairMode(true);
    }
    else if (m_RepairMode == REPAIR_MODE_ON)
    {
        SetRepairMode(false);
    }
}

bool CNewUIMyInventory::IsItem(short int siType, bool bcheckPick) const
{
    if (bcheckPick == true)
    {
        const CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

        if (pPickedItem)
        {
            const ITEM* pItemObj = pPickedItem->GetItem();

            if (pItemObj->Type == siType) return true;
        }
    }

    const ITEM* pholyitemObj = m_pNewInventoryCtrl->FindTypeItem(siType);

    if (pholyitemObj) return true;

    return false;
}

int CNewUIMyInventory::GetNumItemByKey(DWORD dwItemKey) const
{
    return m_pNewInventoryCtrl->GetNumItemByKey(dwItemKey);
}

int CNewUIMyInventory::GetNumItemByType(short sItemType) const
{
    return m_pNewInventoryCtrl->GetNumItemByType(sItemType);
}

BYTE CNewUIMyInventory::GetDurabilityPointedItem() const
{
    const ITEM* pItem = nullptr;

    if (m_iPointedSlot != -1)
    {
        pItem = &CharacterMachine->Equipment[m_iPointedSlot];
        const BYTE byDurability = pItem->Durability;

        return byDurability;
    }

    pItem = m_pNewInventoryCtrl->FindItemPointedSquareIndex();
    if (pItem != nullptr)
    {
        const BYTE byDurability = pItem->Durability;
        return byDurability;
    }

    return 0;
}

int CNewUIMyInventory::GetPointedItemIndex() const
{
    if (m_iPointedSlot != -1)
    {
        return m_iPointedSlot;
    }

    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}

int CNewUIMyInventory::FindManaItemIndex() const
{
    for (int i = ITEM_LARGE_MANA_POTION; i >= ITEM_SMALL_MANA_POTION; i--)
    {
        const int iIndex = FindItemReverseIndex(i);
        if (iIndex != -1)
        {
            return iIndex;
        }
    }

    return -1;
}

int CNewUIMyInventory::FindHealingItemIndex() const
{
    for (int i = ITEM_LARGE_HEALING_POTION; i >= ITEM_APPLE; i--)
    {
        const int iIndex = FindItemReverseIndex(i);
        if (iIndex != -1)
        {
            return iIndex;
        }
    }

    return -1;
}

void CNewUIMyInventory::ResetMouseLButton()
{
    MouseLButton = false;
    MouseLButtonPop = false;
    MouseLButtonPush = false;
}

void CNewUIMyInventory::ResetMouseRButton()
{
    MouseRButton = false;
    MouseRButtonPop = false;
    MouseRButtonPush = false;
}

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
BOOL SEASON3B::CNewUIMyInventory::IsInvenItem(const short sType)
{
    BOOL bInvenItem = FALSE;

    if (FALSE
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
        || (sType == ITEM_HELPER + 128 || sType == ITEM_HELPER + 129 || sType == ITEM_HELPER + 134)
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
        || (sType >= ITEM_HELPER + 130 && sType <= ITEM_HELPER + 133)
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
        )
        bInvenItem = TRUE;

    return bInvenItem;
}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
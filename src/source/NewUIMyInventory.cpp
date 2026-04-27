// NewUIMyInventory.cpp: implementation of the CNewUIMyInventory class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMyInventory.h"
#include "NewUISystem.h"
extern bool SelectFlag;
#ifdef _EDITOR
#include "UI/Console/MuEditorConsoleUI.h"
#endif
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
#include "Guild/UIGuildInfo.h"
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

    m_ActionController.SetContext(this); 

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
    {
        return false;
    }

    if (pTempItem->Type == ITEM_DARK_HORSE_ITEM)
    {
        SocketClient->ToGameServer()->SendPetInfoRequest(PetType::DarkHorse, StorageType::Inventory, iIndex);
    }

    if (pTempItem->Type == ITEM_DARK_RAVEN_ITEM)
    {
        CreatePetDarkSpirit(Hero);
        SocketClient->ToGameServer()->SendPetInfoRequest(PetType::DarkRaven, StorageType::Inventory, iIndex);
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

bool CNewUIMyInventory::IsEquipable(int iIndex, ITEM* pItem) const
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
        if (m_pNewInventoryCtrl->RemoveItemAt(iIndex))
        {
            return;
        }

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

SEASON3B::REPAIR_MODE CNewUIMyInventory::GetRepairMode() const
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
#ifdef _EDITOR
        {
            char dbg[256];
            sprintf_s(dbg, "[DROP] SelectFlag=%d SelectXF=%.1f SelectYF=%.1f "
                           "CollisionPos=(%.0f,%.0f,%.0f)",
                      (int)SelectFlag, SelectXF, SelectYF,
                      CollisionPosition[0], CollisionPosition[1], CollisionPosition[2]);
            g_MuEditorConsoleUI.LogEditor(dbg);
        }
#endif
        if (SelectFlag)
        {
            const int iSourceIndex = pPickedItem->GetSourceLinealPos();
            const int tx = (int)(CollisionPosition[0] / TERRAIN_SCALE);
            const int ty = (int)(CollisionPosition[1] / TERRAIN_SCALE);
            if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl
                || g_pMyInventoryExt->GetOwnerOf(pPickedItem) != nullptr)
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
    mu_swprintf(strText, L"[%ls]", GlobalText[989]);
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
    mu_swprintf(strText, L"[%ls]", GlobalText[2651]);
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
    if (pItem == nullptr)
    {
        return -1;
    }

    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->FindEmptySlot(pItemAttr->Width, pItemAttr->Height);
    }

    return -1;
}

int CNewUIMyInventory::FindEmptySlotIncludingExtensions(IN int cx, IN int cy) const
{
    const int baseInventorySlot = FindEmptySlot(cx, cy);
    if (baseInventorySlot != -1)
    {
        return baseInventorySlot;
    }

    if (g_pMyInventoryExt != nullptr)
    {
        return g_pMyInventoryExt->FindEmptySlot(cx, cy);
    }

    return -1;
}

int CNewUIMyInventory::FindEmptySlotIncludingExtensions(ITEM* pItem) const
{
    if (pItem == nullptr)
    {
        return -1;
    }

    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
    return FindEmptySlotIncludingExtensions(pItemAttr->Width, pItemAttr->Height);
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
            && iSourceIndex != -1
            && !g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP))  // Don't unequip when NPC shop is open
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

    return m_ActionController.HandleInventoryActions(m_pNewInventoryCtrl);
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

bool CNewUIMyInventory::HandleInventoryActions(CNewUIInventoryCtrl* targetControl)
{
    if (g_pMyInventory)
    {
        return g_pMyInventory->m_ActionController.HandleInventoryActions(targetControl);
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
    m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);
}

void CNewUIMyInventory::UnlockMyShopButtonOpen()
{
    m_BtnMyShop.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_BtnMyShop.ChangeTextColor(RGBA(255, 255, 255, 255));
    m_BtnMyShop.UnLock();
    m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);
}

void CNewUIMyInventory::ToggleRepairMode()
{
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
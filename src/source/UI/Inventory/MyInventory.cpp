
#include "stdafx.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "I18N/All.h"
extern bool SelectFlag;
#ifdef _EDITOR
#include "UI/Console/MuEditorConsoleUI.h"
#endif
#include "UI/Dialogs/CustomMessageBox.h"
#include "Engine/AI/GOBoid.h"
#include "Render/Effects/ZzzEffect.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "GameLogic/Pets/w_PetProcess.h"
#include "Character/CSParts.h"
#include "UI/Inventory/UIJewelHarmony.h"
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "Engine/Object/ZzzInventory.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "GameLogic/Quests/CSQuest.h"
#include "Guild/UIGuildInfo.h"
#include "UI/Core/UIManager.h"
#include "GameLogic/Items/CSItemOption.h"
#include "World/MapInfra/MapManager.h"
#include "Network/Server/SocketSystem.h"
#include "World/MapInfra/PortalMgr.h"
#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#include "GameLogic/Events/Event.h"
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#include "GameLogic/Items/ChangeRingManager.h"
#include "GameLogic/Social/MonkSystem.h"
#include "Character/CharacterManager.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzInterface.h"
#include "UI/Scaling/UITransform.h"

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlDraggable.h"
#include "UI/Inventory/ItemOptionTooltipModel.h"
#include "Data/GameConfig/GameConfig.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include <cmath>

using namespace SEASON3B;
using namespace mu::ui::window;

CMyInventory::CMyInventory()
{
    m_pNewUIMng = nullptr;
    m_pNewUI3DRenderMng = nullptr;
    m_pNewInventoryCtrl = nullptr;
    m_Pos.x = m_Pos.y = 0;

    memset(&m_EquipmentSlots, 0, sizeof(EQUIPMENT_ITEM) * MAX_EQUIPMENT_INDEX);
    m_iPointedSlot = -1;

    m_MyShopMode = MYSHOP_MODE_OPEN;
    m_RepairMode = SEASON3B::REPAIR_MODE_OFF;
    m_dwStandbyItemKey = 0;

    m_bRepairEnableLevel = false;
    m_bMyShopOpen = false;
}

CMyInventory::~CMyInventory()
{
    Release();
}

bool CMyInventory::Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
    if (nullptr == pNewUIMng || nullptr == pNewUI3DRenderMng || nullptr == g_pNewItemMng)
        return false;

    // A user-dragged position (this window's own drag-end handler below) overrides the caller's
    // default column/layout position -- before anything else uses x/y, so the equipment grid
    // (created just below) and the panel both start at the same, possibly-overridden spot.
    GameConfig::GetInstance().GetWindowPosition(L"my_inventory", x, y);

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_INVENTORY, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

    m_pNewInventoryCtrl = new CInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::INVENTORY, m_pNewUI3DRenderMng, g_pNewItemMng, this, x + 15, y + 200, 8, 8, MAX_EQUIPMENT))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    m_ActionController.SetContext(this);

    SetPos(x, y);
    LoadImages();
    SetEquipmentSlotInfo();

    // Guarded like every other hybrid window's Create() (re-run on resolution change), so the
    // document/model are created once, ever.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "my_inventory",
            [this](Rml::DataModelConstructor& c, MyInventoryRmlModel& model)
            {
                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);

                c.Bind("title", &model.title);
                c.Bind("gold_text", &model.goldText);
                c.Bind("gold_color", &model.goldColor);

                c.Bind("repair_visible", &model.repairVisible);
                c.Bind("repair_tooltip", &model.repairTooltip);

                c.Bind("myshop_visible", &model.myShopVisible);
                c.Bind("myshop_mode_open", &model.myShopModeOpen);
                c.Bind("myshop_locked", &model.myShopLocked);
                c.Bind("myshop_tooltip", &model.myShopTooltip);

                c.Bind("exit_tooltip", &model.exitTooltip);
                c.Bind("expand_tooltip", &model.expandTooltip);

                c.Bind("set_option_label", &model.setOptionLabel);
                c.Bind("socket_option_label", &model.socketOptionLabel);
                c.Bind("set_option_active", &model.setOptionActive);
                c.Bind("socket_option_active", &model.socketOptionActive);

                c.Bind("item_option_tooltip_visible", &model.itemOptionTooltipVisible);
                auto tooltipLine = c.RegisterStruct<ItemOptionTooltipLineEntry>();
                tooltipLine.RegisterMember("text", &ItemOptionTooltipLineEntry::text);
                tooltipLine.RegisterMember("color_blue", &ItemOptionTooltipLineEntry::colorBlue);
                tooltipLine.RegisterMember("color_yellow", &ItemOptionTooltipLineEntry::colorYellow);
                tooltipLine.RegisterMember("color_green", &ItemOptionTooltipLineEntry::colorGreen);
                tooltipLine.RegisterMember("color_purple", &ItemOptionTooltipLineEntry::colorPurple);
                tooltipLine.RegisterMember("bold", &ItemOptionTooltipLineEntry::bold);
                c.RegisterArray<std::vector<ItemOptionTooltipLineEntry>>();
                c.Bind("item_option_tooltip_lines", &model.itemOptionTooltipLines);

                c.BindEventCallback("my_inventory_set_option_hover",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        m_RmlBinder.GetModel().setOptionHovered = true;
                    });
                c.BindEventCallback("my_inventory_set_option_unhover",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        m_RmlBinder.GetModel().setOptionHovered = false;
                    });
                c.BindEventCallback("my_inventory_socket_option_hover",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        m_RmlBinder.GetModel().socketOptionHovered = true;
                    });
                c.BindEventCallback("my_inventory_socket_option_unhover",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        m_RmlBinder.GetModel().socketOptionHovered = false;
                    });

                c.BindEventCallback("my_inventory_exit_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        if (g_pNewUISystem->IsVisible(INTERFACE_MYSHOP_INVENTORY))
                            g_pNewUISystem->Hide(INTERFACE_MYSHOP_INVENTORY);
                        g_pNewUISystem->Hide(INTERFACE_INVENTORY);
                    });
                c.BindEventCallback("my_inventory_repair_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { ToggleRepairMode(); });
                c.BindEventCallback("my_inventory_myshop_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        if (m_bMyShopLocked) return;
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
                    });
                c.BindEventCallback("my_inventory_expand_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { g_pNewUISystem->Toggle(INTERFACE_INVENTORY_EXT); });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/my_inventory.rml");

        // Drag-by-title-bar -- first real caller of UI::RmlBridge::MakeDraggable() (RmlDraggable.h
        // -- previously wired up nowhere). #title becomes the drag handle; MakeDraggable itself
        // force-sets drag/pointer-events on it, no RCSS changes needed for the mechanism to fire.
        //
        // onMove independently recomputes DockRightTransform (this window's own LayoutMode, set at
        // AddUIObj() time) rather than reading UI::Scaling::GetActiveTransform() -- this callback
        // fires from RmlUi's own event processing, not from inside this window's own
        // ScopedActiveTransform-wrapped Update()/Render() (WindowManager.cpp), so the ambient
        // active transform isn't guaranteed to be this window's. SetPos() (not a raw m_Pos write)
        // keeps the native equipment paperdoll/grid -- this window's real Type-2 companion -- in
        // sync automatically. Feeding the resulting m_Pos back through the same SyncRmlModel()
        // path every frame is what keeps this drag write from fighting root_x/root_y's own live
        // data-model binding (see this window's own scoping notes for the full round-trip
        // reasoning).
        if (m_pRmlDoc)
        {
            Rml::Element* panelEl = m_pRmlDoc->GetElementById("panel");
            Rml::Element* titleEl = m_pRmlDoc->GetElementById("title");
            if (panelEl && titleEl)
            {
                UI::RmlBridge::MakeDraggable(titleEl, panelEl,
                    [this](float newLeftPx, float newTopPx)
                    {
                        const auto transform = UI::Scaling::DockRightTransform(WindowWidth, WindowHeight);
                        const int newX = static_cast<int>(std::lround(UI::Scaling::LogicalX(transform, newLeftPx)));
                        const int newY = static_cast<int>(std::lround(UI::Scaling::LogicalY(transform, newTopPx)));
                        SetPos(newX, newY);
                    },
                    [this]()
                    {
                        // Persist immediately -- m_Pos is
                        // already the drag's final resolved position from the onMove above.
                        GameConfig::GetInstance().SetWindowPosition(L"my_inventory", m_Pos.x, m_Pos.y);
                    });
            }
        }

        // Frame background panel -- see MyInventoryBgRmlModel's own header comment (MyInventory.h)
        // for why this needs the background context instead of the main one.
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            const bool bgModelCreated = m_BgRmlBinder.Create(bgContext, "my_inventory_bg",
                [](Rml::DataModelConstructor& c, MyInventoryBgRmlModel& model)
                {
                    c.Bind("root_x", &model.rootX);
                    c.Bind("root_y", &model.rootY);
                    c.Bind("root_scale", &model.rootScale);
                });
            if (bgModelCreated)
            {
                m_pRmlBgDoc = UI::RmlBridge::LoadThemedDocument(bgContext, "Data/Interface/RmlUi/my_inventory_bg.rml");
                // Shown immediately, unlike m_pRmlDoc below -- RenderBackgroundLayer() is only ever
                // called from Render(), itself only reached while this window's own Show()/
                // visibility already gates it (CManager skips Update()/Render() for hidden
                // objects), so there's no separate "wrong scene" case to guard against here.
                if (m_pRmlBgDoc)
                    m_pRmlBgDoc->Show();
            }
        }

        // Not Show()n here -- this window starts hidden (Show(false) below) like every other
        // closable inventory-family window; m_pRmlDoc's visibility follows this object's own
        // Show()/Hide() via SyncRmlModel(), not an eager Show() at Create() time.
    }

    Show(false);
    return true;
}

void CMyInventory::Release()
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

    // See CMuHelperBar::Release()'s identical rationale -- this object's own release has no other
    // way to hide these once created.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
    if (m_pRmlBgDoc)
        m_pRmlBgDoc->Hide();
}

bool CMyInventory::EquipItem(int iIndex, std::span<const BYTE> pbyItemPacket)
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

void CMyInventory::UnequipItem(int iIndex)
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

void CMyInventory::UnequipAllItems()
{
    if (CharacterMachine)
    {
        for (int i = 0; i < MAX_EQUIPMENT_INDEX; i++)
        {
            UnequipItem(i);
        }
    }
}

bool CMyInventory::IsEquipable(int iIndex, ITEM* pItem) const
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

bool CMyInventory::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket) const
{
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
    }

    return false;
}

void CMyInventory::DeleteItem(int iIndex) const
{
    if (m_pNewInventoryCtrl)
    {
        if (m_pNewInventoryCtrl->RemoveItemAt(iIndex))
        {
            return;
        }

        CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();
        if (pPickedItem)
        {
            if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
            {
                if (pPickedItem->GetSourceLinealPos() == iIndex)
                {
                    CInventoryCtrl::DeletePickedItem();
                }
            }
        }
    }
}

void CMyInventory::DeleteAllItems() const
{
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->RemoveAllItems();
}

void CMyInventory::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    SetEquipmentSlotInfo();

    m_pNewInventoryCtrl->SetPos(x + 15, y + 200);
}

void CMyInventory::RestoreDefaultOrUserPosition(int defaultX, int defaultY)
{
    int x = defaultX;
    int y = defaultY;
    GameConfig::GetInstance().GetWindowPosition(L"my_inventory", x, y); // no-op (x/y stay at the defaults) if nothing was ever saved
    SetPos(x, y);
}

const POINT& CMyInventory::GetPos() const
{
    return m_Pos;
}

SEASON3B::REPAIR_MODE CMyInventory::GetRepairMode() const
{
    return m_RepairMode;
}

void CMyInventory::SetRepairMode(bool bRepair)
{
    if (bRepair)
    {
        m_RepairMode = SEASON3B::REPAIR_MODE_ON;
        if (m_pNewInventoryCtrl)
        {
            m_pNewInventoryCtrl->SetRepairMode(true);
        }
    }
    else
    {
        m_RepairMode = SEASON3B::REPAIR_MODE_OFF;
        if (m_pNewInventoryCtrl)
        {
            m_pNewInventoryCtrl->SetRepairMode(false);
        }
    }
}

bool CMyInventory::UpdateMouseEvent()
{
    if (m_pNewInventoryCtrl && !m_pNewInventoryCtrl->UpdateMouseEvent())
        return false;

    if (true == EquipmentWindowProcess())
        return false;
    if (true == InventoryProcess())
        return false;

    // Frame corner-close "X" -- a shared frame mechanism unrelated to the retired CButton family
    // (BtnProcess() removed); RmlUi's own Context now handles the 4 real buttons via
    // data-event-click (see Create()).
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, INTERFACE_INVENTORY))
        return false;

    CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();
    if (pPickedItem && IsPress(VK_LBUTTON)
        && !UI::Scaling::BottomHudContainsWindowPoint(WindowWidth, WindowHeight,
                                                       g_fWindowMouseX, g_fWindowMouseY))
    {
        if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) == true
            || g_pNewUISystem->IsVisible(INTERFACE_TRADE) == true
            || g_pNewUISystem->IsVisible(INTERFACE_DEVILSQUARE) == true
            || g_pNewUISystem->IsVisible(INTERFACE_BLOODCASTLE) == true
            || g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY) == true
            || g_pNewUISystem->IsVisible(INTERFACE_STORAGE) == true
            || g_pNewUISystem->IsVisible(INTERFACE_MYSHOP_INVENTORY) == true
            || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_LUCKYITEMWND) == true
            || g_pNewUISystem->IsVisible(INTERFACE_PURCHASESHOP_INVENTORY) == true)
        {
            ResetMouseLButton();
            return false;
        }

        ITEM* pItemObj = pPickedItem->GetItem();
        if (pItemObj && pItemObj->Jewel_Of_Harmony_Option != 0)
        {
            g_pSystemLogBox->AddText(I18N::Game::ReinforcedItemCanTBeDropped, TYPE_ERROR_MESSAGE);

            ResetMouseLButton();
            return false;
        }
        if (pItemObj && IsHighValueItem(pItemObj) == true)
        {
            g_pSystemLogBox->AddText(I18N::Game::YouAreNotAllowedToDropThisExpensiveItem, TYPE_ERROR_MESSAGE);
            CInventoryCtrl::BackupPickedItem();

            ResetMouseLButton();
            return false;
        }
        if (pItemObj && IsDropBan(pItemObj))
        {
            g_pSystemLogBox->AddText(I18N::Game::ThisItemCannotBeDropped, TYPE_ERROR_MESSAGE);
            CInventoryCtrl::BackupPickedItem();

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

    if (WindowProcess())
        return false;

    return true;
}

bool CMyInventory::UpdateKeyEvent()
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
            && g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_LUCKYITEMWND) == false
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

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT).Contains(MouseX, MouseY) == false)
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

bool CMyInventory::Update()
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

    SyncRmlModel();
    return true;
}

void CMyInventory::SyncRmlModel()
{
    // Shared transform group -- see MyInventoryRmlModel::rootX's own header comment. Read here
    // (inside Update(), already running within this window's own CManager-pushed
    // ScopedActiveTransform) rather than a HUD-specific helper, since this window is positioned
    // via ordinary SetPos()/m_Pos, not a fixed HUD anchor.
    //
    // m_Pos is a REFERENCE-space coordinate, not a real screen pixel -- every other reference-
    // space value in this codebase (e.g. MainFrameWindow.cpp's bars_left/top) is resolved via
    // screenPos = refPos*scale + offset before being handed to RmlUi's data-style-left/top (which
    // takes literal 'px', not reference pixels). Binding raw m_Pos here leaves the panel at the
    // wrong screen position at any non-1:1 offset -- the same double-offset-shaped bug class the
    // CSprite/WindowGeometry retrofit hit elsewhere in this tier.
    const auto transform = UI::Scaling::GetActiveTransform();
    const float rootX = static_cast<float>(m_Pos.x) * transform.scaleX + transform.offsetX;
    const float rootY = static_cast<float>(m_Pos.y) * transform.scaleY + transform.offsetY;

    if (m_pRmlBgDoc)
    {
        auto& bg = m_BgRmlBinder.GetModel();
        bg.rootX = rootX;
        bg.rootY = rootY;
        bg.rootScale = transform.scaleX;
        m_BgRmlBinder.MarkDirty("root_x");
        m_BgRmlBinder.MarkDirty("root_y");
        m_BgRmlBinder.MarkDirty("root_scale");

        // A closable window needs its own visibility gate, same reasoning as every persistent
        // HUD pilot's SyncDocVisibility() -- RenderBackgroundLayer() renders whatever's currently
        // shown in the shared background context regardless of which window called it, so
        // Hide()/Show() here is what actually keeps this one invisible while the inventory window
        // itself is closed.
        if (IsVisible()) m_pRmlBgDoc->Show(); else m_pRmlBgDoc->Hide();
    }

    if (!m_pRmlDoc) return;
    if (IsVisible()) m_pRmlDoc->Show(); else m_pRmlDoc->Hide();

    {
        auto& model = m_RmlBinder.GetModel();
        model.rootX = rootX;
        model.rootY = rootY;
        model.rootScale = transform.scaleX;
        m_RmlBinder.MarkDirty("root_x");
        m_RmlBinder.MarkDirty("root_y");
        m_RmlBinder.MarkDirty("root_scale");
    }

    auto syncBool = [this](bool MyInventoryRmlModel::* field, const char* boundName, bool value)
    {
        if (m_RmlBinder.GetModel().*field != value) { m_RmlBinder.GetModel().*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncText = [this](Rml::String MyInventoryRmlModel::* field, const char* boundName, const Rml::String& value)
    {
        if (m_RmlBinder.GetModel().*field != value) { m_RmlBinder.GetModel().*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncWide = [&](Rml::String MyInventoryRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        syncText(field, boundName, StringUtils::WideToNarrow(text));
    };

    syncWide(&MyInventoryRmlModel::title, "title", I18N::Game::Inventory);

    const DWORD dwZen = CharacterMachine->Gold;
    wchar_t goldBuf[256] = { 0, };
    ConvertGold(dwZen, goldBuf);
    syncWide(&MyInventoryRmlModel::goldText, "gold_text", goldBuf);

    // getGoldColor() packs (A<<24)+(R<<16)+(G<<8)+B -- unpack into an rgba() CSS string.
    const unsigned int goldArgb = getGoldColor(dwZen);
    char goldColorBuf[32];
    snprintf(goldColorBuf, sizeof(goldColorBuf), "rgba(%u,%u,%u,%u)",
        (goldArgb >> 16) & 0xFF, (goldArgb >> 8) & 0xFF, goldArgb & 0xFF, (goldArgb >> 24) & 0xFF);
    syncText(&MyInventoryRmlModel::goldColor, "gold_color", Rml::String(goldColorBuf));

    // Same 7-window gate RenderButtons()/BtnProcess() used to duplicate separately for
    // visibility vs. interactivity -- RmlUi's data-class-hidden means one flag now covers both.
    const bool otherWindowOpen = g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP)
        || g_pNewUISystem->IsVisible(INTERFACE_TRADE)
        || g_pNewUISystem->IsVisible(INTERFACE_DEVILSQUARE)
        || g_pNewUISystem->IsVisible(INTERFACE_BLOODCASTLE)
        || g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_LUCKYITEMWND)
        || g_pNewUISystem->IsVisible(INTERFACE_STORAGE);

    syncBool(&MyInventoryRmlModel::repairVisible, "repair_visible", m_bRepairEnableLevel && !otherWindowOpen);
    syncWide(&MyInventoryRmlModel::repairTooltip, "repair_tooltip", I18N::Game::RepairL);

    syncBool(&MyInventoryRmlModel::myShopVisible, "myshop_visible", m_bMyShopOpen && !otherWindowOpen);
    syncBool(&MyInventoryRmlModel::myShopModeOpen, "myshop_mode_open", m_MyShopMode == MYSHOP_MODE_OPEN);
    syncBool(&MyInventoryRmlModel::myShopLocked, "myshop_locked", m_bMyShopLocked);
    syncWide(&MyInventoryRmlModel::myShopTooltip, "myshop_tooltip",
        m_MyShopMode == MYSHOP_MODE_OPEN ? I18N::Game::OpenPersonalStoreS : I18N::Game::ClosePersonalStoreS);

    syncWide(&MyInventoryRmlModel::exitTooltip, "exit_tooltip", I18N::Game::CloseIV);
    syncWide(&MyInventoryRmlModel::expandTooltip, "expand_tooltip", I18N::Game::OpenExpandedInventoryK);

    // Set/Socket option header labels + shared hover tooltip -- see
    // ItemOptionTooltipLineEntry's own comment (MyInventory.h). Label text is static per-language,
    // but still routed through syncWide (change-checked) rather than bound once at Create() time,
    // matching every other I18N-sourced field in this model.
    wchar_t setOptionLabelBuf[128];
    mu_swprintf(setOptionLabelBuf, L"[%ls]", I18N::Game::SetOption);
    syncWide(&MyInventoryRmlModel::setOptionLabel, "set_option_label", setOptionLabelBuf);
    syncBool(&MyInventoryRmlModel::setOptionActive, "set_option_active", g_csItemOption.IsAncientSetEquipped());

    wchar_t socketOptionLabelBuf[128];
    mu_swprintf(socketOptionLabelBuf, L"[%ls]", I18N::Game::SocketOption);
    syncWide(&MyInventoryRmlModel::socketOptionLabel, "socket_option_label", socketOptionLabelBuf);
    syncBool(&MyInventoryRmlModel::socketOptionActive, "socket_option_active", g_SocketItemMgr.IsSocketSetOptionEnabled());

    // Shared tooltip -- one hover target at a time (setOptionHovered/socketOptionHovered are set
    // by the RmlUi hover event callbacks below, mutually exclusive same as the legacy left/right
    // header-strip halves were). BuildXxxTooltipModel() is the same content resolution
    // (RenderSetOptionList()/RenderToolTipForSocketSetOption()'s own former native-drawing bodies
    // used) with no drawing -- only the destination (RmlUi vs. legacy TextList) differs, same
    // convention as MainFrameWindow.cpp's skill tooltip (UI::Skills::Tooltip::BuildModelForSlot).
    auto& model = m_RmlBinder.GetModel();
    bool tooltipBuilt = false;
    UI::Inventory::Tooltip::Model tooltipModel;
    if (model.setOptionHovered)
        tooltipBuilt = g_csItemOption.BuildSetOptionTooltipModel(tooltipModel);
    else if (model.socketOptionHovered)
        tooltipBuilt = g_SocketItemMgr.BuildSocketOptionTooltipModel(tooltipModel);

    if (tooltipBuilt)
    {
        model.itemOptionTooltipLines.clear();
        for (int i = 0; i < tooltipModel.count; ++i)
        {
            const UI::Inventory::Tooltip::Line& src = tooltipModel.lines[i];
            ItemOptionTooltipLineEntry line;
            line.text = StringUtils::WideToNarrow(src.text);
            line.colorBlue = (src.color == UI::Inventory::Tooltip::LineColor::Blue);
            line.colorYellow = (src.color == UI::Inventory::Tooltip::LineColor::Yellow);
            line.colorGreen = (src.color == UI::Inventory::Tooltip::LineColor::Green);
            line.colorPurple = (src.color == UI::Inventory::Tooltip::LineColor::Purple);
            line.bold = src.isBold;
            model.itemOptionTooltipLines.push_back(line);
        }
        model.itemOptionTooltipVisible = true;
        m_RmlBinder.MarkDirty("item_option_tooltip_lines");
        m_RmlBinder.MarkDirty("item_option_tooltip_visible");
    }
    else
    {
        syncBool(&MyInventoryRmlModel::itemOptionTooltipVisible, "item_option_tooltip_visible", false);
    }
}

bool CMyInventory::Render()
{
    EnableAlphaTest();

    // Frame background panel moved to RmlUi -- see MyInventoryBgRmlModel's own header comment
    // (MyInventory.h) for why this goes through the background context. Must run
    // before RenderEquippedItem()/m_pNewInventoryCtrl->Render() below only in the sense that
    // both of those are 2D overlays on top of this frame -- the actual ordering constraint (this
    // panel painting behind the *3D* icons) is enforced by RenderBackgroundLayer() itself running
    // earlier in the frame than Render3D()'s C3DRenderMng pass, not by this call's position here.
    RmlUiRuntime::Instance().RenderBackgroundLayer();

    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->Render();

    RenderEquippedItem();
    DisableAlphaBlend();
    return true;
}

void CMyInventory::Render3D()
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

bool CMyInventory::IsVisible() const
{
    return CObject::IsVisible();
}

void CMyInventory::OpenningProcess()
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

void CMyInventory::ClosingProcess()
{
    m_pNewInventoryCtrl->BackupPickedItem();
    RepairEnable = 0;
    SetRepairMode(false);
}

float CMyInventory::GetLayerDepth()
{
    return 4.2f;
}

CInventoryCtrl* CMyInventory::GetInventoryCtrl() const
{
    return m_pNewInventoryCtrl;
}

ITEM* CMyInventory::FindItem(int iLinealPos) const
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItem(iLinealPos);
    return nullptr;
}

ITEM* CMyInventory::FindItemByKey(DWORD dwKey) const
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItemByKey(dwKey);
    return nullptr;
}

int CMyInventory::FindItemIndex(short int siType, int iLevel) const
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItemIndex(siType, iLevel);
    return -1;
}

int CMyInventory::FindItemReverseIndex(short sType, int iLevel) const
{
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->FindItemReverseIndex(sType, iLevel);
    }

    return -1;
}

int CMyInventory::FindEmptySlot(IN int cx, IN int cy) const
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindEmptySlot(cx, cy);
    return -1;
}

int CMyInventory::FindEmptySlot(ITEM* pItem) const
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

int CMyInventory::FindEmptySlotIncludingExtensions(IN int cx, IN int cy) const
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

int CMyInventory::FindEmptySlotIncludingExtensions(ITEM* pItem) const
{
    if (pItem == nullptr)
    {
        return -1;
    }

    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
    return FindEmptySlotIncludingExtensions(pItemAttr->Width, pItemAttr->Height);
}

void CMyInventory::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD /*dwParamB*/)
{
    if (pClass)
    {
        auto* pMyInventory = (CMyInventory*)(pClass);
        pMyInventory->RenderItemToolTip(dwParamA);
    }
}

void CMyInventory::SetStandbyItemKey(DWORD dwItemKey)
{
    m_dwStandbyItemKey = dwItemKey;
}

DWORD CMyInventory::GetStandbyItemKey() const
{
    return m_dwStandbyItemKey;
}

int CMyInventory::GetStandbyItemIndex() const
{
    if (ITEM* pItem = GetStandbyItem())
    {
        return m_pNewInventoryCtrl->GetIndexByItem(pItem);
    }
    return -1;
}

ITEM* CMyInventory::GetStandbyItem() const
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItemByKey(m_dwStandbyItemKey);
    return nullptr;
}

void CMyInventory::CreateEquippingEffect(ITEM* pItem)
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

void CMyInventory::DeleteEquippingEffectBug(ITEM* pItem)
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

void CMyInventory::DeleteEquippingEffect()
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

void CMyInventory::SetEquipmentSlotInfo()
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

void CMyInventory::LoadImages() const
{
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
}

void CMyInventory::UnloadImages()
{
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
}

void CMyInventory::RenderEquippedItem()
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

            unsigned int overlayColor;
            if (pEquipmentItemSlot->Durability <= 0)
                overlayColor = 0x40FF0000u;
            else if (pEquipmentItemSlot->Durability <= (iMaxDurability * 0.2f))
                overlayColor = 0x40FF2600u;
            else if (pEquipmentItemSlot->Durability <= (iMaxDurability * 0.3f))
                overlayColor = 0x40FF8000u;
            else if (pEquipmentItemSlot->Durability <= (iMaxDurability * 0.5f))
                overlayColor = 0x40FFFF00u;
            else if (IsEquipable(i, pEquipmentItemSlot) == false)
                overlayColor = 0x40FF0000u;
            else
            {
                continue;
            }

            EnableAlphaTest();
            RenderColorQuadARGB(m_EquipmentSlots[i].x + 1, m_EquipmentSlots[i].y,
                m_EquipmentSlots[i].width - 4, m_EquipmentSlots[i].height - 4, overlayColor);
        }
    }

    if (CInventoryCtrl::GetPickedItem() && m_iPointedSlot != -1)
    {
        ITEM* pItemObj = CInventoryCtrl::GetPickedItem()->GetItem();
        const ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[m_iPointedSlot];
        if (pItemObj && (pEquipmentItemSlot->Type != -1 || false == IsEquipable(m_iPointedSlot, pItemObj))
            && !((gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER) && (m_iPointedSlot == EQUIPMENT_GLOVES)))
        {
            EnableAlphaTest();
            RenderColorQuadARGB(m_EquipmentSlots[m_iPointedSlot].x + 1, m_EquipmentSlots[m_iPointedSlot].y,
                m_EquipmentSlots[m_iPointedSlot].width - 4, m_EquipmentSlots[m_iPointedSlot].height - 4,
                0x66E61A1Au);
        }
    }

    if (m_iPointedSlot != -1 && m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, m_iPointedSlot, 0);
    }
}

bool CMyInventory::EquipmentWindowProcess()
{
    if (m_iPointedSlot != -1 && IsRelease(VK_LBUTTON))
    {
        if (CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem())
        {
            ITEM* pItemObj = pPickedItem->GetItem();
            const int iSourceIndex = pPickedItem->GetSourceLinealPos();
            const int iTargetIndex = m_iPointedSlot;
            if (pItemObj->bPeriodItem && pItemObj->bExpiredPeriod)
            {
                g_pSystemLogBox->AddText(I18N::Game::CanTWearItem, mu::ui::window::TYPE_ERROR_MESSAGE);
                CInventoryCtrl::BackupPickedItem();

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
                    g_pSystemLogBox->AddText(I18N::Game::CanTWearItem, TYPE_ERROR_MESSAGE);
                    CInventoryCtrl::BackupPickedItem();

                    ResetMouseLButton();
                    return false;
                }
            }

            if (IsEquipable(iTargetIndex, pItemObj))
            {
                const STORAGE_TYPE sourceType = pPickedItem->GetSourceStorageType();

                if (sourceType == STORAGE_TYPE::INVENTORY && iSourceIndex == iTargetIndex)
                {
                    CInventoryCtrl::BackupPickedItem();
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
            if (GetRepairMode() == SEASON3B::REPAIR_MODE_ON)
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
                        if (CInventoryCtrl::CreatePickedItem(nullptr, pEquippedItem))
                        {
                            UnequipItem(m_iPointedSlot);
                        }
                    }
                }
                else
                {
                    if (CInventoryCtrl::CreatePickedItem(nullptr, pEquippedItem))
                    {
                        UnequipItem(m_iPointedSlot);
                    }
                }
            }
        }
    }

    if (IsRelease(VK_RBUTTON))
    {
        const CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();

        const int iSourceIndex = m_iPointedSlot;
        if (GetRepairMode() != SEASON3B::REPAIR_MODE_ON && EquipmentItem == false
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
                    if (CInventoryCtrl::CreatePickedItem(nullptr, pEquippedItem))
                    {
                        CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();
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
bool CMyInventory::InventoryProcess() const
{
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT).Contains(MouseX, MouseY) == false)
    {
        return false;
    }

    if (m_pNewInventoryCtrl == nullptr)
    {
        return false;
    }

    return m_ActionController.HandleInventoryActions(m_pNewInventoryCtrl);
}

bool CMyInventory::WindowProcess()
{
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT).Contains(MouseX, MouseY) == false)
    {
        return false;
    }

    if (IsPress(VK_RBUTTON))
    {
        ResetMouseRButton();
    }

    return true;
}

void CMyInventory::RenderItemToolTip(int iSlotIndex) const
{
    if (m_iPointedSlot != -1)
    {
        ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[iSlotIndex];
        if (pEquipmentItemSlot->Type != -1)
        {
            const int iTargetX = m_EquipmentSlots[iSlotIndex].x + m_EquipmentSlots[iSlotIndex].width / 2;
            const int iTargetY = m_EquipmentSlots[iSlotIndex].y + m_EquipmentSlots[iSlotIndex].height / 2;

            pEquipmentItemSlot->bySelectedSlotIndex = iSlotIndex;

            if (m_RepairMode == SEASON3B::REPAIR_MODE_OFF)
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

bool CMyInventory::CanRegisterItemHotKey(int iType)
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

bool CMyInventory::HandleInventoryActions(CInventoryCtrl* targetControl)
{
    if (g_pMyInventory)
    {
        return g_pMyInventory->m_ActionController.HandleInventoryActions(targetControl);
    }
    return false;
}

bool CMyInventory::CanOpenMyShopInterface()
{
    if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP)
        || g_pNewUISystem->IsVisible(INTERFACE_STORAGE)
        || g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_LUCKYITEMWND)
        || g_pNewUISystem->IsVisible(INTERFACE_TRADE)
        || gMapManager.IsCursedTemple()
        )
    {
        return false;
    }
    return true;
}

bool CMyInventory::IsRepairEnableLevel() const
{
    return m_bRepairEnableLevel;
}

void CMyInventory::SetRepairEnableLevel(bool bOver)
{
    m_bRepairEnableLevel = bOver;
}

void CMyInventory::ChangeMyShopButtonStateOpen()
{
    m_MyShopMode = MYSHOP_MODE_OPEN;
}

void CMyInventory::ChangeMyShopButtonStateClose()
{
    m_MyShopMode = MYSHOP_MODE_CLOSE;
}

void CMyInventory::LockMyShopButtonOpen()
{
    m_bMyShopLocked = true;
}

void CMyInventory::UnlockMyShopButtonOpen()
{
    m_bMyShopLocked = false;
}

void CMyInventory::ToggleRepairMode()
{
    if (m_RepairMode == SEASON3B::REPAIR_MODE_OFF)
    {
        SetRepairMode(true);
    }
    else if (m_RepairMode == SEASON3B::REPAIR_MODE_ON)
    {
        SetRepairMode(false);
    }
}

bool CMyInventory::IsItem(short int siType, bool bcheckPick) const
{
    if (bcheckPick == true)
    {
        const CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();

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

int CMyInventory::GetNumItemByKey(DWORD dwItemKey) const
{
    return m_pNewInventoryCtrl->GetNumItemByKey(dwItemKey);
}

int CMyInventory::GetNumItemByType(short sItemType) const
{
    return m_pNewInventoryCtrl->GetNumItemByType(sItemType);
}

BYTE CMyInventory::GetDurabilityPointedItem() const
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

int CMyInventory::GetPointedItemIndex() const
{
    if (m_iPointedSlot != -1)
    {
        return m_iPointedSlot;
    }

    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}

int CMyInventory::FindManaItemIndex() const
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

int CMyInventory::FindHealingItemIndex() const
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

void CMyInventory::ResetMouseLButton()
{
    MouseLButton = false;
    MouseLButtonPop = false;
    MouseLButtonPush = false;
}

void CMyInventory::ResetMouseRButton()
{
    MouseRButton = false;
    MouseRButtonPop = false;
    MouseRButtonPush = false;
}

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
BOOL mu::ui::window::CMyInventory::IsInvenItem(const short sType)
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

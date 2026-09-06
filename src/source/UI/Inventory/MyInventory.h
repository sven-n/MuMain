
#if !defined(AFX_NEWUIMYINVENTORY_H__74DA6D7A_CF5A_46E9_8C72_9D38F0DC95EC__INCLUDED_)
#define AFX_NEWUIMYINVENTORY_H__74DA6D7A_CF5A_46E9_8C72_9D38F0DC95EC__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Core/Window3DRenderMng.h"
#include "UI/Inventory/InventoryActionController.h"
#include "GameLogic/Items/IInventoryActionContext.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include <span>
#include <vector>
#include "Core/Globals/_enum.h"

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CMyInventory
        : public CObject
        , public I3DRenderObj
        , public SEASON3B::IInventoryActionContext
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_INVENTORY_MYSHOP_OPEN_BTN = BITMAP_MYSHOPINTERFACE_NEW_PERSONALINVENTORY_BEGIN + 1,
            IMAGE_INVENTORY_MYSHOP_CLOSE_BTN = BITMAP_MYSHOPINTERFACE_NEW_PERSONALINVENTORY_BEGIN + 2,
            IMAGE_INVENTORY_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_INVENTORY_BACK_TOP = BITMAP_INTERFACE_NEW_PERSONALINVENTORY_BEGIN,	//"newui_item_back01.tga"
            IMAGE_INVENTORY_BACK_TOP2,	//"newui_item_back04.tga"
            IMAGE_INVENTORY_BACK_LEFT,	//"newui_item_back02-L.tga"
            IMAGE_INVENTORY_BACK_RIGHT,	//"newui_item_back02-R.tga"
            IMAGE_INVENTORY_BACK_BOTTOM,	//"newui_item_back03.tga"
            IMAGE_INVENTORY_ITEM_BOOT,	//"newui_item_boots.tga"
            IMAGE_INVENTORY_ITEM_HELM,	//"newui_item_cap.tga"
            IMAGE_INVENTORY_ITEM_FAIRY,	//"newui_item_fairy.tga"
            IMAGE_INVENTORY_ITEM_WING,	//"newui_item_wing.tga"
            IMAGE_INVENTORY_ITEM_RIGHT,	//"newui_item_weapon(L).tga"
            IMAGE_INVENTORY_ITEM_LEFT,	//"newui_item_weapon(R).tga"
            IMAGE_INVENTORY_ITEM_ARMOR,	//"newui_item_upper.tga"
            IMAGE_INVENTORY_ITEM_GLOVES,	//"newui_item_gloves.tga"
            IMAGE_INVENTORY_ITEM_PANTS,	//"newui_item_lower.tga"
            IMAGE_INVENTORY_ITEM_RING,	//"newui_item_ring.tga"
            IMAGE_INVENTORY_ITEM_NECKLACE,	//"newui_item_necklace.tga"
            IMAGE_INVENTORY_MONEY,	//"newui_item_money.tga"
            IMAGE_INVENTORY_EXIT_BTN, //"newui_exit_00.tga"
            IMAGE_INVENTORY_REPAIR_BTN, //"newui_repair_00.tga"
            IMAGE_INVENTORY_EXPAND_BTN, //"newui_expansion_btn.tga"
        };

        enum MYSHOP_MODE
        {
            MYSHOP_MODE_OPEN = 0,
            MYSHOP_MODE_CLOSE,
        };

    private:
        static constexpr float INVENTORY_WIDTH  = 190.0f;
        static constexpr float INVENTORY_HEIGHT = 429.0f;

        typedef struct tagEQUIPMENT_ITEM
        {
            int x, y;
            int width, height;
            DWORD dwBgImage;
        } EQUIPMENT_ITEM;

        CManager* m_pNewUIMng;
        C3DRenderMng* m_pNewUI3DRenderMng;
        CInventoryCtrl* m_pNewInventoryCtrl;
        CInventoryActionController m_ActionController;
        POINT m_Pos;

        EQUIPMENT_ITEM m_EquipmentSlots[MAX_EQUIPMENT_INDEX];
        int	m_iPointedSlot;

        MYSHOP_MODE m_MyShopMode;
        SEASON3B::REPAIR_MODE m_RepairMode;
        DWORD m_dwStandbyItemKey;

        bool m_bRepairEnableLevel;
        bool m_bMyShopOpen;
        bool m_bMyShopLocked = false;

        // Stage 3: one shared hover tooltip for both the Set Option and Socket Option header
        // labels (mutually exclusive within this window, same "one shared tooltip element for
        // multiple hover targets" convention as MainFrameRmlModel::skillTooltipLines). Converted
        // from UI::Inventory::Tooltip::Model (ItemOptionTooltipModel.h) the same way
        // MainFrameWindow.cpp converts UI::Skills::Tooltip::Model -- see SyncRmlModel()'s own
        // comment.
        struct ItemOptionTooltipLineEntry
        {
            Rml::String text;
            bool colorBlue = false;
            bool colorYellow = false;
            bool colorGreen = false;
            bool colorPurple = false;
            bool bold = false;
        };

        // Stage 1 (H7): window frame/title/gold/buttons -- see docs/ui-target-architecture.md
        // Section H item 7. Equipment paperdoll and the inventory grid stay fully native this
        // stage (their icons are live 3D model renders, RenderItem3D()/Render3D() -- permanently
        // native, Section E -- and their chrome is deferred to a later stage).
        struct MyInventoryRmlModel
        {
            // Shared transform group for every element in this document -- this window is
            // movable (SetPos()), not HUD-anchored, so this is sourced from
            // UI::Scaling::GetActiveTransform() (read in Update(), inside this window's own
            // CManager-pushed ScopedActiveTransform) rather than a HUD transform helper. Same
            // "one shared group, not a per-element binding" shape as
            // MainFrameRmlModel::barsLeft (MainFrameWindow.h).
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;

            Rml::String title;
            Rml::String goldText;
            Rml::String goldColor; // "rgba(r,g,b,a)" -- mirrors getGoldColor()'s amount-tier color

            bool repairVisible = false;
            Rml::String repairTooltip;

            bool myShopVisible = false;
            bool myShopModeOpen = true;
            bool myShopLocked = false;
            Rml::String myShopTooltip;

            Rml::String exitTooltip;
            Rml::String expandTooltip;

            // Stage 3 (H7): Set/Socket option header labels + shared hover tooltip -- see
            // ItemOptionTooltipLineEntry's own comment above and SyncRmlModel()'s.
            Rml::String setOptionLabel;
            Rml::String socketOptionLabel;
            bool setOptionActive = false;    // IsAncientSetEquipped() -- label color
            bool socketOptionActive = false; // IsSocketSetOptionEnabled() -- label color
            bool setOptionHovered = false;
            bool socketOptionHovered = false;

            bool itemOptionTooltipVisible = false;
            std::vector<ItemOptionTooltipLineEntry> itemOptionTooltipLines;
        };
        RmlModelBinder<MyInventoryRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // RmlUi-behind-3D-icons -- the frame background panel sits underneath the paperdoll's and
        // the inventory grid's live 3D icons (Render3D(), CInventoryCtrl::Render3D()), both of
        // which keep rendering via the C3DRenderMng/I3DRenderObj interleave. RmlUi's "main"
        // context always renders last in the frame, so the panel goes through
        // RmlUiRuntime::GetBackgroundContext()/RenderBackgroundLayer() instead (the same mechanism
        // CMainFrameWindow::RenderLeftFrame() already proved) -- called from Render(), which
        // already runs before either Render3D() call this same frame. Separate document/model from
        // m_pRmlDoc's own -- RmlUi data models are per-context.
        struct MyInventoryBgRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        };
        RmlModelBinder<MyInventoryBgRmlModel> m_BgRmlBinder;
        Rml::ElementDocument* m_pRmlBgDoc = nullptr;

        void SyncRmlModel();

    public:
        CMyInventory();
        virtual ~CMyInventory();

        bool Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng, int x, int y);
        void Release();

        bool EquipItem(int iIndex, std::span<const BYTE> pbyItemPacket);
        void UnequipItem(int iIndex);
        void UnequipAllItems();

        SEASON3B::REPAIR_MODE GetRepairMode() const override;
        bool IsEquipable(int iIndex, ITEM* pItem) const override;
        void ResetMouseRButton() override;
        void ResetMouseLButton() override;
        int  FindEmptySlot(ITEM* pItem) const override;
        bool IsRepairEnableLevel() const override;

        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket) const;
        void DeleteItem(int iIndex) const;
        void DeleteAllItems() const;

        void SetPos(int x, int y);
        const POINT& GetPos() const;

        // Replaces a bare SetPos(defaultX, defaultY) at the "return to idle position" call sites
        // (WindowSystem.cpp) -- restores the user's dragged position instead, if one was ever
        // saved (GameConfig::GetWindowPosition(), written by this window's own drag-end handler),
        // so a user customization survives Character-info/etc. temporarily shifting this window
        // sideways to avoid overlap and then closing again. Those sideways collision-avoidance
        // shifts themselves are unrelated and stay unconditional -- see this window's own
        // architecture-principles.md §10/§11 reassessment (STATUS.md) for why the two are not the
        // same kind of position write.
        void RestoreDefaultOrUserPosition(int defaultX, int defaultY);

        void SetRepairMode(bool bRepair);

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        BOOL IsInvenItem(const short sType);
#endif

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void Render3D();

        bool IsVisible() const;

        void OpenningProcess();
        void ClosingProcess();

        float GetLayerDepth();

        CInventoryCtrl* GetInventoryCtrl() const;

        ITEM* FindItem(int iLinealPos) const;
        ITEM* FindItemByKey(DWORD dwKey) const;
        int   FindItemIndex(short int siType, int iLevel = -1) const;
        int   FindItemReverseIndex(short sType, int iLevel = -1) const;
        int   FindEmptySlot(IN int cx, IN int cy) const;
        int   FindEmptySlotIncludingExtensions(IN int cx, IN int cy) const;
        int   FindEmptySlotIncludingExtensions(ITEM* pItem) const;
        bool  IsItem(short int siType, bool bcheckPick = false) const;
        int   GetNumItemByKey(DWORD dwItemKey) const;
        int   GetNumItemByType(short sItemType) const;
        BYTE  GetDurabilityPointedItem() const;
        int   GetPointedItemIndex() const;
        int   FindManaItemIndex() const;
        int   FindHealingItemIndex() const;

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

        void  SetStandbyItemKey(DWORD dwItemKey);
        DWORD GetStandbyItemKey() const;
        int   GetStandbyItemIndex() const;
        ITEM* GetStandbyItem() const;

        void SetRepairEnableLevel(bool bOver);

        void ChangeMyShopButtonStateOpen();
        void ChangeMyShopButtonStateClose();
        void LockMyShopButtonOpen();
        void UnlockMyShopButtonOpen();

        void CreateEquippingEffect(ITEM* pItem);

        static bool CanRegisterItemHotKey(int iType);
        static bool HandleInventoryActions(CInventoryCtrl* targetControl);

        // Public to allow refresh when resolution changes
        void SetEquipmentSlotInfo();

    protected:
        void DeleteEquippingEffect();
        void DeleteEquippingEffectBug(ITEM* pItem);

    private:
        void LoadImages() const;
        void UnloadImages();

        void RenderEquippedItem();

        bool EquipmentWindowProcess();
        bool InventoryProcess() const;
        bool WindowProcess();

        void RenderItemToolTip(int iSlotIndex) const;
        bool CanOpenMyShopInterface();
        void ToggleRepairMode();
    };

} // namespace mu::ui::window

#endif // !defined(AFX_NEWUIMYINVENTORY_H__74DA6D7A_CF5A_46E9_8C72_9D38F0DC95EC__INCLUDED_)

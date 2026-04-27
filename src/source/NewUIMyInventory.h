// NewUIPersonalInventory.h: interface for the CNewUIMyInventory class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIMYINVENTORY_H__74DA6D7A_CF5A_46E9_8C72_9D38F0DC95EC__INCLUDED_)
#define AFX_NEWUIMYINVENTORY_H__74DA6D7A_CF5A_46E9_8C72_9D38F0DC95EC__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUI3DRenderMng.h"
#include "NewUIButton.h"
#include "NewUIInventoryActionController.h"
#include "IInventoryActionContext.h"
#include <span>
#include "_enum.h"

namespace SEASON3B
{
    class CNewUIMyInventory
        : public CNewUIObj
        , public INewUI3DRenderObj
        , public IInventoryActionContext
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_INVENTORY_MYSHOP_OPEN_BTN = BITMAP_MYSHOPINTERFACE_NEW_PERSONALINVENTORY_BEGIN + 1,
            IMAGE_INVENTORY_MYSHOP_CLOSE_BTN = BITMAP_MYSHOPINTERFACE_NEW_PERSONALINVENTORY_BEGIN + 2,
            IMAGE_INVENTORY_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
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
        enum ITEM_OPTION
        {
            ITEM_SET_OPTION = 1,
            ITEM_SOCKET_SET_OPTION = 2,
        };

        static constexpr float INVENTORY_WIDTH  = 190.0f;
        static constexpr float INVENTORY_HEIGHT = 429.0f;

        typedef struct tagEQUIPMENT_ITEM
        {
            int x, y;
            int width, height;
            DWORD dwBgImage;
        } EQUIPMENT_ITEM;

        CNewUIManager* m_pNewUIMng;
        CNewUI3DRenderMng* m_pNewUI3DRenderMng;
        CNewUIInventoryCtrl* m_pNewInventoryCtrl;
        CNewUIInventoryActionController m_ActionController;
        POINT m_Pos;

        EQUIPMENT_ITEM m_EquipmentSlots[MAX_EQUIPMENT_INDEX];
        int	m_iPointedSlot;

        CNewUIButton m_BtnRepair;
        CNewUIButton m_BtnExit;
        CNewUIButton m_BtnMyShop;
        CNewUIButton m_BtnExpand;

        MYSHOP_MODE m_MyShopMode;
        SEASON3B::REPAIR_MODE m_RepairMode;
        DWORD m_dwStandbyItemKey;

        bool m_bRepairEnableLevel;
        bool m_bMyShopOpen;

    public:
        CNewUIMyInventory();
        virtual ~CNewUIMyInventory();

        bool Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y);
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

        CNewUIInventoryCtrl* GetInventoryCtrl() const;

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
        static bool HandleInventoryActions(CNewUIInventoryCtrl* targetControl);

        // Public to allow refresh when resolution changes
        void SetEquipmentSlotInfo();

    protected:
        void DeleteEquippingEffect();
        void DeleteEquippingEffectBug(ITEM* pItem);
        void SetButtonInfo();

    private:
        void LoadImages() const;
        void UnloadImages();

        void RenderFrame() const;
        void RenderSetOption();
        void RenderSocketOption();
        void RenderEquippedItem();
        void RenderButtons();
        void RenderInventoryDetails() const;

        bool EquipmentWindowProcess();
        bool InventoryProcess() const;
        bool BtnProcess();

        void RenderItemToolTip(int iSlotIndex) const;
        bool CanOpenMyShopInterface();
        void ToggleRepairMode();
    };

} // namespace SEASON3B

#endif // !defined(AFX_NEWUIMYINVENTORY_H__74DA6D7A_CF5A_46E9_8C72_9D38F0DC95EC__INCLUDED_)
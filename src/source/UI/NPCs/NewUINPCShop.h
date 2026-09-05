// NewUINPCShop.h: interface for the CNPCShop class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUINPCSHOP_H__EEE639A8_C89E_47B3_8DBA_22560F102D98__INCLUDED_)
#define AFX_NEWUINPCSHOP_H__EEE639A8_C89E_47B3_8DBA_22560F102D98__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Inventory/NewUIInventoryCtrl.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Widgets/NewUIButton.h"

namespace mu::ui::window
{
    class CNPCShop : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_NPCSHOP_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_NPCSHOP_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP2,
            IMAGE_NPCSHOP_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_NPCSHOP_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_NPCSHOP_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_NPCSHOP_BTN_REPAIR = CMyInventory::IMAGE_INVENTORY_REPAIR_BTN,
            IMAGE_NPCSHOP_REPAIR_MONEY = BITMAP_INTERFACE_NEW_NPCSHOP_BEGIN,
        };

        enum
        {
            NPCSHOP_POS_X = 260,
            NPCSHOP_POS_Y = 0,
            SHOP_STATE_BUYNSELL = 1,
            SHOP_STATE_REPAIR = 2,
        };

    private:
        enum
        {
            NPCSHOP_WIDTH = 190,
            NPCSHOP_HEIGHT = 429,
        };

        CManager* m_pNewUIMng;
        CInventoryCtrl* m_pNewInventoryCtrl;
        POINT m_Pos;

        DWORD m_dwShopState;
        int m_iTaxRate;
        bool m_bRepairShop;
        bool m_bIsNPCShopOpen;

        CButton m_BtnRepair;
        CButton m_BtnRepairAll;

        DWORD m_dwStandbyItemKey;

        bool m_bSellingItem;

    public:
        CNPCShop();
        virtual ~CNPCShop();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 2.5f

        void SetTaxRate(int iTaxRate);
        int GetTaxRate();

        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);

        void OpenningProcess();
        void DeleteAllItems();

        void ClosingProcess();
        void SetRepairShop(bool bRepair);
        bool IsRepairShop();
        void ToggleState();
        DWORD GetShopState();

        int GetPointedItemIndex();

        //. Exporting Functions
        void SetStandbyItemKey(DWORD dwItemKey);
        DWORD GetStandbyItemKey() const;
        int GetStandbyItemIndex();
        ITEM* GetStandbyItem();

        void SetSellingItem(bool bFlag);
        bool IsSellingItem();

    private:
        void Init();
        void SetButtonInfo();

        void LoadImages();
        void UnloadImages();

        bool InventoryProcess();
        bool BtnProcess();
        bool WindowProcess();

        void RenderFrame();
        void RenderTexts();
        void RenderButton();
        void RenderRepairMoney();
    };
}

#endif // !defined(AFX_NEWUINPCSHOP_H__EEE639A8_C89E_47B3_8DBA_22560F102D98__INCLUDED_)

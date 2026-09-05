// NewUIPurchaseShopInventory.h: interface for the CPurchaseShopInventory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIPURCHASESHOPINVENTORY_H__5D417396_5ACE_46AF_9477_102810B6A1B8__INCLUDED_)
#define AFX_NEWUIPURCHASESHOPINVENTORY_H__5D417396_5ACE_46AF_9477_102810B6A1B8__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Inventory/NewUIInventoryCtrl.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Widgets/NewUIButton.h"
#include "UI/Inventory/NewUIMyShopInventory.h"

namespace mu::ui::window
{
    class CPurchaseShopInventory : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_MSGBOX_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_INVENTORY_BACK_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_INVENTORY_BACK_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_INVENTORY_BACK_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_INVENTORY_BACK_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_INVENTORY_EXIT_BTN = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_MYSHOPINVENTORY_EDIT = CMyShopInventory::IMAGE_MYSHOPINVENTORY_EDIT,
        };

    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

    public:
        CPurchaseShopInventory();
        virtual ~CPurchaseShopInventory();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();
        void SetPos(int x, int y);
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        void ClosingProcess();

    public:
        float GetLayerDepth();	//. 3.2f
        CInventoryCtrl* GetInventoryCtrl() const;
        const int GetShopCharacterIndex();
        const std::wstring& GetTitleText();
        const int GetSourceIndex();
        int GetPointedItemIndex();

    public:
        void ChangeShopCharacterIndex(int index);
        void ChangeTitleText(wchar_t* text);
        void ChangeSourceIndex(int sindex);

    public:
        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);
        void DeleteItem(int iIndex);
        ITEM* FindItem(int iLinealPos);
        int GetItemInventoryIndex(ITEM* pItem);

    private:
        void LoadImages();
        void UnloadImages();

    private:
        bool PurchaseShopInventoryProcess();
        bool WindowProcess();

    private:
        void RenderFrame();
        void RenderTextInfo();

    private:
        CManager* m_pNewUIMng;
        CInventoryCtrl* m_pNewInventoryCtrl;
        POINT					m_Pos;
        int						m_ShopCharacterIndex;
       std::wstring		m_TitleText;
        CButton* m_Button;
        int						m_SourceIndex;
    };

    inline
        void CPurchaseShopInventory::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        void CPurchaseShopInventory::ChangeShopCharacterIndex(int index)
    {
        m_ShopCharacterIndex = index;
    }

    inline
        void CPurchaseShopInventory::ChangeTitleText(wchar_t* text)
    {
        m_TitleText = text;
    }

    inline
        void CPurchaseShopInventory::ChangeSourceIndex(int sindex)
    {
        m_SourceIndex = sindex;
    }

    inline
        CInventoryCtrl* CPurchaseShopInventory::GetInventoryCtrl() const
    {
        return m_pNewInventoryCtrl;
    }

    inline
        float CPurchaseShopInventory::GetLayerDepth()
    {
        return 3.2f;
    }

    inline
        const int CPurchaseShopInventory::GetShopCharacterIndex()
    {
        return m_ShopCharacterIndex;
    }

    inline
        const std::wstring& CPurchaseShopInventory::GetTitleText()
    {
        return m_TitleText;
    }

    inline
        const int CPurchaseShopInventory::GetSourceIndex()
    {
        return m_SourceIndex;
    }
};

#endif // !defined(AFX_NEWUIPURCHASESHOPINVENTORY_H__5D417396_5ACE_46AF_9477_102810B6A1B8__INCLUDED_)

// NewUIPurchaseShopInventory.h: interface for the CNewUIPurchaseShopInventory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIPURCHASESHOPINVENTORY_H__5D417396_5ACE_46AF_9477_102810B6A1B8__INCLUDED_)
#define AFX_NEWUIPURCHASESHOPINVENTORY_H__5D417396_5ACE_46AF_9477_102810B6A1B8__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"
#include "NewUIMyShopInventory.h"

namespace SEASON3B
{
    class CNewUIPurchaseShopInventory : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_MSGBOX_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_INVENTORY_BACK_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_INVENTORY_BACK_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_INVENTORY_BACK_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_INVENTORY_BACK_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_INVENTORY_EXIT_BTN = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_MYSHOPINVENTORY_EDIT = CNewUIMyShopInventory::IMAGE_MYSHOPINVENTORY_EDIT,
        };

    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

    public:
        CNewUIPurchaseShopInventory();
        virtual ~CNewUIPurchaseShopInventory();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();
        void SetPos(int x, int y);
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        void ClosingProcess();

    public:
        float GetLayerDepth();	//. 3.2f
        CNewUIInventoryCtrl* GetInventoryCtrl() const;
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

    private:
        void RenderFrame();
        void RenderTextInfo();

    private:
        CNewUIManager* m_pNewUIMng;
        CNewUIInventoryCtrl* m_pNewInventoryCtrl;
        POINT					m_Pos;
        int						m_ShopCharacterIndex;
       std::wstring		m_TitleText;
        CNewUIButton* m_Button;
        int						m_SourceIndex;
    };

    inline
        void CNewUIPurchaseShopInventory::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        void CNewUIPurchaseShopInventory::ChangeShopCharacterIndex(int index)
    {
        m_ShopCharacterIndex = index;
    }

    inline
        void CNewUIPurchaseShopInventory::ChangeTitleText(wchar_t* text)
    {
        m_TitleText = text;
    }

    inline
        void CNewUIPurchaseShopInventory::ChangeSourceIndex(int sindex)
    {
        m_SourceIndex = sindex;
    }

    inline
        CNewUIInventoryCtrl* CNewUIPurchaseShopInventory::GetInventoryCtrl() const
    {
        return m_pNewInventoryCtrl;
    }

    inline
        float CNewUIPurchaseShopInventory::GetLayerDepth()
    {
        return 3.2f;
    }

    inline
        const int CNewUIPurchaseShopInventory::GetShopCharacterIndex()
    {
        return m_ShopCharacterIndex;
    }

    inline
        const std::wstring& CNewUIPurchaseShopInventory::GetTitleText()
    {
        return m_TitleText;
    }

    inline
        const int CNewUIPurchaseShopInventory::GetSourceIndex()
    {
        return m_SourceIndex;
    }
};

#endif // !defined(AFX_NEWUIPURCHASESHOPINVENTORY_H__5D417396_5ACE_46AF_9477_102810B6A1B8__INCLUDED_)

// NewUIMyShopInventory.h: interface for the CNewUIMyShopInventory class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIMYSHOPINVENTORY_H__A0C3DD4A_C4D5_4CF2_9702_DF54540DB6FD__INCLUDED_)
#define AFX_NEWUIMYSHOPINVENTORY_H__A0C3DD4A_C4D5_4CF2_9702_DF54540DB6FD__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"
#include "UIControls.h"

namespace SEASON3B
{
    class CNewUIMyShopInventory : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_MYSHOPINVENTORY_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_MYSHOPINVENTORY_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_MYSHOPINVENTORY_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_MYSHOPINVENTORY_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_MYSHOPINVENTORY_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_MYSHOPINVENTORY_EXIT_BTN = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,

            IMAGE_MYSHOPINVENTORY_EDIT = BITMAP_MYSHOPINTERFACE_NEW_PERSONALINVENTORY_BEGIN,
            IMAGE_MYSHOPINVENTORY_OPEN,
            IMAGE_MYSHOPINVENTORY_CLOSE,
        };

        enum SHOPTYEP
        {
            PERSONALSHOPSALE = 0,
            PERSONALSHOPPURCHASE,
        };

    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

        enum
        {
            MYSHOPINVENTORY_EXIT = 0,
            MYSHOPINVENTORY_OPEN,
            MYSHOPINVENTORY_CLOSE,
            MYSHOPINVENTORY_MAXBUTTONCOUNT,
        };

        CNewUIManager* m_pNewUIMng;
        CNewUIInventoryCtrl* m_pNewInventoryCtrl;
        POINT m_Pos;

    public:
        CNewUIMyShopInventory();
        virtual ~CNewUIMyShopInventory();
        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();
        void SetPos(int x, int y);
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void ClosingProcess();
        float GetLayerDepth();	//. 3.2f

        CNewUIInventoryCtrl* GetInventoryCtrl() const;

    public:
        void ChangeSourceIndex(int sindex);
        const int GetSourceIndex();
        void ChangeTargetIndex(int tindex);
        const int GetTargetIndex();
        ITEM* FindItem(int iLinealPos);
        int GetItemInventoryIndex(ITEM* pItem);
        void ChangeEditBox(const UISTATES type);
        void ChangeTitle(wchar_t* titletext);
        void GetTitle(wchar_t* titletext);
        void SetTitle(wchar_t* titletext);
        void ChangePersonal(bool state);
        const bool IsEnablePersonalShop() const;
        void OpenButtonLock();
        void OpenButtonUnLock();
        int GetPointedItemIndex();
        void ResetSubject();
        bool IsEnableInputValueTextBox();
        void SetInputValueTextBox(bool bIsEnable);

    public:
        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);
        void DeleteItem(int iIndex);
        void DeleteAllItems();

    private:
        void LoadImages();
        void UnloadImages();

    private:
        bool MyShopInventoryProcess();

    private:
        void RenderFrame();
        void RenderTextInfo();

    private:
        int					m_TargetIndex;
        int					m_SourceIndex;
        bool				m_EnablePersonalShop;
        bool				m_bIsEnableInputValueTextBox;

        CNewUIButton* m_Button;
        CUITextInputBox* m_EditBox;
    };

    inline
        void CNewUIMyShopInventory::ChangeTitle(wchar_t* titletext)
    {
        m_EditBox->SetText(titletext);
    }

    inline
        void CNewUIMyShopInventory::ChangeSourceIndex(int sindex)
    {
        m_SourceIndex = sindex;
    }

    inline
        void CNewUIMyShopInventory::ChangeTargetIndex(int tindex)
    {
        m_TargetIndex = tindex;
    }

    inline
        float CNewUIMyShopInventory::GetLayerDepth()
    {
        return 3.2f;
    }

    inline
        CNewUIInventoryCtrl* CNewUIMyShopInventory::GetInventoryCtrl() const
    {
        return m_pNewInventoryCtrl;
    }

    inline
        const int CNewUIMyShopInventory::GetSourceIndex()
    {
        return m_SourceIndex;
    }

    inline
        const int CNewUIMyShopInventory::GetTargetIndex()
    {
        return m_TargetIndex;
    }
}

#endif // !defined(AFX_NEWUIMYSHOPINVENTORY_H__A0C3DD4A_C4D5_4CF2_9702_DF54540DB6FD__INCLUDED_)

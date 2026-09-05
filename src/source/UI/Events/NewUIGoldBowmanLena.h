// NewUIGoldBowmanLena.h: interface for the NewUIGoldBowmanLena class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "UI/Core/NewUIManager.h"
#include "UI/Inventory/NewUIInventoryCtrl.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "Guild/NewUIGuildMakeWindow.h"

namespace mu::ui::window
{
    class CGoldBowmanLena : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GBL_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_GBL_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP2,
            IMAGE_GBL_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GBL_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GBL_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GBL_EXCHANGEBTN = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_GBL_BTN_SERIAL = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_GBL_BTN_EXIT = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        };

    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

    public:
        CManager* m_pNewUIMng;
        CButton			m_BtnRegister;
        CButton			m_BtnExit;
        POINT					m_Pos;

    public:
        CGoldBowmanLena();
        virtual ~CGoldBowmanLena();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);
        const POINT& GetPos();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	// 3.4f

    public:
        void OpeningProcess();
        void ClosingProcess();

    private:
        void LoadImages();
        void UnloadImages();
        void RenderFrame();
        void RenderTexts();
        void RendeerButton();
        void Render3D();
    };

    inline
        void CGoldBowmanLena::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        const POINT& CGoldBowmanLena::GetPos()
    {
        return m_Pos;
    }
};

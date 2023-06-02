// NewUIGoldBowmanLena.h: interface for the NewUIGoldBowmanLena class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "NewUIManager.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIGuildMakeWindow.h"

namespace SEASON3B
{
    class CNewUIGoldBowmanLena : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GBL_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_GBL_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,
            IMAGE_GBL_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GBL_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GBL_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GBL_EXCHANGEBTN = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_GBL_BTN_SERIAL = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_GBL_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        };

    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

    public:
        CNewUIManager* m_pNewUIMng;
        CNewUIButton			m_BtnRegister;
        CNewUIButton			m_BtnExit;
        POINT					m_Pos;

    public:
        CNewUIGoldBowmanLena();
        virtual ~CNewUIGoldBowmanLena();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
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
        void CNewUIGoldBowmanLena::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        const POINT& CNewUIGoldBowmanLena::GetPos()
    {
        return m_Pos;
    }
};

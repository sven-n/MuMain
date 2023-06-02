// NewUIGoldBowmanWindow.h: interface for the CNewUIGoldBowmanWindow class.
//////////////////////////////////////////////////////////////////////

#pragma once
#include "NewUIManager.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIGuildMakeWindow.h"

namespace SEASON3B
{
    class CNewUIGoldBowmanWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GB_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_GB_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,
            IMAGE_GB_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GB_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GB_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GB_EXCHANGEBTN = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_GB_BTN_SERIAL = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_GB_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_GB_EDITBOX = CNewUIGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX,
        };

    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

    public:
        CNewUIManager* m_pNewUIMng;
        CUITextInputBox* m_EditBox;
        CNewUIButton			m_BtnSerial;
        CNewUIButton			m_BtnExit;
        POINT					m_Pos;

    public:
        CNewUIGoldBowmanWindow();
        virtual ~CNewUIGoldBowmanWindow();

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
        void ChangeEditBox(const UISTATES type);
    };

    inline
        void CNewUIGoldBowmanWindow::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        const POINT& CNewUIGoldBowmanWindow::GetPos()
    {
        return m_Pos;
    }
};

// NewUIGoldBowmanWindow.h: interface for the CGoldBowmanWindow class.
//////////////////////////////////////////////////////////////////////

#pragma once
#include "UI/Core/NewUIManager.h"
#include "UI/Inventory/NewUIInventoryCtrl.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "Guild/NewUIGuildMakeWindow.h"

namespace mu::ui::window
{
    class CGoldBowmanWindow : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GB_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_GB_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP2,
            IMAGE_GB_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GB_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GB_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GB_EXCHANGEBTN = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_GB_BTN_SERIAL = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_GB_BTN_EXIT = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_GB_EDITBOX = CGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX,
        };

    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

    public:
        CManager* m_pNewUIMng;
        CUITextInputBox* m_EditBox;
        CButton			m_BtnSerial;
        CButton			m_BtnExit;
        POINT					m_Pos;

    public:
        CGoldBowmanWindow();
        virtual ~CGoldBowmanWindow();

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
        void ChangeEditBox(const UISTATES type);
    };

    inline
        void CGoldBowmanWindow::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        const POINT& CGoldBowmanWindow::GetPos()
    {
        return m_Pos;
    }
};

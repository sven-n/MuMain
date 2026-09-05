// NewGatemanWindow.h: interface for the CNewGatemanWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWGATEMANWINDOW_H__F53A1778_D5C8_4EB6_BE74_0A9A16D1FF26__INCLUDED_)
#define AFX_NEWGATEMANWINDOW_H__F53A1778_D5C8_4EB6_BE74_0A9A16D1FF26__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Widgets/NewUIButton.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Combat/NewUICastleWindow.h"

namespace mu::ui::window
{
    class CGatemanWindow : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GATEMANWINDOW_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_GATEMANWINDOW_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_GATEMANWINDOW_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GATEMANWINDOW_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GATEMANWINDOW_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GATEMANWINDOW_EXIT_BTN = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_GATEMANWINDOW_BUTTON = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,

            IMAGE_GATEMANWINDOW_SCROLL_UP_BTN = CCastleWindow::IMAGE_CASTLEWINDOW_SCROLL_UP_BTN,
            IMAGE_GATEMANWINDOW_SCROLL_DOWN_BTN = CCastleWindow::IMAGE_CASTLEWINDOW_SCROLL_DOWN_BTN,
        };

    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

        CManager* m_pNewUIMng;
        POINT m_Pos;

        CButton m_BtnExit;

        CButton m_BtnEnter;			// 입장 버튼
        CButton m_BtnSet;				// 입장료 설정 버튼
        CButton m_BtnFeeUp;			// 입장료 up
        CButton m_BtnFeeDn;			// 입장료 down

    public:
        CGatemanWindow();
        virtual ~CGatemanWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        void OpeningProcess();
        void ClosingProcess();

        float GetLayerDepth();	//. 5.0f

    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        bool BtnProcess();

        void InitButton(CButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption);

        void UpdateGuildMasterMode();
        void UpdateGuildMemeberMode();
        void UpdateGuestMode();

        void RenderGuildMasterMode();
        void RenderGuildMemeberMode();
        void RenderGuestMode();
    };
}

#endif // !defined(AFX_NEWGATEMANWINDOW_H__F53A1778_D5C8_4EB6_BE74_0A9A16D1FF26__INCLUDED_)

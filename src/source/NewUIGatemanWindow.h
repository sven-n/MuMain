// NewGatemanWindow.h: interface for the CNewGatemanWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWGATEMANWINDOW_H__F53A1778_D5C8_4EB6_BE74_0A9A16D1FF26__INCLUDED_)
#define AFX_NEWGATEMANWINDOW_H__F53A1778_D5C8_4EB6_BE74_0A9A16D1FF26__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIButton.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUICastleWindow.h"

namespace SEASON3B
{
    class CNewUIGatemanWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GATEMANWINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_GATEMANWINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_GATEMANWINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GATEMANWINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GATEMANWINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GATEMANWINDOW_EXIT_BTN = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_GATEMANWINDOW_BUTTON = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,

            IMAGE_GATEMANWINDOW_SCROLL_UP_BTN = CNewUICastleWindow::IMAGE_CASTLEWINDOW_SCROLL_UP_BTN,
            IMAGE_GATEMANWINDOW_SCROLL_DOWN_BTN = CNewUICastleWindow::IMAGE_CASTLEWINDOW_SCROLL_DOWN_BTN,
        };

    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

        CNewUIManager* m_pNewUIMng;
        POINT m_Pos;

        CNewUIButton m_BtnExit;

        CNewUIButton m_BtnEnter;			// 입장 버튼
        CNewUIButton m_BtnSet;				// 입장료 설정 버튼
        CNewUIButton m_BtnFeeUp;			// 입장료 up
        CNewUIButton m_BtnFeeDn;			// 입장료 down

    public:
        CNewUIGatemanWindow();
        virtual ~CNewUIGatemanWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
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

        void InitButton(CNewUIButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption);

        void UpdateGuildMasterMode();
        void UpdateGuildMemeberMode();
        void UpdateGuestMode();

        void RenderGuildMasterMode();
        void RenderGuildMemeberMode();
        void RenderGuestMode();
    };
}

#endif // !defined(AFX_NEWGATEMANWINDOW_H__F53A1778_D5C8_4EB6_BE74_0A9A16D1FF26__INCLUDED_)

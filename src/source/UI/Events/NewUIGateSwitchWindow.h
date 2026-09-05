// NewUIGateSwitchWindow.h: interface for the CGateSwitchWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIGATESWITCHWINDOW_H__89BA066C_7870_4064_B38E_F2F5AA919F9F__INCLUDED_)
#define AFX_NEWUIGATESWITCHWINDOW_H__89BA066C_7870_4064_B38E_F2F5AA919F9F__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Widgets/NewUIButton.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"

namespace mu::ui::window
{
    class CGateSwitchWindow : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GATESWITCHWINDOW_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_GATESWITCHWINDOW_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_GATESWITCHWINDOW_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GATESWITCHWINDOW_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GATESWITCHWINDOW_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GATESWITCHWINDOW_EXIT_BTN = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_GATESWITCHWINDOW_BUTTON = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,

            IMAGE_GATESWITCHWINDOW_TABLE_TOP_LEFT = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,	//. newui_item_table01(L).tga (14,14)
            IMAGE_GATESWITCHWINDOW_TABLE_TOP_RIGHT = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,	//. newui_item_table01(R).tga (14,14)
            IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_LEFT = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,	//. newui_item_table02(L).tga (14,14)
            IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_RIGHT = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
            IMAGE_GATESWITCHWINDOW_TABLE_TOP_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
            IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
            IMAGE_GATESWITCHWINDOW_TABLE_LEFT_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
            IMAGE_GATESWITCHWINDOW_TABLE_RIGHT_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)
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
        CButton m_BtnOpen;			// 열기 버튼

    public:
        CGateSwitchWindow();
        virtual ~CGateSwitchWindow();

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

        void RenderOutlineUpper(float fPos_x, float fPos_y, float fWidth, float fHeight);
        void RenderOutlineLower(float fPos_x, float fPos_y, float fWidth, float fHeight);
    };
}
#endif // !defined(AFX_NEWUIGATESWITCHWINDOW_H__89BA066C_7870_4064_B38E_F2F5AA919F9F__INCLUDED_)

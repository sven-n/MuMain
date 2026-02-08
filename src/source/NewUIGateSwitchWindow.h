// NewUIGateSwitchWindow.h: interface for the CNewUIGateSwitchWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIGATESWITCHWINDOW_H__89BA066C_7870_4064_B38E_F2F5AA919F9F__INCLUDED_)
#define AFX_NEWUIGATESWITCHWINDOW_H__89BA066C_7870_4064_B38E_F2F5AA919F9F__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIButton.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"

namespace SEASON3B
{
    class CNewUIGateSwitchWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GATESWITCHWINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_GATESWITCHWINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_GATESWITCHWINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GATESWITCHWINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GATESWITCHWINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GATESWITCHWINDOW_EXIT_BTN = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_GATESWITCHWINDOW_BUTTON = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,

            IMAGE_GATESWITCHWINDOW_TABLE_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,	//. newui_item_table01(L).tga (14,14)
            IMAGE_GATESWITCHWINDOW_TABLE_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,	//. newui_item_table01(R).tga (14,14)
            IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,	//. newui_item_table02(L).tga (14,14)
            IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
            IMAGE_GATESWITCHWINDOW_TABLE_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
            IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
            IMAGE_GATESWITCHWINDOW_TABLE_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
            IMAGE_GATESWITCHWINDOW_TABLE_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)
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
        CNewUIButton m_BtnOpen;			// 열기 버튼

    public:
        CNewUIGateSwitchWindow();
        virtual ~CNewUIGateSwitchWindow();

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

        void RenderOutlineUpper(float fPos_x, float fPos_y, float fWidth, float fHeight);
        void RenderOutlineLower(float fPos_x, float fPos_y, float fWidth, float fHeight);
    };
}
#endif // !defined(AFX_NEWUIGATESWITCHWINDOW_H__89BA066C_7870_4064_B38E_F2F5AA919F9F__INCLUDED_)

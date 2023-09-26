// NewUICastleWindow.h: interface for the CNewUICastleWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICASTLEWINDOW_H__C0C75F67_38B5_48C4_98EF_DC0F4E9EB866__INCLUDED_)
#define AFX_NEWUICASTLEWINDOW_H__C0C75F67_38B5_48C4_98EF_DC0F4E9EB866__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIButton.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIGuildInfoWindow.h"
#include "NewUINPCShop.h"
#include "NewUIMyQuestInfoWindow.h"

namespace SEASON3B
{
    class CNewUICastleWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_CASTLEWINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_CASTLEWINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_CASTLEWINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_CASTLEWINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_CASTLEWINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_CASTLEWINDOW_EXIT_BTN = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_CASTLEWINDOW_TAB_BTN = CNewUIGuildInfoWindow::IMAGE_GUILDINFO_TAB_BUTTON,
            IMAGE_CASTLEWINDOW_LINE = CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
            IMAGE_CASTLEWINDOW_BUTTON = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
            IMAGE_CASTLEWINDOW_TABLE_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,	//. newui_item_table01(L).tga (14,14)
            IMAGE_CASTLEWINDOW_TABLE_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,	//. newui_item_table01(R).tga (14,14)
            IMAGE_CASTLEWINDOW_TABLE_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,	//. newui_item_table02(L).tga (14,14)
            IMAGE_CASTLEWINDOW_TABLE_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
            IMAGE_CASTLEWINDOW_TABLE_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
            IMAGE_CASTLEWINDOW_TABLE_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
            IMAGE_CASTLEWINDOW_TABLE_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
            IMAGE_CASTLEWINDOW_TABLE_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)
            IMAGE_CASTLEWINDOW_MONEY = CNewUINPCShop::IMAGE_NPCSHOP_REPAIR_MONEY,
            IMAGE_CASTLEWINDOW_SCROLL_UP_BTN = BITMAP_INTERFACE_NEW_CASTLE_WINDOW_BEGIN,
            IMAGE_CASTLEWINDOW_SCROLL_DOWN_BTN,
        };
        enum CASTLE_MSGBOX_REQUEST
        {
            CASTLE_MSGREQ_NULL,
            CASTLE_MSGREQ_BUY_GATE,
            CASTLE_MSGREQ_REPAIR_GATE,
            CASTLE_MSGREQ_UPGRADE_GATE_HP,
            CASTLE_MSGREQ_UPGRADE_GATE_DEFENSE,
            CASTLE_MSGREQ_BUY_STATUE,
            CASTLE_MSGREQ_REPAIR_STATUE,
            CASTLE_MSGREQ_UPGRADE_STATUE_HP,
            CASTLE_MSGREQ_UPGRADE_STATUE_DEFENSE,
            CASTLE_MSGREQ_UPGRADE_STATUE_RECOVER,
            CASTLE_MSGREQ_APPLY_TAX,
            CASTLE_MSGREQ_WITHDRAW,
        };
    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };
        enum CURR_OPEN_TAB_BUTTON
        {
            TAB_GATE_MANAGING,
            TAB_STATUE_MANAGING,
            TAB_TAX_MANAGING,
            TAB_CASTLE_MIX
        };

        CNewUIManager* m_pNewUIMng;
        POINT m_Pos;

        CNewUIRadioGroupButton	m_TabBtn;
        int						m_iNumCurOpenTab;
        int						m_iCurrMsgBoxRequest;

        CNewUIButton m_BtnExit;

        CNewUIButton m_BtnBuy;
        CNewUIButton m_BtnRepair;
        CNewUIButton m_BtnUpgradeHP;
        CNewUIButton m_BtnUpgradeDefense;
        CNewUIButton m_BtnUpgradeRecover;
        CNewUIButton m_BtnApplyTax;
        CNewUIButton m_BtnWithdraw;
        CNewUIButton m_BtnChaosTaxUp;
        CNewUIButton m_BtnChaosTaxDn;
        CNewUIButton m_BtnNPCTaxUp;
        CNewUIButton m_BtnNPCTaxDn;
    public:
        CNewUICastleWindow();
        virtual ~CNewUICastleWindow();

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

        int GetCurrMsgBoxRequest() { return m_iCurrMsgBoxRequest; }
    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        bool BtnProcess();

        void SetCurrMsgBoxRequest(int iMsgBoxRequest) { m_iCurrMsgBoxRequest = iMsgBoxRequest; }
        void InsertComma(wchar_t* pszText, DWORD dwNumber);
        void InsertComma64(wchar_t* pszText, __int64 iNumber);

        void UpdateGateManagingTab();
        void UpdateStatueManagingTab();
        void UpdateTaxManagingTab();

        void RenderGateManagingTab();
        void RenderStatueManagingTab();
        void RenderTaxManagingTab();

        void RenderCastleItem(int nPosX, int nPosY, LPPMSG_NPCDBLIST pInfo);
        void InitButton(CNewUIButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption);

        void RenderOutlineUpper(float fPos_x, float fPos_y, float fWidth, float fHeight);
        void RenderOutlineLower(float fPos_x, float fPos_y, float fWidth, float fHeight);
    };
}

#endif // !defined(AFX_NEWUICASTLEWINDOW_H__C0C75F67_38B5_48C4_98EF_DC0F4E9EB866__INCLUDED_)

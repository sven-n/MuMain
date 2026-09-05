// NewUICastleWindow.h: interface for the CCastleWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICASTLEWINDOW_H__C0C75F67_38B5_48C4_98EF_DC0F4E9EB866__INCLUDED_)
#define AFX_NEWUICASTLEWINDOW_H__C0C75F67_38B5_48C4_98EF_DC0F4E9EB866__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Widgets/NewUIButton.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "Guild/NewUIGuildInfoWindow.h"
#include "UI/NPCs/NewUINPCShop.h"
#include "UI/Quests/NewUIMyQuestInfoWindow.h"

namespace mu::ui::window
{
    class CCastleWindow : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_CASTLEWINDOW_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_CASTLEWINDOW_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_CASTLEWINDOW_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_CASTLEWINDOW_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_CASTLEWINDOW_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_CASTLEWINDOW_EXIT_BTN = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_CASTLEWINDOW_TAB_BTN = CGuildInfoWindow::IMAGE_GUILDINFO_TAB_BUTTON,
            IMAGE_CASTLEWINDOW_LINE = CMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
            IMAGE_CASTLEWINDOW_BUTTON = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
            IMAGE_CASTLEWINDOW_TABLE_TOP_LEFT = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,	//. newui_item_table01(L).tga (14,14)
            IMAGE_CASTLEWINDOW_TABLE_TOP_RIGHT = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,	//. newui_item_table01(R).tga (14,14)
            IMAGE_CASTLEWINDOW_TABLE_BOTTOM_LEFT = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,	//. newui_item_table02(L).tga (14,14)
            IMAGE_CASTLEWINDOW_TABLE_BOTTOM_RIGHT = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
            IMAGE_CASTLEWINDOW_TABLE_TOP_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
            IMAGE_CASTLEWINDOW_TABLE_BOTTOM_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
            IMAGE_CASTLEWINDOW_TABLE_LEFT_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
            IMAGE_CASTLEWINDOW_TABLE_RIGHT_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)
            IMAGE_CASTLEWINDOW_MONEY = CNPCShop::IMAGE_NPCSHOP_REPAIR_MONEY,
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

        CManager* m_pNewUIMng;
        POINT m_Pos;

        CRadioGroupButton	m_TabBtn;
        int						m_iNumCurOpenTab;
        int						m_iCurrMsgBoxRequest;

        CButton m_BtnExit;

        CButton m_BtnBuy;
        CButton m_BtnRepair;
        CButton m_BtnUpgradeHP;
        CButton m_BtnUpgradeDefense;
        CButton m_BtnUpgradeRecover;
        CButton m_BtnApplyTax;
        CButton m_BtnWithdraw;
        CButton m_BtnChaosTaxUp;
        CButton m_BtnChaosTaxDn;
        CButton m_BtnNPCTaxUp;
        CButton m_BtnNPCTaxDn;
    public:
        CCastleWindow();
        virtual ~CCastleWindow();

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
        void InitButton(CButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption);

        void RenderOutlineUpper(float fPos_x, float fPos_y, float fWidth, float fHeight);
        void RenderOutlineLower(float fPos_x, float fPos_y, float fWidth, float fHeight);
    };
}

#endif // !defined(AFX_NEWUICASTLEWINDOW_H__C0C75F67_38B5_48C4_98EF_DC0F4E9EB866__INCLUDED_)

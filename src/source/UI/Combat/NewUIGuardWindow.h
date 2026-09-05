// NewUIGuardWindow.h: interface for the CGuardWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIGUARDWINDOW_H__0FFE1FE7_59B6_47D8_B79C_7E3DD9912E14__INCLUDED_)
#define AFX_NEWUIGUARDWINDOW_H__0FFE1FE7_59B6_47D8_B79C_7E3DD9912E14__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Widgets/NewUIButton.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "Guild/NewUIGuildInfoWindow.h"

namespace mu::ui::window
{
    class CGuardWindow : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GUARDWINDOW_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_GUARDWINDOW_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_GUARDWINDOW_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GUARDWINDOW_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GUARDWINDOW_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GUARDWINDOW_EXIT_BTN = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_GUARDWINDOW_TAB_BTN = CGuildInfoWindow::IMAGE_GUILDINFO_TAB_BUTTON,
            IMAGE_GUARDWINDOW_BUTTON = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,

            IMAGE_GUARDWINDOW_TOP_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,
            IMAGE_GUARDWINDOW_BOTTOM_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,
            IMAGE_GUARDWINDOW_LEFT_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,
            IMAGE_GUARDWINDOW_RIGHT_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,

            IMAGE_GUARDWINDOW_SCROLL_TOP = CChatLogWindow::IMAGE_SCROLL_TOP,
            IMAGE_GUARDWINDOW_SCROLL_MIDDLE = CChatLogWindow::IMAGE_SCROLL_MIDDLE,
            IMAGE_GUARDWINDOW_SCROLL_BOTTOM = CChatLogWindow::IMAGE_SCROLL_BOTTOM,
            IMAGE_GUARDWINDOW_SCROLLBAR_ON = CChatLogWindow::IMAGE_SCROLLBAR_ON,
            IMAGE_GUARDWINDOW_SCROLLBAR_OFF = CChatLogWindow::IMAGE_SCROLLBAR_OFF,
        };

    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };
        enum CURR_OPEN_TAB_BUTTON
        {
            TAB_SIEGE_INFO,
            TAB_REGISTER,
            TAB_REGISTER_INFO
        };

        CManager* m_pNewUIMng;
        POINT m_Pos;

        CRadioGroupButton	m_TabBtn;
        int						m_iNumCurOpenTab;		// ���� �����ִ� �ǹ�ư��ȣ

        CButton m_BtnExit;

        CButton m_BtnProclaim;			// ���� ���� ��ư
        CButton m_BtnRegister;			// ǥ�� ��� ��ư
        CButton m_BtnGiveUp;			// ���� ���� ��ư

        // ������ ��� ����Ʈ
        CUIBCDeclareGuildListBox	m_DeclareGuildListBox;
        // Ȯ���� ��� ����Ʈ
        CUIBCGuildListBox			m_GuildListBox;

        // UI ��� ��
        CASTLESIEGE_STATE	m_eTimeType;

        wchar_t		m_szOwnerGuild[8 + 1];
        wchar_t		m_szOwnerGuildMaster[10 + 1];

        WORD		m_wStartYear;
        BYTE		m_byStartMonth;
        BYTE		m_byStartDay;
        BYTE		m_byStartHour;
        BYTE		m_byStartMinute;
        WORD		m_wEndYear;
        BYTE		m_byEndMonth;
        BYTE		m_byEndDay;
        BYTE		m_byEndHour;
        BYTE		m_byEndMinute;
        WORD		m_wSiegeStartYear;
        BYTE		m_bySiegeStartMonth;
        BYTE		m_bySiegeStartDay;
        BYTE		m_bySiegeStartHour;
        BYTE		m_bySiegeStartMinute;
        DWORD		m_dwStateLeftSec;

    public:
        CGuardWindow();
        virtual ~CGuardWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        void RenderScrollBarFrame(int iPos_x, int iPos_y, int iHeight);
        void RenderScrollBar(int iPos_x, int iPos_y, BOOL bIsClicked);

        void OpeningProcess();
        void ClosingProcess();

        float GetLayerDepth();	//. 5.0f

        void SetData(LPPMSG_ANS_CASTLESIEGESTATE Info);	// �������� �޾� ȭ�� ǥ�� ����

        void AddDeclareGuildList(wchar_t* szGuildName, int nMarkCount, BYTE byIsGiveUP, BYTE bySeqNum);
        void ClearDeclareGuildList();
        void SortDeclareGuildList();
        void AddGuildList(wchar_t* szGuildName, BYTE byCsJoinSide, BYTE byGuildInvolved, int iGuildScore);
        void ClearGuildList();

    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        bool BtnProcess();

        void InitButton(CButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* const* pCaptionSlot);

        void UpdateSeigeInfoTab();
        void UpdateRegisterTab();
        void UpdateRegisterInfoTab();

        void RenderSeigeInfoTab();
        void RenderRegisterTab();
        void RenderRegisterInfoTab();
    };
}

#endif // !defined(AFX_NEWUIGUARDWINDOW_H__0FFE1FE7_59B6_47D8_B79C_7E3DD9912E14__INCLUDED_)

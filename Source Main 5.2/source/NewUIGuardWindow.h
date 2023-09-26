// NewUIGuardWindow.h: interface for the CNewUIGuardWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIGUARDWINDOW_H__0FFE1FE7_59B6_47D8_B79C_7E3DD9912E14__INCLUDED_)
#define AFX_NEWUIGUARDWINDOW_H__0FFE1FE7_59B6_47D8_B79C_7E3DD9912E14__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIButton.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIGuildInfoWindow.h"

namespace SEASON3B
{
    class CNewUIGuardWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GUARDWINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_GUARDWINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_GUARDWINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GUARDWINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GUARDWINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GUARDWINDOW_EXIT_BTN = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_GUARDWINDOW_TAB_BTN = CNewUIGuildInfoWindow::IMAGE_GUILDINFO_TAB_BUTTON,
            IMAGE_GUARDWINDOW_BUTTON = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,

            IMAGE_GUARDWINDOW_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,
            IMAGE_GUARDWINDOW_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,
            IMAGE_GUARDWINDOW_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,
            IMAGE_GUARDWINDOW_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,

            IMAGE_GUARDWINDOW_SCROLL_TOP = CNewUIChatLogWindow::IMAGE_SCROLL_TOP,
            IMAGE_GUARDWINDOW_SCROLL_MIDDLE = CNewUIChatLogWindow::IMAGE_SCROLL_MIDDLE,
            IMAGE_GUARDWINDOW_SCROLL_BOTTOM = CNewUIChatLogWindow::IMAGE_SCROLL_BOTTOM,
            IMAGE_GUARDWINDOW_SCROLLBAR_ON = CNewUIChatLogWindow::IMAGE_SCROLLBAR_ON,
            IMAGE_GUARDWINDOW_SCROLLBAR_OFF = CNewUIChatLogWindow::IMAGE_SCROLLBAR_OFF,
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

        CNewUIManager* m_pNewUIMng;
        POINT m_Pos;

        CNewUIRadioGroupButton	m_TabBtn;
        int						m_iNumCurOpenTab;		// 현재 열려있는 탭버튼번호

        CNewUIButton m_BtnExit;

        CNewUIButton m_BtnProclaim;			// 공성 선포 버튼
        CNewUIButton m_BtnRegister;			// 표식 등록 버튼
        CNewUIButton m_BtnGiveUp;			// 공성 포기 버튼

        // 선포한 길드 리스트
        CUIBCDeclareGuildListBox	m_DeclareGuildListBox;
        // 확정된 길드 리스트
        CUIBCGuildListBox			m_GuildListBox;

        // UI 출력 값
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
        CNewUIGuardWindow();
        virtual ~CNewUIGuardWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
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

        void SetData(LPPMSG_ANS_CASTLESIEGESTATE Info);	// 서버에서 받아 화면 표시 설정

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

        void InitButton(CNewUIButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption);

        void UpdateSeigeInfoTab();
        void UpdateRegisterTab();
        void UpdateRegisterInfoTab();

        void RenderSeigeInfoTab();
        void RenderRegisterTab();
        void RenderRegisterInfoTab();
    };
}

#endif // !defined(AFX_NEWUIGUARDWINDOW_H__0FFE1FE7_59B6_47D8_B79C_7E3DD9912E14__INCLUDED_)

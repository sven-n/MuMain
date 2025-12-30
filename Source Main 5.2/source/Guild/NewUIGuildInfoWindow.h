// NewUIGuildInfoWindow.h: interface for the CNewUIGuildInfoWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_)
#define AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIChatLogWindow.h"
#include "NewUIGuildMakeWindow.h"
#include "NewUIButton.h"

namespace SEASON3B
{
    struct ServerMessageInfo
    {
        BYTE s_byRelationShipType;
        BYTE s_byRelationShipRequestType;
        BYTE s_byTargetUserIndexH;
        BYTE s_byTargetUserIndexL;

        ServerMessageInfo() : s_byRelationShipType(0), s_byRelationShipRequestType(0),
            s_byTargetUserIndexH(0), s_byTargetUserIndexL(0) {}
    };

    class CNewUIGuildInfoWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GUILDINFO_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference

            IMAGE_GUILDINFO_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_GUILDINFO_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GUILDINFO_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GUILDINFO_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GUILDINFO_EXIT_BTN = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,

            IMAGE_GUILDINFO_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,
            IMAGE_GUILDINFO_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,
            IMAGE_GUILDINFO_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,
            IMAGE_GUILDINFO_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,

            IMAGE_GUILDINFO_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,	//. newui_item_table01(L).tga (14,14)
            IMAGE_GUILDINFO_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,	//. newui_item_table01(R).tga (14,14)
            IMAGE_GUILDINFO_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,	//. newui_item_table02(L).tga (14,14)
            IMAGE_GUILDINFO_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)

            IMAGE_GUILDINFO_SCROLL_TOP = CNewUIChatLogWindow::IMAGE_SCROLL_TOP,
            IMAGE_GUILDINFO_SCROLL_MIDDLE = CNewUIChatLogWindow::IMAGE_SCROLL_MIDDLE,
            IMAGE_GUILDINFO_SCROLL_BOTTOM = CNewUIChatLogWindow::IMAGE_SCROLL_BOTTOM,
            IMAGE_GUILDINFO_SCROLLBAR_ON = CNewUIChatLogWindow::IMAGE_SCROLLBAR_ON,
            IMAGE_GUILDINFO_SCROLLBAR_OFF = CNewUIChatLogWindow::IMAGE_SCROLLBAR_OFF,
            IMAGE_GUILDINFO_DRAG_BTN = CNewUIChatLogWindow::IMAGE_DRAG_BTN,

            IMAGE_GUILDINFO_BUTTON = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,

            IMAGE_GUILDINFO_TAB_BUTTON = BITMAP_GUILDINFO_BEGIN,
            IMAGE_GUILDINFO_TAB_LIST,
            IMAGE_GUILDINFO_TAB_POINT,

            IMAGE_GUILDINFO_TAB_HEAD,
        };

    private:
        enum
        {
            GUILDINFO_WIDTH = 190,
            GUILDINFO_HEIGHT = 429,
        };
        enum EVENT_STATE
        {
            EVENT_NONE = 0,
            EVENT_SCROLL_BTN_DOWN,
        };
        enum BUTTON_EVENT
        {
            BUTTON_GUILD_OUT = 0,
            BUTTON_GET_POSITION,
            BUTTON_FREE_POSITION,
            BUTTON_GET_OUT,
            BUTTON_UNION_CREATE,
            BUTTON_UNION_OUT,
            BUTTON_END,
        };
        EVENT_STATE				m_EventState;

        CNewUIManager* m_pNewUIMng;
        POINT					m_Pos;
        int						m_nCurrentTab;
        int						m_Loc;
        int						m_Loc_Bk;
        int						m_BackUp;
        int						m_CurrentListPos;
        int						m_Tot_Notice;
        DWORD					m_dwPopupID;

        CNewUIButton* m_Button;
        CNewUIButton			m_BtnExit;

        CUIGuildNoticeListBox		m_GuildNotice;
        CUINewGuildMemberListBox	m_GuildMember;
        CUIUnionGuildListBox		m_UnionListBox;
        ServerMessageInfo		    m_MessageInfo;

        bool m_bRequestUnionList;

        wchar_t m_RivalGuildName[MAX_GUILDNAME + 1];

    public:
        CNewUIGuildInfoWindow();
        virtual ~CNewUIGuildInfoWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 4.5f

        CNewUIGuildInfoWindow* GetGuildInfo() const;

        void OpenningProcess();
        void ClosingProcess();

        void AddGuildNotice(wchar_t* szText);
        void SetRivalGuildName(wchar_t* szName);
        void AddGuildMember(GUILD_LIST_t* pInfo);
        void GuildClear();
        void NoticeClear();
        void UnionGuildClear();
        void AddUnionList(BYTE* pGuildMark, wchar_t* szGuildName, int nMemberCount);

        int GetUnionCount();

    public:
        const ServerMessageInfo& GetServerMessage();

    public:
        void ReceiveGuildRelationShip(BYTE byRelationShipType, BYTE byRequestType,
            BYTE  byTargetUserIndexH, BYTE byTargetUserIndexL);

    private:
        void LoadImages();
        void UnloadImages();

        bool Check_Mouse(int mx, int my);
        bool Check_Btn();

        int GetGuildMemberIndex(wchar_t* szName);

        void RenderFrame();
        void RenderNoneGuild();
        void RenderTabButton();
        void Render_Guild_Enum();
        void Render_Guild_Info();
        void RenderScrollBar();
        void Render_Guild_History();
        void Render_Text();
    };

    inline
        const ServerMessageInfo& CNewUIGuildInfoWindow::GetServerMessage()
    {
        return m_MessageInfo;
    }
}

#endif // !defined(AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_)

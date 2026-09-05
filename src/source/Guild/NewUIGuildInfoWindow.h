// NewUIGuildInfoWindow.h: interface for the CGuildInfoWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_)
#define AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Inventory/NewUIInventoryCtrl.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/HUD/NewUIChatLogWindow.h"
#include "NewUIGuildMakeWindow.h"
#include "UI/Widgets/NewUIButton.h"
#include "GuildConstants.h"

namespace mu::ui::window
{
    struct ServerMessageInfo
    {
        GuildRelationshipType s_byRelationShipType;
        GuildRequestType s_byRelationShipRequestType;
        BYTE s_byTargetUserIndexH;
        BYTE s_byTargetUserIndexL;

        ServerMessageInfo() : s_byRelationShipType(GuildRelationshipType::Undefined), s_byRelationShipRequestType(GuildRequestType::Undefined),
            s_byTargetUserIndexH(0), s_byTargetUserIndexL(0) {}
    };

    class CGuildInfoWindow : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GUILDINFO_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference

            IMAGE_GUILDINFO_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_GUILDINFO_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GUILDINFO_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GUILDINFO_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GUILDINFO_EXIT_BTN = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,

            IMAGE_GUILDINFO_TOP_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,
            IMAGE_GUILDINFO_BOTTOM_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,
            IMAGE_GUILDINFO_LEFT_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,
            IMAGE_GUILDINFO_RIGHT_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,

            IMAGE_GUILDINFO_TOP_LEFT = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,	//. newui_item_table01(L).tga (14,14)
            IMAGE_GUILDINFO_TOP_RIGHT = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,	//. newui_item_table01(R).tga (14,14)
            IMAGE_GUILDINFO_BOTTOM_LEFT = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,	//. newui_item_table02(L).tga (14,14)
            IMAGE_GUILDINFO_BOTTOM_RIGHT = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)

            IMAGE_GUILDINFO_SCROLL_TOP = CChatLogWindow::IMAGE_SCROLL_TOP,
            IMAGE_GUILDINFO_SCROLL_MIDDLE = CChatLogWindow::IMAGE_SCROLL_MIDDLE,
            IMAGE_GUILDINFO_SCROLL_BOTTOM = CChatLogWindow::IMAGE_SCROLL_BOTTOM,
            IMAGE_GUILDINFO_SCROLLBAR_ON = CChatLogWindow::IMAGE_SCROLLBAR_ON,
            IMAGE_GUILDINFO_SCROLLBAR_OFF = CChatLogWindow::IMAGE_SCROLLBAR_OFF,
            IMAGE_GUILDINFO_DRAG_BTN = CChatLogWindow::IMAGE_DRAG_BTN,

            IMAGE_GUILDINFO_BUTTON = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,

            IMAGE_GUILDINFO_TAB_BUTTON = BITMAP_GUILDINFO_BEGIN,
            IMAGE_GUILDINFO_TAB_LIST,
            IMAGE_GUILDINFO_TAB_POINT,

            IMAGE_GUILDINFO_TAB_HEAD,
        };

    private:
        enum
        {
            GUILDINFO_WIDTH = GuildConstants::UILayout::WINDOW_WIDTH,
            GUILDINFO_HEIGHT = GuildConstants::UILayout::WINDOW_HEIGHT,
        };
        enum EVENT_STATE
        {
            EVENT_NONE = 0,
            EVENT_SCROLL_BTN_DOWN,
        };
        enum BUTTON_EVENT
        {
            BUTTON_GUILD_OUT = static_cast<int>(GuildConstants::GuildInfoButton::GUILD_OUT),
            BUTTON_GET_POSITION = static_cast<int>(GuildConstants::GuildInfoButton::GET_POSITION),
            BUTTON_FREE_POSITION = static_cast<int>(GuildConstants::GuildInfoButton::FREE_POSITION),
            BUTTON_GET_OUT = static_cast<int>(GuildConstants::GuildInfoButton::GET_OUT),
            BUTTON_UNION_CREATE = static_cast<int>(GuildConstants::GuildInfoButton::UNION_CREATE),
            BUTTON_UNION_OUT = static_cast<int>(GuildConstants::GuildInfoButton::UNION_OUT),
            BUTTON_END = static_cast<int>(GuildConstants::GuildInfoButton::END),
        };
        EVENT_STATE				m_EventState;

        CManager* m_pNewUIMng;
        POINT					m_Pos;
        int						m_nCurrentTab;
        int						m_Loc;
        int						m_Loc_Bk;
        int						m_BackUp;
        int						m_CurrentListPos;
        int						m_Tot_Notice;
        DWORD					m_dwPopupID;

        CButton* m_Button;
        CButton			m_BtnExit;

        CUIGuildNoticeListBox		m_GuildNotice;
        CUINewGuildMemberListBox	m_GuildMember;
        CUIUnionGuildListBox		m_UnionListBox;
        ServerMessageInfo		    m_MessageInfo;

        bool m_bRequestUnionList;

        wchar_t m_RivalGuildName[MAX_GUILDNAME + 1];

    public:
        CGuildInfoWindow();
        virtual ~CGuildInfoWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 4.5f

        CGuildInfoWindow* GetGuildInfo() const;

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
        void ReceiveGuildRelationShip(GuildRelationshipType byRelationShipType, GuildRequestType byRequestType,
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
        const ServerMessageInfo& CGuildInfoWindow::GetServerMessage()
    {
        return m_MessageInfo;
    }
}

#endif // !defined(AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_)

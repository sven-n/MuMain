
#if !defined(AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_)
#define AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Widgets/UIControls.h"
#include "Guild/GuildInfoRmlModel.h"
#include "GuildMakeWindow.h"
#include "GuildConstants.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml
{
class ElementDocument;
}

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

    // The guild window, docked right: the no-guild hint, or the Guild / Members / Alliance tabs.
    // guild_info.rml draws it; the notice, member and alliance lists stay native controls for
    // their data, scrolling and line clicks (their lines are drawn by the document), and the tab
    // and scroll thumb hit tests stay native too. C++ keeps every request.
    class CGuildInfoWindow : public CObject
    {
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
            BUTTON_EXIT = BUTTON_END,
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

        RmlModelBinder<GuildInfoRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
        int m_PendingButton = -1; // a BUTTON_EVENT, or BUTTON_EXIT

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

        void ReloadRmlTheme();

    private:
        bool Check_Mouse(int mx, int my);
        bool Check_Btn(int button);
        void UpdateScrollThumb();

        int GetGuildMemberIndex(wchar_t* szName);

        void BuildRmlUi();
        void SyncRmlModel();
        void SyncContent();
    };

    inline
        const ServerMessageInfo& CGuildInfoWindow::GetServerMessage()
    {
        return m_MessageInfo;
    }
}

#endif // !defined(AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_)

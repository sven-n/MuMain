
#if !defined(AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_)
#define AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Dialogs/MessageBox.h"
#include "Guild/GuildInfoRmlModel.h"
#include "GuildMakeWindow.h"
#include "GuildConstants.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include <string>
#include <vector>

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

    // RmlUi owns the guild window presentation and list input; C++ validates requests.
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
        int						m_Tot_Notice;

        void BindRmlModel(Rml::DataModelConstructor& c, GuildInfoRmlModel& model);
        void OnRmlReloaded();
        UI::RmlBridge::ThemedView<GuildInfoRmlModel> m_RmlView{"guild_info",
            [this](Rml::DataModelConstructor& c, GuildInfoRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/guild_info.rml"}}, {.afterReload = [this] { OnRmlReloaded(); }}};
        int m_PendingButton = -1; // a BUTTON_EVENT, or BUTTON_EXIT

        // Announcement lines in reading order; RmlUi owns scrolling.
        std::vector<std::wstring>   m_NoticeLines;
        // One guild member as the window holds it. Selection is kept by name, not by row index,
        // so it survives the list being rebuilt when the server resends it.
        struct MemberEntry
        {
            std::wstring name;
            BYTE number = 0;
            BYTE server = 255;
            BYTE guildStatus = 0;
        };
        std::vector<MemberEntry>    m_Members;
        std::wstring                m_SelectedMember;
        // One allied guild as the window holds it. Selection is kept by name for the same
        // reason the member list's is: the server resends the whole alliance list.
        struct UnionEntry
        {
            std::wstring name;
            int memberCount = 0;
            BYTE mark[64] = {};
        };
        std::vector<UnionEntry>     m_Unions;
        std::wstring                m_SelectedUnion;
        bool m_ListsDirty = true;
        float m_ListScale = -1.f;
        float m_ListTextPx = -1.f;
        ServerMessageInfo		    m_MessageInfo;

        bool m_bRequestUnionList;

        wchar_t m_RivalGuildName[MAX_GUILDNAME + 1];

    public:
        CGuildInfoWindow();
        virtual ~CGuildInfoWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        Rml::ElementDocument* GetFillDocument() const override { return m_RmlView.Document(); }
        void Release();

        void SetPos(int x, int y);

        // The highlighted member, or nullptr when the selection no longer names a live row.
        const MemberEntry* SelectedMember() const;
        const UnionEntry* SelectedUnion() const;

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
        // Select a member from the displayed rows.
        void SelectMember(int displayIndex);
        // Select an alliance from the displayed rows.
        void SelectUnion(int displayIndex);
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
        bool Check_Mouse(int mx, int my);
        bool Check_Btn(int button);

        int GetGuildMemberIndex(const wchar_t* szName);

        void BuildRmlUi();
        void SyncRmlModel();
        void SyncContent();
        void SyncListContent();
        std::vector<GuildMemberRow> BuildMemberRows() const;
        std::vector<GuildUnionRow> BuildUnionRows() const;
    };

    inline
        const ServerMessageInfo& CGuildInfoWindow::GetServerMessage()
    {
        return m_MessageInfo;
    }
}

#endif // !defined(AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_)

//////////////////////////////////////////////////////////////////////
// ServerListManager.h: interface for the CServerListManager class.
//////////////////////////////////////////////////////////////////////
#pragma once
#include "ServerGroup.h"

#include "Data/GameData/ServerListData/ButtonColors.h"
#include "Data/GameData/ServerListData/ChannelNameBuilder.h"
#include "Data/GameData/ServerListData/ServerListData.h"

#include <array>
#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

typedef std::map<int, CServerGroup*> type_mapServerGroup;

class CServerListManager
{
protected:
    CServerListManager();

public:
    virtual ~CServerListManager();

public:
    static CServerListManager* GetInstance();

    void InsertServerGroup(int iConnectIndex, int iServerPercent);
    void Release();
    // Reads Data/Local/ServerList.json. False when it has fatal problems; they are in
    // errorMessage then, for the player.
    bool LoadServerListScript(std::string& errorMessage);
    // The list the login screen names the servers with: the groups in their
    // order, the VIP types and the texts. The selected channel follows it
    // (SetSelectServerInfo).
    void SetServerList(const Data::ServerList::ServerListData& list);
    void SetFirst();
    bool GetNext(OUT CServerGroup*& pServerGroup);
    CServerGroup* GetServerGroupByBtnPos(int iBtnPos);
    // The server the connect server knows by this ID (CServerInfo::m_iConnectIndex), and the
    // group it is in. nullptr (and no group) when the list doesn't have it.
    CServerInfo* FindServer(int iConnectIndex, CServerGroup*& pServerGroup);

    int GetServerGroupSize();

    // The channel the player joined: channel `iIndex` of the group `groupId` (its name `pszName`), with its PvP and
    // VIP type. While the list has the group, its name and the channel's settings follow the list
    // (SetServerList: MuEditor changes them while the game runs).
    void SetSelectServerInfo(int groupId, const wchar_t* pszName, int iIndex,
                             const Data::ServerList::ChannelSettings& channel);
    wchar_t* GetSelectServerName();
    int GetSelectServerIndex();
    // Whether the selected channel has PvP off.
    bool IsNonPvP();
    // The selected channel as `place` names it, in the game's language:
    // "[Valhalla-1 (Gold Non-PvP)]". `character` fills {character} (the
    // screenshot message).
    std::wstring GetSelectServerTitle(Data::ServerList::ChannelNamePlace place, const wchar_t* character = L"");
    // The same at `place`, in UTF-8, for the windows that show it every frame (login, character
    // info): built again only when the list, the selected channel or the game's language changed.
    const std::string& GetSelectServerTitleUtf8(Data::ServerList::ChannelNamePlace place);
    // The load text the server's button shows on its load bar, in the game's language: the list's text of
    // a full or overloaded server ("(Full)"); none while it can be joined or the button template has no
    // {load} (Data::ServerList::GetButtonLoadText).
    std::string GetServerLoadText(const CServerInfo& server) const;
    // A channel's button text without its load text, for the channel `index` (1 to MAX_SERVER_PER_GROUP) of
    // `group`, whether or not the connect server sent it: "Valhalla-4 (Gold Non-PvP)". The login screen shows the
    // load text apart from it, at the button's right.
    std::string GetServerButtonName(const CServerGroup& group, int index) const;
    // A load text a button can show, in the game's language, and its look.
    struct ButtonLoadText
    {
        std::string text;
        bool bold = false;
        bool italic = false;
    };
    // The list's load texts of a full and of an overloaded server, those not empty; none when the button
    // template has no {load}.
    std::vector<ButtonLoadText> GetButtonLoadTexts() const;
    // The groups, VIP types, colors and texts the channels are named with.
    const Data::ServerList::ServerListData& GetServerList() const
    {
        return m_serverList;
    }
    // Its color there in the theme folder `theme`, by the list's button color rules.
    Data::ServerList::ButtonColor GetServerButtonColor(const CServerInfo& server, std::string_view theme) const;
    void SetTotalServer(int iTotalServer);
    int GetTotalServer();

#ifdef _EDITOR
    // A server of the last list the connect server sent.
    struct ReceivedServer
    {
        int connectIndex = 0;
        int percent = 0;
    };

    const std::vector<ReceivedServer>& GetReceivedServers() const
    {
        return m_receivedServers;
    }
    // Changes each time the received servers change, so a view of them is
    // built again only then.
    int GetReceivedServersVersion() const
    {
        return m_receivedServersVersion;
    }
    // Builds the groups of the last list the connect server sent again, with
    // the groups SetServerList set since.
    void RebuildServerGroups();
    // The highest load the preview of the offline servers takes: an overloaded server's.
    static constexpr int MaxOfflineServerPercent = 140;
    // MuEditor's preview of the servers that are offline: every channel 1 to 20 of every group of the list
    // that the connect server didn't send is shown too, at `percent` load, and can't be joined (m_offline). It
    // takes effect with the next RebuildServerGroups.
    void SetOfflinePreview(bool show, int percent);
    bool IsShowingOfflineServers() const
    {
        return m_showOffline;
    }
    int GetOfflineServerPercent() const
    {
        return m_offlinePercent;
    }
#endif

protected:
    // The selected channel's group name and settings as the list has them now; kept when the list has no such group.
    void FollowSelectedChannel();
    // The list's group with the ID, and its place in the list from 1 (`sequence`); nullptr when the list has none.
    const Data::ServerList::ServerGroup* FindListGroup(int groupId, int& sequence) const;
    bool MakeServerGroup(IN int iServerGroupIndex, OUT CServerGroup* pServerGroup);
    // The group of the server with this connect index, made when the list has it; nullptr when it doesn't.
    CServerGroup* FindOrMakeGroup(int iConnectIndex);
    void InsertServer(CServerGroup* pServerGroup, int iConnectIndex, int iServerPercent);

public:
    type_mapServerGroup m_mapServerGroup;
    type_mapServerGroup::iterator m_iterServerGroup;

    int m_iTotalServer;
    // The selected channel's group (its ID, -1 for none, and name), its number and settings.
    int m_iSelectServerGroupId = -1;
    wchar_t m_szSelectServerName[MAX_TEXT_LENGTH];
    int m_iSelectServerIndex;
    Data::ServerList::ChannelSettings m_selectChannel;

protected:
    // The groups, VIP types, colors and texts the channels are named with.
    Data::ServerList::ServerListData m_serverList;
    // The index of each group of m_serverList by its ID.
    std::map<int, size_t> m_groupIndexById;
    // A title of GetSelectServerTitleUtf8, as it was built.
    struct CachedTitle
    {
        int version = -1;
        std::string locale;
        std::string text;
    };
    std::array<CachedTitle, Data::ServerList::ChannelNamePlaceCount> m_titles;
    // Changes with the list and the selected channel: a cached title older than it is built again.
    int m_titleVersion = 0;
#ifdef _EDITOR
    std::vector<ReceivedServer> m_receivedServers;
    int m_receivedServersVersion = 0;
    bool m_showOffline = false;
    int m_offlinePercent = 50;
#endif
};

#define g_ServerListManager CServerListManager::GetInstance()

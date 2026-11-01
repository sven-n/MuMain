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
    // order, the VIP types and the texts.
    void SetServerList(const Data::ServerList::ServerListData& list);
    void SetFirst();
    bool GetNext(OUT CServerGroup*& pServerGroup);
    CServerGroup* GetServerGroupByBtnPos(int iBtnPos);

    int GetServerGroupSize();

    void SetSelectServerInfo(const wchar_t* pszName, int iIndex, const Data::ServerList::ChannelSettings& channel);
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
    // A channel's button on the login screen, split at its {load} text: "Valhalla-1 (Gold Non-PvP)",
    // with the list's text for a full or overloaded server ("(Full)") after it.
    Data::ServerList::ButtonTextParts GetServerButtonParts(const CServerGroup& group, const CServerInfo& server) const;
    // The same in one text.
    std::wstring GetServerButtonText(const CServerGroup& group, const CServerInfo& server) const;
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

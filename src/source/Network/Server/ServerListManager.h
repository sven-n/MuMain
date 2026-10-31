//////////////////////////////////////////////////////////////////////
// ServerListManager.h: interface for the CServerListManager class.
//////////////////////////////////////////////////////////////////////
#pragma once
#include "ServerGroup.h"

#include "Data/GameData/ServerListData/ButtonColors.h"
#include "Data/GameData/ServerListData/ServerListData.h"

#include <string>
#include <string_view>
#include <vector>

// A group of Data/Local/ServerList.json, with its place in the list.
struct SServerGroupInfo
{
    Data::ServerList::ServerGroup group;
    int sequence = 0;
};

typedef std::map<int, SServerGroupInfo> ServerListScriptMap;
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
    // A channel's button on the login screen, with its load: "Valhalla-1 (Gold Non-PvP) Connect".
    std::wstring GetServerButtonText(const CServerGroup& group, const CServerInfo& server) const;
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
#endif

protected:
    const SServerGroupInfo* GetServerGroupInfoInScript(int iServerGroupIndex);
    bool MakeServerGroup(IN int iServerGroupIndex, OUT CServerGroup* pServerGroup);
    void InsertServer(CServerGroup* pServerGroup, int iConnectIndex, int iServerPercent);

public:
    type_mapServerGroup m_mapServerGroup;
    type_mapServerGroup::iterator m_iterServerGroup;

    int m_iTotalServer;
    wchar_t m_szSelectServerName[MAX_TEXT_LENGTH];
    int m_iSelectServerIndex;
    Data::ServerList::ChannelSettings m_selectChannel;

protected:
    // The VIP types and texts the channels are named with; its groups by ID in
    // m_mapServerListScript.
    Data::ServerList::ServerListData m_serverList;
    ServerListScriptMap m_mapServerListScript;
#ifdef _EDITOR
    std::vector<ReceivedServer> m_receivedServers;
    int m_receivedServersVersion = 0;
#endif
};

#define g_ServerListManager CServerListManager::GetInstance()

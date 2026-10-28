//////////////////////////////////////////////////////////////////////
// ServerListManager.h: interface for the CServerListManager class.
//////////////////////////////////////////////////////////////////////
#pragma once
#include "ServerGroup.h"

#include "Data/GameData/ServerListData/ServerListGroup.h"

#include <span>
#include <string>
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
    // The groups the login screen names the servers with, in their order.
    void SetServerGroups(std::span<const Data::ServerList::ServerGroup> groups);
    void SetFirst();
    bool GetNext(OUT CServerGroup*& pServerGroup);
    CServerGroup* GetServerGroupByBtnPos(int iBtnPos);

    int GetServerGroupSize();

    void SetSelectServerInfo(wchar_t* pszName, int iIndex, BYTE byNonPvP);
    wchar_t* GetSelectServerName();
    int GetSelectServerIndex();
    BYTE GetNonPVPInfo();
    bool IsNonPvP();
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
    // the groups SetServerGroups set since.
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
    BYTE m_byNonPvP;

protected:
    ServerListScriptMap m_mapServerListScript;
#ifdef _EDITOR
    std::vector<ReceivedServer> m_receivedServers;
    int m_receivedServersVersion = 0;
#endif
};

#define g_ServerListManager CServerListManager::GetInstance()

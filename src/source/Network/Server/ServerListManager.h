//////////////////////////////////////////////////////////////////////
// ServerListManager.h: interface for the CServerListManager class.
//////////////////////////////////////////////////////////////////////
#pragma once
#include "ServerGroup.h"

#include "Data/GameData/ServerListData/ServerGroup.h"

#include <span>
#include <string>

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
    // Reads Data/Local/ServerList.json. False when it has errors; they are in
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
};

#define g_ServerListManager CServerListManager::GetInstance()

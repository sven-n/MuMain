//////////////////////////////////////////////////////////////////////
// ServerListManager.cpp: implementation of the CServerListManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerListManager.h"
#include "I18N/All.h"

#include "Core/Text/Utf8.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/GameData/ServerListData/ServerListJson.h"

#include <filesystem>
#include <memory>
#include <string_view>

namespace
{
constexpr const char* ServerListName = "server list";
// The bit of a channel type that turns PvP off.
constexpr BYTE NonPvpBit = 0x01;
} // namespace

CServerListManager::CServerListManager()
{
    m_iTotalServer = 0;
    m_szSelectServerName[0] = '\0';
    m_iSelectServerIndex = -1;
}

CServerListManager::~CServerListManager()
{
    Release();
}

CServerListManager* CServerListManager::GetInstance()
{
    static CServerListManager s_ServerListManager;
    return &s_ServerListManager;
}

void CServerListManager::Release()
{
#ifdef _EDITOR
    m_receivedServers.clear();
    ++m_receivedServersVersion;
#endif
    auto iterServerGroup = m_mapServerGroup.begin();
    for (; iterServerGroup != m_mapServerGroup.end(); iterServerGroup++)
    {
        delete iterServerGroup->second;
    }

    m_mapServerGroup.clear();
    m_iTotalServer = 0;
}

bool CServerListManager::LoadServerListScript(std::string& errorMessage)
{
    const std::filesystem::path file = Data::ServerList::ServerListFile;
    Data::ServerList::ServerListLoadResult result = Data::ServerList::LoadServerListFile(file);
    Data::LogDataIssues("Server list:", result.issues);
    if (Data::Items::HasFatal(result.issues))
    {
        errorMessage = Data::DescribeFatalIssues(ServerListName, file, result.issues);
        return false;
    }
    SetServerGroups(result.groups);
    return true;
}

void CServerListManager::SetServerGroups(std::span<const Data::ServerList::ServerGroup> groups)
{
    m_mapServerListScript.clear();
    int sequence = 0;
    for (const Data::ServerList::ServerGroup& group : groups)
    {
        m_mapServerListScript[group.id] = {group, ++sequence};
    }
}

#ifdef _EDITOR
void CServerListManager::RebuildServerGroups()
{
    const std::vector<ReceivedServer> servers = m_receivedServers;
    const int totalServer = m_iTotalServer;
    Release();
    SetTotalServer(totalServer);
    for (const ReceivedServer& server : servers)
    {
        InsertServerGroup(server.connectIndex, server.percent);
    }
}
#endif

const SServerGroupInfo* CServerListManager::GetServerGroupInfoInScript(int iServerGroupIndex)
{
    ServerListScriptMap::const_iterator iter = m_mapServerListScript.find(iServerGroupIndex);
    if (iter == m_mapServerListScript.end())
        return NULL;

    return &(iter->second);
}

void CServerListManager::InsertServerGroup(int iConnectIndex, int iServerPercent)
{
#ifdef _EDITOR
    m_receivedServers.push_back({iConnectIndex, iServerPercent});
    ++m_receivedServersVersion;
#endif
    CServerGroup* pServerGroup = NULL;

    auto iterServerGroup = m_mapServerGroup.begin();

    bool bEqual = false;
    while (iterServerGroup != m_mapServerGroup.end())
    {
        if ((iterServerGroup->second)->m_iServerIndex == iConnectIndex / MAX_SERVER_PER_GROUP)
        {
            bEqual = true;
            break;
        }

        iterServerGroup++;
    }

    if (bEqual == true)
    {
        pServerGroup = iterServerGroup->second;
    }
    else
    {
        // A server of a group the list does not have is not shown.
        auto pNewServerGroup = std::make_unique<CServerGroup>();
        if (MakeServerGroup(iConnectIndex / MAX_SERVER_PER_GROUP, pNewServerGroup.get()) == false)
            return;

        pServerGroup = pNewServerGroup.release();
        m_mapServerGroup.insert(type_mapServerGroup::value_type(pServerGroup->m_iSequence, pServerGroup));
    }

    InsertServer(pServerGroup, iConnectIndex, iServerPercent);

    m_iterServerGroup = m_mapServerGroup.begin();
}

bool CServerListManager::MakeServerGroup(IN int iServerGroupIndex, OUT CServerGroup* pServerGroup)
{
    const SServerGroupInfo* pServerGroupInfo = GetServerGroupInfoInScript(iServerGroupIndex);
    if (NULL == pServerGroupInfo)
        return false;

    const Data::ServerList::ServerGroup& group = pServerGroupInfo->group;
    const std::wstring name = Core::Text::FromUtf8(group.name);
    const std::wstring description = Core::Text::FromUtf8(group.description);
    wcsncpy_s(pServerGroup->m_szName, name.c_str(), _TRUNCATE);
    wcsncpy_s(pServerGroup->m_szDescription, description.c_str(), _TRUNCATE);
    pServerGroup->m_iSequence = pServerGroupInfo->sequence;
    pServerGroup->m_iWidthPos = group.testServer ? CServerGroup::SBP_CENTER : CServerGroup::SBP_LEFT;
    pServerGroup->m_iServerIndex = iServerGroupIndex;
    pServerGroup->m_bPvPServer = true;
    for (int i = 0; i < MAX_SERVER_PER_GROUP; ++i)
    {
        pServerGroup->m_abyNonPvpServer[i] = static_cast<BYTE>(group.channels[i]);
        if (NonPvpBit & pServerGroup->m_abyNonPvpServer[i])
            pServerGroup->m_bPvPServer = false;
    }

    return true;
}

void CServerListManager::InsertServer(CServerGroup* pServerGroup, int iConnectIndex, int iServerPercent)
{
    auto* pServerInfo = new CServerInfo;
    pServerInfo->m_iSequence = pServerGroup->GetServerSize();
    pServerInfo->m_iIndex = (iConnectIndex % MAX_SERVER_PER_GROUP) + 1;
    pServerInfo->m_iConnectIndex = iConnectIndex;
    pServerInfo->m_iPercent = iServerPercent;
    pServerInfo->m_byNonPvP = pServerGroup->m_abyNonPvpServer[pServerInfo->m_iIndex - 1];

    const wchar_t* loadText;
    if (iServerPercent >= 128)
    {
        loadText = I18N::Game::NowPreparing;
    }
    else if (iServerPercent >= 100)
    {
        loadText = I18N::Game::Full;
    }
    else
    {
        loadText = I18N::Game::Connect;
    }

    // The suffix is ASCII, so each char is its wchar_t.
    const std::string_view suffix =
        Data::ServerList::GetChannelSuffix(static_cast<Data::ServerList::ChannelType>(pServerInfo->m_byNonPvP));
    const std::wstring wideSuffix(suffix.begin(), suffix.end());
    mu_swprintf(pServerInfo->m_bName, L"%ls-%d%ls %ls", pServerGroup->m_szName, pServerInfo->m_iIndex,
                wideSuffix.c_str(), loadText);

    pServerGroup->InsertServerInfo(pServerInfo);
}

int CServerListManager::GetServerGroupSize()
{
    return m_mapServerGroup.size();
}

void CServerListManager::SetFirst()
{
    m_iterServerGroup = m_mapServerGroup.begin();
}

bool CServerListManager::GetNext(OUT CServerGroup*& pServerGroup)
{
    if (m_iterServerGroup == m_mapServerGroup.end())
    {
        pServerGroup = NULL;

        return false;
    }

    pServerGroup = m_iterServerGroup->second;

    m_iterServerGroup++;

    return true;
}

CServerGroup* CServerListManager::GetServerGroupByBtnPos(int iBtnPos)
{
    auto iterServerGroup = m_mapServerGroup.begin();

    while (iterServerGroup != m_mapServerGroup.end())
    {
        if (iterServerGroup->second->m_iBtnPos == iBtnPos)
            return iterServerGroup->second;

        iterServerGroup++;
    }

    return NULL;
}

void CServerListManager::SetSelectServerInfo(wchar_t* pszName, int iIndex, BYTE byNonPvP)
{
    wcscpy(m_szSelectServerName, pszName);
    m_iSelectServerIndex = iIndex;
    m_byNonPvP = byNonPvP;
}

wchar_t* CServerListManager::GetSelectServerName()
{
    return m_szSelectServerName;
}

int CServerListManager::GetSelectServerIndex()
{
    return m_iSelectServerIndex;
}

BYTE CServerListManager::GetNonPVPInfo()
{
    return m_byNonPvP;
}

bool CServerListManager::IsNonPvP()
{
    return bool(NonPvpBit & GetNonPVPInfo());
}

void CServerListManager::SetTotalServer(int iTotalServer)
{
    m_iTotalServer = iTotalServer;
}

int CServerListManager::GetTotalServer()
{
    return m_iTotalServer;
}
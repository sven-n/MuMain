//////////////////////////////////////////////////////////////////////
// ServerListManager.cpp: implementation of the CServerListManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerListManager.h"
#include "I18N/All.h"

#include "Core/Text/Utf8.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/GameData/ServerListData/ChannelNameBuilder.h"
#include "Data/GameData/ServerListData/ServerListJson.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string_view>

namespace
{
constexpr const char* ServerListName = "server list";

// What a channel's button says about the server's load.
const wchar_t* GetLoadText(int percent)
{
    if (percent >= 128)
    {
        return I18N::Game::NowPreparing;
    }
    if (percent >= 100)
    {
        return I18N::Game::Full;
    }
    return I18N::Game::Connect;
}

std::string_view GetGameLocale()
{
    const char* locale = I18N::GetCurrentLocale();
    return locale != nullptr ? std::string_view(locale) : Data::LocalizedString::NeutralLocale;
}
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
    Data::ServerList::ServerListLoadResult result =
        Data::ServerList::LoadServerListFile(file, I18N::GetAvailableLocales());
    Data::LogDataIssues("Server list:", result.issues);
    if (Data::Items::HasFatal(result.issues))
    {
        errorMessage = Data::DescribeFatalIssues(ServerListName, file, result.issues);
        return false;
    }
    SetServerList(result.list);
    return true;
}

void CServerListManager::SetServerList(const Data::ServerList::ServerListData& list)
{
    m_serverList = list;
    m_mapServerListScript.clear();
    int sequence = 0;
    for (const Data::ServerList::ServerGroup& group : list.groups)
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
    pServerGroup->m_channels = group.channels;
    pServerGroup->m_bPvPServer =
        std::all_of(group.channels.begin(), group.channels.end(),
                    [](const Data::ServerList::ChannelSettings& channel) { return channel.pvp; });

    return true;
}

void CServerListManager::InsertServer(CServerGroup* pServerGroup, int iConnectIndex, int iServerPercent)
{
    auto* pServerInfo = new CServerInfo;
    pServerInfo->m_iSequence = pServerGroup->GetServerSize();
    pServerInfo->m_iIndex = (iConnectIndex % MAX_SERVER_PER_GROUP) + 1;
    pServerInfo->m_iConnectIndex = iConnectIndex;
    pServerInfo->m_iPercent = iServerPercent;
    pServerInfo->m_channel = pServerGroup->m_channels[pServerInfo->m_iIndex - 1];

    pServerGroup->InsertServerInfo(pServerInfo);
}

std::wstring CServerListManager::GetServerButtonText(const CServerGroup& group, const CServerInfo& server) const
{
    const std::string name = Core::Text::ToUtf8(group.m_szName);
    const std::string load = Core::Text::ToUtf8(GetLoadText(server.m_iPercent));
    Data::ServerList::ChannelNameValues values;
    values.name = name;
    values.channel = server.m_iIndex;
    values.load = load;
    return Core::Text::FromUtf8(Data::ServerList::BuildChannelName(
        m_serverList, Data::ServerList::ChannelNamePlace::Button, server.m_channel, values, GetGameLocale()));
}

Data::ServerList::ThemeColor CServerListManager::GetServerButtonColor(const CServerInfo& server) const
{
    const Data::ServerList::VipType* vipType =
        server.m_channel.vip.empty() ? nullptr : m_serverList.FindVipType(server.m_channel.vip);
    if (vipType != nullptr)
    {
        return vipType->themeColor;
    }
    return server.m_channel.pvp ? Data::ServerList::ThemeColor::Gray : Data::ServerList::ThemeColor::Default;
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

void CServerListManager::SetSelectServerInfo(const wchar_t* pszName, int iIndex,
                                             const Data::ServerList::ChannelSettings& channel)
{
    wcsncpy_s(m_szSelectServerName, pszName, _TRUNCATE);
    m_iSelectServerIndex = iIndex;
    m_selectChannel = channel;
}

wchar_t* CServerListManager::GetSelectServerName()
{
    return m_szSelectServerName;
}

int CServerListManager::GetSelectServerIndex()
{
    return m_iSelectServerIndex;
}

bool CServerListManager::IsNonPvP()
{
    return !m_selectChannel.pvp;
}

std::wstring CServerListManager::GetSelectServerTitle(Data::ServerList::ChannelNamePlace place,
                                                      const wchar_t* character)
{
    const std::string name = Core::Text::ToUtf8(GetSelectServerName());
    const std::string characterName = Core::Text::ToUtf8(character);
    Data::ServerList::ChannelNameValues values;
    values.name = name;
    values.channel = GetSelectServerIndex();
    values.character = characterName;
    return Core::Text::FromUtf8(
        Data::ServerList::BuildChannelName(m_serverList, place, m_selectChannel, values, GetGameLocale()));
}

void CServerListManager::SetTotalServer(int iTotalServer)
{
    m_iTotalServer = iTotalServer;
}

int CServerListManager::GetTotalServer()
{
    return m_iTotalServer;
}
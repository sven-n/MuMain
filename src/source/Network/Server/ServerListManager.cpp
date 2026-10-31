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
#include <map>
#include <memory>
#include <optional>
#include <string_view>

namespace
{
constexpr const char* ServerListName = "server list";

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
    ++m_titleVersion;
    m_groupIndexById.clear();
    for (size_t index = 0; index < m_serverList.groups.size(); ++index)
    {
        m_groupIndexById[m_serverList.groups[index].id] = index;
    }
    FollowSelectedChannel();
}

void CServerListManager::FollowSelectedChannel()
{
    int sequence = 0;
    const Data::ServerList::ServerGroup* group =
        m_iSelectServerGroupId >= 0 ? FindListGroup(m_iSelectServerGroupId, sequence) : nullptr;
    if (group == nullptr || m_iSelectServerIndex < 1 || m_iSelectServerIndex > MAX_SERVER_PER_GROUP)
        return;

    wcsncpy_s(m_szSelectServerName, Core::Text::FromUtf8(group->name).c_str(), _TRUNCATE);
    m_selectChannel = group->channels[static_cast<size_t>(m_iSelectServerIndex - 1)];
}

#ifdef _EDITOR
void CServerListManager::RebuildServerGroups()
{
    const std::vector<ReceivedServer> servers = m_receivedServers;
    const int totalServer = m_iTotalServer;
    Release();
    SetTotalServer(totalServer);
    if (!m_showOffline)
    {
        for (const ReceivedServer& server : servers)
        {
            InsertServerGroup(server.connectIndex, server.percent);
        }
        return;
    }
    // With the preview, each group has all its channels, in their order: the load of a sent one, none for the others.
    std::map<int, std::optional<int>> channels;
    for (const ReceivedServer& server : servers)
    {
        channels[server.connectIndex] = server.percent;
    }
    for (const Data::ServerList::ServerGroup& group : m_serverList.groups)
    {
        for (int channel = 0; channel < MAX_SERVER_PER_GROUP; ++channel)
        {
            channels.try_emplace(group.id * MAX_SERVER_PER_GROUP + channel, std::nullopt);
        }
    }
    for (const auto& [connectIndex, percent] : channels)
    {
        if (percent)
        {
            InsertServerGroup(connectIndex, *percent);
        }
        else if (CServerGroup* pServerGroup = FindOrMakeGroup(connectIndex))
        {
            InsertServer(pServerGroup, connectIndex, m_offlinePercent);
            pServerGroup->m_listServerInfo.back()->m_offline = true;
            m_iterServerGroup = m_mapServerGroup.begin();
        }
    }
}

void CServerListManager::SetOfflinePreview(bool show, int percent)
{
    m_showOffline = show;
    m_offlinePercent = std::clamp(percent, 0, MaxOfflineServerPercent);
}
#endif

const Data::ServerList::ServerGroup* CServerListManager::FindListGroup(int groupId, int& sequence) const
{
    const auto found = m_groupIndexById.find(groupId);
    if (found == m_groupIndexById.end())
    {
        return nullptr;
    }
    sequence = static_cast<int>(found->second) + 1;
    return &m_serverList.groups[found->second];
}

void CServerListManager::InsertServerGroup(int iConnectIndex, int iServerPercent)
{
#ifdef _EDITOR
    m_receivedServers.push_back({iConnectIndex, iServerPercent});
    ++m_receivedServersVersion;
#endif
    // A server of a group the list does not have is not shown.
    CServerGroup* pServerGroup = FindOrMakeGroup(iConnectIndex);
    if (pServerGroup == nullptr)
        return;

    InsertServer(pServerGroup, iConnectIndex, iServerPercent);

    m_iterServerGroup = m_mapServerGroup.begin();
}

CServerGroup* CServerListManager::FindOrMakeGroup(int iConnectIndex)
{
    for (const auto& entry : m_mapServerGroup)
    {
        if (entry.second->m_iServerIndex == iConnectIndex / MAX_SERVER_PER_GROUP)
        {
            return entry.second;
        }
    }

    auto pNewServerGroup = std::make_unique<CServerGroup>();
    if (MakeServerGroup(iConnectIndex / MAX_SERVER_PER_GROUP, pNewServerGroup.get()) == false)
        return nullptr;

    CServerGroup* pServerGroup = pNewServerGroup.release();
    m_mapServerGroup.insert(type_mapServerGroup::value_type(pServerGroup->m_iSequence, pServerGroup));
    return pServerGroup;
}

bool CServerListManager::MakeServerGroup(IN int iServerGroupIndex, OUT CServerGroup* pServerGroup)
{
    int sequence = 0;
    const Data::ServerList::ServerGroup* listGroup = FindListGroup(iServerGroupIndex, sequence);
    if (listGroup == nullptr)
        return false;

    const Data::ServerList::ServerGroup& group = *listGroup;
    const std::wstring name = Core::Text::FromUtf8(group.name);
    const std::wstring description = Core::Text::FromUtf8(group.description);
    wcsncpy_s(pServerGroup->m_szName, name.c_str(), _TRUNCATE);
    wcsncpy_s(pServerGroup->m_szDescription, description.c_str(), _TRUNCATE);
    pServerGroup->m_iSequence = sequence;
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

std::string CServerListManager::GetServerLoadText(const CServerInfo& server) const
{
    const Data::ServerList::LoadText* look = m_serverList.load.Text(Data::ServerList::GetLoadState(server.m_iPercent));
    if (look == nullptr)
    {
        return {};
    }
    const std::string_view locale = GetGameLocale();
    return Data::ServerList::GetButtonLoadText(m_serverList, look->text.Get(locale), locale);
}

std::string CServerListManager::GetServerButtonName(const CServerGroup& group, int index) const
{
    if (index < 1 || index > MAX_SERVER_PER_GROUP)
    {
        return {};
    }
    const std::string name = Core::Text::ToUtf8(group.m_szName);
    Data::ServerList::ChannelNameValues values;
    values.name = name;
    values.channel = index;
    return Data::ServerList::BuildChannelName(m_serverList, Data::ServerList::ChannelNamePlace::Button,
                                              group.m_channels[static_cast<size_t>(index - 1)], values,
                                              GetGameLocale());
}

std::vector<CServerListManager::ButtonLoadText> CServerListManager::GetButtonLoadTexts() const
{
    const std::string_view locale = GetGameLocale();
    std::vector<ButtonLoadText> texts;
    for (const Data::ServerList::LoadState state :
         {Data::ServerList::LoadState::Full, Data::ServerList::LoadState::Overloaded})
    {
        const Data::ServerList::LoadText* look = m_serverList.load.Text(state);
        if (look == nullptr)
        {
            continue;
        }
        // A template without {load} shows none of them.
        std::string text = Data::ServerList::GetButtonLoadText(m_serverList, look->text.Get(locale), locale);
        if (text.empty())
        {
            continue;
        }
        texts.push_back({std::move(text), look->bold, look->italic});
    }
    return texts;
}

Data::ServerList::ButtonColor CServerListManager::GetServerButtonColor(const CServerInfo& server,
                                                                       std::string_view theme) const
{
    return Data::ServerList::GetButtonColor(m_serverList, server.m_channel, theme);
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

CServerInfo* CServerListManager::FindServer(int iConnectIndex, CServerGroup*& pServerGroup)
{
    for (const auto& entry : m_mapServerGroup)
    {
        for (CServerInfo* pServerInfo : entry.second->m_listServerInfo)
        {
            if (pServerInfo->m_iConnectIndex == iConnectIndex)
            {
                pServerGroup = entry.second;
                return pServerInfo;
            }
        }
    }

    pServerGroup = nullptr;
    return nullptr;
}

void CServerListManager::SetSelectServerInfo(int groupId, const wchar_t* pszName, int iIndex,
                                             const Data::ServerList::ChannelSettings& channel)
{
    m_iSelectServerGroupId = groupId;
    wcsncpy_s(m_szSelectServerName, pszName, _TRUNCATE);
    m_iSelectServerIndex = iIndex;
    m_selectChannel = channel;
    ++m_titleVersion;
}

const std::string& CServerListManager::GetSelectServerTitleUtf8(Data::ServerList::ChannelNamePlace place)
{
    CachedTitle& title = m_titles[static_cast<size_t>(place)];
    const std::string_view locale = GetGameLocale();
    if (title.version != m_titleVersion || title.locale != locale)
    {
        title.text = Core::Text::ToUtf8(GetSelectServerTitle(place).c_str());
        title.locale = locale;
        title.version = m_titleVersion;
    }
    return title.text;
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
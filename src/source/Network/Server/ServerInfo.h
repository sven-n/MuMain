// ServerInfo.h: interface for the CServerInfo class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Data/GameData/ServerListData/ChannelSettings.h"

class CServerInfo
{
public:
    CServerInfo();
    virtual ~CServerInfo();

    int m_iSequence;
    int m_iIndex;
    int m_iConnectIndex;
    int m_iPercent;
    // PvP and the VIP type, from the server list (CServerListManager::GetServerButtonName names it).
    Data::ServerList::ChannelSettings m_channel;
    // A channel the connect server didn't send, which only MuEditor's preview shows
    // (CServerListManager::SetOfflinePreview); it can't be joined.
    bool m_offline = false;
};

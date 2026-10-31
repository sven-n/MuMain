// ServerInfo.h: interface for the CServerInfo class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Data/GameData/ServerListData/ServerListGroup.h"

class CServerInfo
{
public:
    CServerInfo();
    virtual ~CServerInfo();

    int m_iSequence;
    int m_iIndex;
    int m_iConnectIndex;
    int m_iPercent;
    // PvP and the VIP type, from the server list (CServerListManager::GetServerButtonText names it).
    Data::ServerList::ChannelSettings m_channel;
};

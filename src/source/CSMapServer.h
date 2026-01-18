//////////////////////////////////////////////////////////////////////////
//  CSMapServer.h
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <string>

#include "Singleton.h"

struct MapServerInfo
{
    std::array<char, 16> m_szMapSvrIpAddress {};
    std::uint16_t m_wMapSvrPort {0};
    std::uint16_t m_wMapSvrCode {0};
    std::int32_t m_iJoinAuthCode1 {0};
    std::int32_t m_iJoinAuthCode2 {0};
    std::int32_t m_iJoinAuthCode3 {0};
    std::int32_t m_iJoinAuthCode4 {0};
};

using MServerInfo = MapServerInfo;

class CSMServer : public Singleton<CSMServer>
{
public:
    CSMServer();
    ~CSMServer() = default;

    void Init(void);

    void SetHeroID(wchar_t* ID);

    void SetServerInfo(MServerInfo sInfo);
    void GetServerInfo(MServerInfo& sInfo);

    void GetServerAddress(wchar_t* szAddress);
    std::uint16_t GetServerPort() const { return (m_hasServerInfo ? m_serverInfo.m_wMapSvrPort : 0); }
    std::uint16_t GetServerCode() const { return (m_hasServerInfo ? m_serverInfo.m_wMapSvrCode : 0); }

    void ConnectChangeMapServer(MServerInfo sInfo);
    void SendChangeMapServer(void);

private:
    std::wstring m_heroId;
    MapServerInfo m_serverInfo {};
    bool m_hasServerInfo {false};
};

#define g_csMapServer CSMServer::GetSingleton()

#include "stdafx.h"

#include "CSMapServer.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <cwchar>
#include <thread>

#include "MultiLanguage.h"
#include "WSclient.h"
#include "ZzzCharacter.h"
#include "ZzzOpenData.h"

extern int LogIn;
extern wchar_t LogInID[MAX_USERNAME_SIZE + 1];
extern int HeroKey;
extern BYTE Version[SIZE_PROTOCOLVERSION];
extern BYTE Serial[SIZE_PROTOCOLSERIAL + 1];

namespace
{
constexpr std::size_t kIpAddressLength = 16;
constexpr std::chrono::milliseconds kReconnectDelay(20);

std::uint32_t GetCurrentTickMilliseconds()
{
    const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch());
    return static_cast<std::uint32_t>(now.count());
}
} // namespace

static CSMServer g_csMapServerInstance;

CSMServer::CSMServer()
{
    Init();
}

void CSMServer::Init(void)
{
    m_hasServerInfo = false;
    m_serverInfo = {};
    m_heroId.clear();
}

void CSMServer::SetHeroID(wchar_t* ID)
{
    if (ID == nullptr)
    {
        m_heroId.clear();
        return;
    }

    m_heroId.assign(ID);
}

void CSMServer::SetServerInfo(MServerInfo sInfo)
{
    m_serverInfo = sInfo;
    m_hasServerInfo = true;
}

void CSMServer::GetServerInfo(MServerInfo& sInfo)
{
    if (m_hasServerInfo)
    {
        sInfo = m_serverInfo;
    }
    else
    {
        sInfo = {};
    }
}

void CSMServer::GetServerAddress(wchar_t* szAddress)
{
    if (szAddress == nullptr)
    {
        return;
    }

    std::wmemset(szAddress, 0, kIpAddressLength);

    if (!m_hasServerInfo)
    {
        return;
    }

    CMultiLanguage::ConvertFromUtf8(
        szAddress,
        m_serverInfo.m_szMapSvrIpAddress.data(),
        static_cast<int>(kIpAddressLength));
}

extern BOOL g_bGameServerConnected;
void CSMServer::ConnectChangeMapServer(MServerInfo sInfo)
{
    SetServerInfo(sInfo);

    if (!m_hasServerInfo || LogIn == 0)
    {
        return;
    }

    DeleteSocket();
    SaveOptions();
    SaveMacro(L"Data\\Macro.txt");

    std::this_thread::sleep_for(kReconnectDelay);

    std::array<wchar_t, kIpAddressLength> ipAddress {};
    CMultiLanguage::ConvertFromUtf8(
        ipAddress.data(),
        m_serverInfo.m_szMapSvrIpAddress.data(),
        static_cast<int>(kIpAddressLength));

    if (CreateSocket(ipAddress.data(), m_serverInfo.m_wMapSvrPort))
    {
        g_bGameServerConnected = TRUE;
    }
}

void CSMServer::SendChangeMapServer(void)
{
    if (!m_hasServerInfo || LogIn == 0)
    {
        return;
    }

    constexpr std::size_t kCharacterIdLength = MAX_USERNAME_SIZE + 1;
    constexpr std::size_t kPacketBufferLength = MAX_USERNAME_SIZE + 2;

    std::array<wchar_t, kCharacterIdLength> characterIdBuffer {};

    const std::size_t heroIdCopyLength = std::min<std::size_t>(m_heroId.size(), MAX_USERNAME_SIZE);
    std::wmemcpy(characterIdBuffer.data(), m_heroId.c_str(), heroIdCopyLength);
    characterIdBuffer[heroIdCopyLength] = L'\0';

    ClearCharacters(-1);
    InitGame();

    std::array<wchar_t, kPacketBufferLength> loginIdBuffer {};
    std::array<wchar_t, kPacketBufferLength> characterPacketBuffer {};

    // TODO: Populate loginIdBuffer with LogInID if credentials are required again.
    std::wmemcpy(characterPacketBuffer.data(), characterIdBuffer.data(), heroIdCopyLength + 1);

    BuxConvert(reinterpret_cast<BYTE*>(loginIdBuffer.data()), kPacketBufferLength);
    SocketClient->ToGameServer()->SendServerChangeAuthentication(
        reinterpret_cast<BYTE*>(loginIdBuffer.data()),
        MAX_USERNAME_SIZE,
        reinterpret_cast<BYTE*>(characterPacketBuffer.data()),
        MAX_USERNAME_SIZE,
        m_serverInfo.m_iJoinAuthCode1,
        m_serverInfo.m_iJoinAuthCode2,
        m_serverInfo.m_iJoinAuthCode3,
        m_serverInfo.m_iJoinAuthCode4,
        GetCurrentTickMilliseconds(),
        Version,
        SIZE_PROTOCOLVERSION,
        Serial,
        SIZE_PROTOCOLSERIAL);
}
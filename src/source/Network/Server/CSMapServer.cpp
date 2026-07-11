#include "stdafx.h"

#include "Network/Server/CSMapServer.h"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <cwchar>
#include <thread>

#include "Data/Translation/MultiLanguage.h"
#include "Network/Server/WSclient.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzOpenData.h"

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

    // The wire fields for the login ID and character name are fixed-width narrow
    // byte arrays (MAX_USERNAME_SIZE each). The extra byte gives the conversion
    // room for a null terminator so a full MAX_USERNAME_SIZE-character name is
    // not truncated; only the first MAX_USERNAME_SIZE bytes are encrypted and
    // sent.
    constexpr std::size_t kNameFieldLength = MAX_USERNAME_SIZE + 1;

    // Convert the wide character name down to narrow wire bytes. Reinterpreting
    // the wchar_t buffer as bytes would embed the platform-dependent wchar_t
    // width (2 bytes on Windows, 4 on Linux) and interleave null bytes into the
    // field, corrupting the name the server reads.
    std::array<char, kNameFieldLength> characterName {};
    CMultiLanguage::ConvertToUtf8(characterName.data(), m_heroId.c_str(), static_cast<int>(characterName.size()));

    ClearCharacters(-1);
    InitGame();

    // TODO: Populate loginId with LogInID if credentials are required again.
    // The map change re-authenticates via the auth codes, so it is left empty.
    std::array<char, kNameFieldLength> loginId {};

    BuxConvert(reinterpret_cast<BYTE*>(loginId.data()), MAX_USERNAME_SIZE);
    BuxConvert(reinterpret_cast<BYTE*>(characterName.data()), MAX_USERNAME_SIZE);
    SocketClient->ToGameServer()->SendServerChangeAuthentication(
        reinterpret_cast<BYTE*>(loginId.data()),
        MAX_USERNAME_SIZE,
        reinterpret_cast<BYTE*>(characterName.data()),
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
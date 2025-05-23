#include "stdafx.h"
#include "ZzzCharacter.h"
#include "ZzzTexture.h"
#include "zzzOpenData.h"

#include "CSMapServer.h"


#include "WSclient.h"

extern int  LogIn;
extern wchar_t LogInID[MAX_ID_SIZE + 1];
extern int HeroKey;
extern BYTE Version[SIZE_PROTOCOLVERSION];
extern BYTE Serial[SIZE_PROTOCOLSERIAL + 1];

static  CSMServer csMapServer;

CSMServer::CSMServer()
{
    Init();
}

void CSMServer::Init(void)
{
    m_bFillServerInfo = false;
    memset(&m_vServerInfo, 0, sizeof(MServerInfo));
}

void CSMServer::SetHeroID(wchar_t* ID)
{
    m_strHeroID = ID;
}

void CSMServer::SetServerInfo(MServerInfo sInfo)
{
    memcpy(&m_vServerInfo, &sInfo, sizeof(MServerInfo));

    m_bFillServerInfo = true;
}

void CSMServer::GetServerInfo(MServerInfo& sInfo)
{
    if (m_bFillServerInfo)
    {
        memcpy(&sInfo, &m_vServerInfo, sizeof(MServerInfo));
    }
    else
    {
        memset(&sInfo, 0, sizeof(MServerInfo));
    }
}

void CSMServer::GetServerAddress(wchar_t* szAddress)
{
    if (m_bFillServerInfo)
    {
        memcpy(szAddress, m_vServerInfo.m_szMapSvrIpAddress, sizeof(char) * 16);
    }
    else
    {
        memset(szAddress, 0, sizeof(char) * 16);
    }
}

extern BOOL g_bGameServerConnected;
void CSMServer::ConnectChangeMapServer(MServerInfo sInfo)
{
    SetServerInfo(sInfo);

    if (m_bFillServerInfo && LogIn != 0)
    {
        DeleteSocket();
        SaveOptions();
        SaveMacro(L"Data\\Macro.txt");

        ::Sleep(20);

        wchar_t ip[16];
        CMultiLanguage::ConvertFromUtf8(ip, m_vServerInfo.m_szMapSvrIpAddress);

        if (CreateSocket(ip, m_vServerInfo.m_wMapSvrPort))
        {
            g_bGameServerConnected = TRUE;
        }
    }
}

void CSMServer::SendChangeMapServer(void)
{
    if (m_bFillServerInfo == false || LogIn == 0) return;

    wchar_t  CharID[MAX_ID_SIZE + 1];

    wcscpy(CharID, m_strHeroID.c_str());
    //	memcpy ( CharID, m_strHeroID.c_str(), MAX_ID_SIZE );
    CharID[MAX_ID_SIZE] = NULL;

    ClearCharacters(-1);
    InitGame();
    
    wchar_t lpszID[MAX_ID_SIZE + 2];
    wchar_t lpszCHR[MAX_ID_SIZE + 2];
    ZeroMemory(lpszID, MAX_ID_SIZE + 2);
    ZeroMemory(lpszCHR, MAX_ID_SIZE + 2);
    // TODO wcscpy(lpszID, LogInID);
    wcscpy(lpszCHR, CharID);
    BuxConvert((BYTE*)lpszID, MAX_ID_SIZE + 2);
    SocketClient->ToGameServer()->SendServerChangeAuthentication(
        (BYTE*)lpszID, MAX_ID_SIZE,
        (BYTE*)CharID, MAX_ID_SIZE,
        m_vServerInfo.m_iJoinAuthCode1,
        m_vServerInfo.m_iJoinAuthCode2,
        m_vServerInfo.m_iJoinAuthCode3,
        m_vServerInfo.m_iJoinAuthCode4,
        GetTickCount(),
        Version,
        SIZE_PROTOCOLVERSION,
        Serial,
        SIZE_PROTOCOLSERIAL);
    //SendChangeMServer(LogInID, CharID, m_vServerInfo.m_iJoinAuthCode1, m_vServerInfo.m_iJoinAuthCode2, m_vServerInfo.m_iJoinAuthCode3, m_vServerInfo.m_iJoinAuthCode4);
}
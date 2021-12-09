//////////////////////////////////////////////////////////////////////////
//  CSMapServer.h
//////////////////////////////////////////////////////////////////////////
#ifndef __CS_MAP_SERVER_H__
#define __CS_MAP_SERVER_H__

#include "Singleton.h"
typedef struct
{
    char            m_szMapSvrIpAddress[16];
    WORD            m_wMapSvrPort;
    WORD            m_wMapSvrCode;
    int             m_iJoinAuthCode1;
    int             m_iJoinAuthCode2;
    int             m_iJoinAuthCode3;
    int             m_iJoinAuthCode4;
}MServerInfo;

class CSMServer : public Singleton<CSMServer>
{
private:
    bool        m_bFillServerInfo;
    std::string m_strHeroID;
    MServerInfo m_vServerInfo;

public:
    CSMServer ();
    ~CSMServer (){}

    void    Init ( void );

    void    SetHeroID ( char* ID );

    void    SetServerInfo ( MServerInfo sInfo );
    void    GetServerInfo ( MServerInfo& sInfo );

    void    GetServerAddress ( char* szAddress );
    WORD    GetServerPort ( void ) { return (m_bFillServerInfo ? m_vServerInfo.m_wMapSvrPort : 0); }
    WORD    GetServerCode ( void ) { return (m_bFillServerInfo ? m_vServerInfo.m_wMapSvrCode : 0); }

    void    ConnectChangeMapServer ( MServerInfo sInfo );
    void    SendChangeMapServer ( void );
};


#define g_csMapServer CSMServer::GetSingleton ()

#endif// __CS_MAP_SERVER_H__


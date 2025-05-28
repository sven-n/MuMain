//////////////////////////////////////////////////////////////////////
// ServerGroup.h: interface for the CServerGroup class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "ServerInfo.h"

typedef std::list<CServerInfo*>	type_listServer;

class CServerGroup
{
public:
    enum SERVER_BTN_POSITION
    {
        SBP_LEFT = 0,
        SBP_RIGHT = 1,
        SBP_CENTER = 2,
    };

public:
    CServerGroup();
    virtual ~CServerGroup();

public:
    void Release();

public:
    int		m_iSequence;
    int		m_iWidthPos;
    int		m_iBtnPos;
    int		m_iServerIndex;
    int		m_iNumServer;
    bool	m_bPvPServer;
    BYTE	m_abyNonPvpServer[MAX_SERVER_PER_GROUP];
    wchar_t	m_szName[MAX_TEXT_LENGTH];
    wchar_t	m_szDescription[MAX_TEXT_LENGTH];

    type_listServer				m_listServerInfo;

protected:
    type_listServer::iterator	m_iterServerList;

public:
    void InsertServerInfo(CServerInfo* pServerInfo);

    void SetFirst();
    bool GetNext(OUT CServerInfo*& pServerInfo);

    int GetServerSize();
    CServerInfo* GetServerInfo(int iSequence);
};

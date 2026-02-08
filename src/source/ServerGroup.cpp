#include "stdafx.h"
#include "ServerGroup.h"

CServerGroup::CServerGroup()
{
    m_iSequence = 0;
    m_iServerIndex = 0;
    m_iNumServer = 0;

    for (int i = 0; i < MAX_SERVER_PER_GROUP; i++)
    {
        m_abyNonPvpServer[i] = 0;
    }
}

CServerGroup::~CServerGroup()
{
    Release();
}

void CServerGroup::Release()
{
    auto iterServer = m_listServerInfo.begin();
    for (; iterServer != m_listServerInfo.end(); iterServer++)
    {
        delete (*iterServer);
    }

    m_listServerInfo.clear();
}

void CServerGroup::InsertServerInfo(CServerInfo* pServerInfo)
{
    m_listServerInfo.push_back(pServerInfo);

    m_iterServerList = m_listServerInfo.begin();

    m_iNumServer = m_listServerInfo.size();
}

void CServerGroup::SetFirst()
{
    m_iterServerList = m_listServerInfo.begin();
}

bool CServerGroup::GetNext(OUT CServerInfo*& pServerInfo)
{
    if (m_iterServerList == m_listServerInfo.end())
    {
        pServerInfo = NULL;

        return false;
    }

    pServerInfo = (*m_iterServerList);

    m_iterServerList++;

    return true;
}

int CServerGroup::GetServerSize()
{
    return m_listServerInfo.size();
}

CServerInfo* CServerGroup::GetServerInfo(int iSequence)
{
    auto iterServer = m_listServerInfo.begin();

    while (iterServer != m_listServerInfo.end())
    {
        if ((*iterServer)->m_iSequence == iSequence)
            return (*iterServer);

        iterServer++;
    }

    return NULL;
}
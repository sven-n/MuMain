// ServerGroup.cpp: implementation of the CServerGroup class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef KJH_ADD_SERVER_LIST_SYSTEM

#include "ServerGroup.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerGroup::CServerGroup()
{
	m_iSequence		= 0;				// 서버 순서
	m_iServerIndex	= 0;				// 서버 고유번호
	m_iNumServer	= 0;				// 서버 갯수
	
	for( int i=0 ; i<MAX_SERVER_LOW ; i++ )
	{
#ifdef ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
		m_abyNonPvpServer[i] = 0;
#else	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
		m_bNonPvpServer[i] = false;	// NonPvP서버
#endif	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	}
}

CServerGroup::~CServerGroup()
{
	Release();
}

void CServerGroup::Release()
{
	type_listServer::iterator iterServer = m_listServerInfo.begin();
	for( ; iterServer!=m_listServerInfo.end() ; iterServer++ )
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

bool CServerGroup::GetNext(OUT CServerInfo* &pServerInfo)
{
	if( m_iterServerList == m_listServerInfo.end() )
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
	type_listServer::iterator iterServer = m_listServerInfo.begin();
	
	while(iterServer != m_listServerInfo.end())
	{
		if( (*iterServer)->m_iSequence == iSequence )
			return (*iterServer);

		iterServer++;
	}
	
	return NULL;
}

#endif // KJH_ADD_SERVER_LIST_SYSTEM

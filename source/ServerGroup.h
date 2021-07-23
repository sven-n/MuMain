// ServerGroup.h: interface for the CServerGroup class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERGROUP_H__2C57D7A7_11E9_42E8_812A_8D1B3E311657__INCLUDED_)
#define AFX_SERVERGROUP_H__2C57D7A7_11E9_42E8_812A_8D1B3E311657__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_ADD_SERVER_LIST_SYSTEM

#include "ServerInfo.h"

typedef std::list<CServerInfo*>	type_listServer;

class CServerGroup  
{
public:
#ifdef ASG_ADD_SERVER_LIST_SCRIPTS
	enum SERVER_BTN_POSITION
	{
		SBP_LEFT	= 0,
		SBP_RIGHT	= 1,
		SBP_CENTER	= 2,
	};
#else	// ASG_ADD_SERVER_LIST_SCRIPTS
	enum SERVER_BTN_POSITION
	{
		SBP_CENTER	= 0,
		SBP_LEFT	= 1,
		SBP_RIGHT	= 2,
	};
#endif	// ASG_ADD_SERVER_LIST_SCRIPTS

public:
	CServerGroup();
	virtual ~CServerGroup();

public:
	void Release();

public:
	int		m_iSequence;				// 서버순위(오름차순)
	int		m_iWidthPos;				// 서버버튼의 가로 포지션 (enum SERVER_BTN_POSITION 참조)
	int		m_iBtnPos;					// 서버버튼의 실제 포지션 - 버튼정의할때 기입한다.
	int		m_iServerIndex;				// 서버 고유번호
	int		m_iNumServer;				// 서버 갯수
	bool	m_bPvPServer;				// PvP서버우뮤
#ifdef ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	BYTE	m_abyNonPvpServer[MAX_SERVER_LOW];	// 0: 일반 Pvp, 1: 일반 NonPvp, 2: 유료채널 Pvp, 3: 유료채널 NonPvp
#else	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	bool	m_bNonPvpServer[MAX_SERVER_LOW];	// NonPvP서버(ServerGroup이 PvP 면 모두 false)
#endif	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	unicode::t_char	m_szName[MAX_TEXT_LENGTH];				// 서버이름
	unicode::t_char	m_szDescription[MAX_TEXT_LENGTH];		// 서버설명

	type_listServer				m_listServerInfo;		// 서버 정보 리스트
	
protected:
	type_listServer::iterator	m_iterServerList;		// 서버 정보리스트 iterator


public:
	void InsertServerInfo(CServerInfo* pServerInfo);

	void SetFirst();
	bool GetNext(OUT CServerInfo* &pServerInfo);

	int GetServerSize();
	CServerInfo* GetServerInfo(int iSequence);
};

#endif // KJH_ADD_SERVER_LIST_SYSTEM

#endif // !defined(AFX_SERVERGROUP_H__2C57D7A7_11E9_42E8_812A_8D1B3E311657__INCLUDED_)

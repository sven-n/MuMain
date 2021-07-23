// ServerInfo.h: interface for the CServerInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERINFO_H__59F799C9_1587_4E04_8151_1D7AC4D2884D__INCLUDED_)
#define AFX_SERVERINFO_H__59F799C9_1587_4E04_8151_1D7AC4D2884D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_ADD_SERVER_LIST_SYSTEM

class CServerInfo  
{
public:
	CServerInfo();
	virtual ~CServerInfo();

	int			m_iSequence;
	int			m_iIndex;
	int			m_iConnectIndex;
	int			m_iPercent;
#ifdef ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	BYTE		m_byNonPvP;
#else	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	bool		m_bNonPvP;
#endif	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	unicode::t_char m_bName[MAX_TEXT_LENGTH];
};

#endif KJH_ADD_SERVER_LIST_SYSTEM

#endif // !defined(AFX_SERVERINFO_H__59F799C9_1587_4E04_8151_1D7AC4D2884D__INCLUDED_)

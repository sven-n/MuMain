// ServerListManager.h: interface for the CServerListManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERLISTMANAGER_H__9FFD345C_6AD5_4D74_9194_FEF1D016B3FD__INCLUDED_)
#define AFX_SERVERLISTMANAGER_H__9FFD345C_6AD5_4D74_9194_FEF1D016B3FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_ADD_SERVER_LIST_SYSTEM

#include "ServerGroup.h"

#ifdef ASG_ADD_SERVER_LIST_SCRIPTS
using namespace std;

#define SLM_MAX_SERVER_NAME_LENGTH	32		// 서버 이름 길이
#define SLM_MAX_SERVER_COUNT		15		// 서버군 내의 서버 개수

// 서버군 정보 구조체
struct SServerGroupInfo
{
	char	m_szName[SLM_MAX_SERVER_NAME_LENGTH];	// 서버군 이름
	BYTE	m_byPos;							// 서버군 위치(좌:0, 우:1, 하:2)
	BYTE	m_bySequence;						// 서버군 순서
	BYTE	m_abyNonPVP[SLM_MAX_SERVER_COUNT];	// NonPVP 서버인가?(서버군에서 각각 서버의 NonPVP 설정)
	string	m_strDescript;						// 서버군 설명 문자열
};

typedef map<WORD, SServerGroupInfo> ServerListScriptMap;
#endif	// ASG_ADD_SERVER_LIST_SCRIPTS

typedef std::map<int, CServerGroup*>	type_mapServerGroup;

class CServerListManager  
{
protected:
	CServerListManager();
public:
	virtual ~CServerListManager();

public:
	static CServerListManager* GetInstance();

	void InsertServerGroup(int iConnectIndex, int iServerPercent);
	void Release();

#ifdef ASG_ADD_SERVER_LIST_SCRIPTS
	void LoadServerListScript();
#endif	// ASG_ADD_SERVER_LIST_SCRIPTS
#ifdef ASG_MOD_SERVER_LIST_CHANGE_MSG
	const char* GetNotice();
#endif	// ASG_MOD_SERVER_LIST_CHANGE_MSG
	
	void SetFirst();
	bool GetNext(OUT CServerGroup* &pServerGroup);
	CServerGroup* GetServerGroupByBtnPos(int iBtnPos);

	int GetServerGroupSize();

	// 현재 선택된 서버 정보
#ifdef ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	void SetSelectServerInfo(unicode::t_char* pszName, int iIndex, int iCensorshipIndex,
		BYTE byNonPvP, bool bTestServer);
#else	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	void SetSelectServerInfo(unicode::t_char* pszName, int iIndex, int iCensorshipIndex, bool bNonPvP, bool bTestServer);
#endif	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	unicode::t_char* GetSelectServerName();
	int	GetSelectServerIndex();
	int GetCensorshipIndex();
#ifdef ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	BYTE GetNonPVPInfo();
#endif	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	bool IsNonPvP();
	bool IsTestServer();
	void SetTotalServer(int iTotalServer);
	int GetTotalServer();			// 전체 서버 갯수
	
protected:
#ifdef ASG_ADD_SERVER_LIST_SCRIPTS
	const SServerGroupInfo* GetServerGroupInfoInScript(WORD wServerGroupIndex);
#endif	// ASG_ADD_SERVER_LIST_SCRIPTS
	bool MakeServerGroup(IN int iServerGroupIndex, OUT CServerGroup* pServerGroup);
	void InsertServer(CServerGroup* pServerGroup, int iConnectIndex, int iServerPercent);
#ifdef ASG_ADD_SERVER_LIST_SCRIPTS_GLOBAL_TEMP
	void BuxConvert(BYTE* pbyBuffer, int nSize);
#endif	// ASG_ADD_SERVER_LIST_SCRIPTS_GLOBAL_TEMP

public:
	type_mapServerGroup			m_mapServerGroup;
	type_mapServerGroup::iterator	m_iterServerGroup;

	int				m_iTotalServer;
	unicode::t_char m_szSelectServerName[MAX_TEXT_LENGTH];
	int				m_iSelectServerIndex;
	int				m_iCensorshipIndex;
#ifdef ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	BYTE			m_byNonPvP;
#else	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	bool			m_bNonPvP;
#endif	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	bool			m_bTestServer;

#ifdef ASG_ADD_SERVER_LIST_SCRIPTS
protected:
	ServerListScriptMap		m_mapServerListScript;	// ServerList.txt 내용이 여기에 저장.(서버 그룹 인덱스가 키)
#endif	// ASG_ADD_SERVER_LIST_SCRIPTS
#ifdef ASG_MOD_SERVER_LIST_CHANGE_MSG
	string					m_strNotice;			// 서버선택 화면에서의 공지사항.
#endif	// ASG_MOD_SERVER_LIST_CHANGE_MSG
};

#define g_ServerListManager CServerListManager::GetInstance()

#endif // KJH_ADD_SERVER_LIST_SYSTEM

#endif // !defined(AFX_SERVERLISTMANAGER_H__9FFD345C_6AD5_4D74_9194_FEF1D016B3FD__INCLUDED_)

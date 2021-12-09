//////////////////////////////////////////////////////////////////////
// ServerListManager.h: interface for the CServerListManager class.
//////////////////////////////////////////////////////////////////////
#pragma once
#include "ServerGroup.h"

#define SLM_MAX_SERVER_NAME_LENGTH	32
#define SLM_MAX_SERVER_COUNT		15

struct SServerGroupInfo
{
	char	m_szName[SLM_MAX_SERVER_NAME_LENGTH];
	BYTE	m_byPos;
	BYTE	m_bySequence;
	BYTE	m_abyNonPVP[SLM_MAX_SERVER_COUNT];
	std::string	m_strDescript;
};

typedef std::map<WORD, SServerGroupInfo> ServerListScriptMap;
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
	void LoadServerListScript();
	void SetFirst();
	bool GetNext(OUT CServerGroup* &pServerGroup);
	CServerGroup* GetServerGroupByBtnPos(int iBtnPos);

	int GetServerGroupSize();

	void SetSelectServerInfo(unicode::t_char* pszName, int iIndex, int iCensorshipIndex,
		BYTE byNonPvP, bool bTestServer);
	unicode::t_char* GetSelectServerName();
	int	GetSelectServerIndex();
	int GetCensorshipIndex();
	BYTE GetNonPVPInfo();
	bool IsNonPvP();
	bool IsTestServer();
	void SetTotalServer(int iTotalServer);
	int GetTotalServer();
	
protected:
	const SServerGroupInfo* GetServerGroupInfoInScript(WORD wServerGroupIndex);
	bool MakeServerGroup(IN int iServerGroupIndex, OUT CServerGroup* pServerGroup);
	void InsertServer(CServerGroup* pServerGroup, int iConnectIndex, int iServerPercent);
	void BuxConvert(BYTE* pbyBuffer, int nSize);

public:
	type_mapServerGroup			m_mapServerGroup;
	type_mapServerGroup::iterator	m_iterServerGroup;

	int				m_iTotalServer;
	unicode::t_char m_szSelectServerName[MAX_TEXT_LENGTH];
	int				m_iSelectServerIndex;
	int				m_iCensorshipIndex;
	BYTE			m_byNonPvP;
	bool			m_bTestServer;

protected:
	ServerListScriptMap		m_mapServerListScript;

};

#define g_ServerListManager CServerListManager::GetInstance()


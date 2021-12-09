#include "stdafx.h"
#include "./Utilities/Log/ErrorReport.h"
#include "GlobalPortalSystem.h"

#ifdef LDK_ADD_GLOBAL_PORTAL_SYSTEM

GlobalPortalSystem& GlobalPortalSystem::Instance()
{
	static GlobalPortalSystem m_GlobalPortalSys;
	return m_GlobalPortalSys;
}

GlobalPortalSystem::~GlobalPortalSystem()
{

}

void GlobalPortalSystem::Initialize()
{
	g_ErrorReport.Write( "-- 글로벌포탈 초기화 -- \r\n" );
	ZeroMemory(m_Id, sizeof(m_Id));

	m_isAuthSet = FALSE;
	m_isAuthSet = SetAuthInfo();
}

void GlobalPortalSystem::SetPathToRegistry(	HKEY* hKey )
{
	char szCurrentDir[256];
	ZeroMemory(szCurrentDir, sizeof(szCurrentDir));
	GetCurrentDirectory(256, szCurrentDir);
	strcat(szCurrentDir, "\\Mu.exe");
	RegSetValueEx( *hKey, "Exe", 0, REG_SZ, (const unsigned char*)szCurrentDir, sizeof(szCurrentDir) );

	//g_ErrorReport.Write( "%s\r\n", szCurrentDir );
}

bool GlobalPortalSystem::SetAuthInfo()
{
	if( m_isAuthSet )
	{
		// error log
		g_ErrorReport.Write( "-- 키정보 설정 함수 중복 호출 -- \r\n" );
		return FALSE;
	}

	char strGameName[128];
 	ZeroMemory(strGameName, sizeof(strGameName));
	strcat(strGameName, "MU");

 	WLAUTH_INFO AuthInfo = {0};
// 	StringCbPrintf(m_AuthInfo.id, sizeof(m_AuthInfo.id), _T("%s"), strUserID);
// 	RtlCopyMemory(&m_AuthInfo.key, (void*)strAuthCode, sizeof(m_AuthInfo.key));

	RMSM sm(CreateShareMem());
	int result = (sm.get())->ReadFromShareMemory(strGameName, &AuthInfo);
 	if( 0 != result )
  	{
		g_ErrorReport.Write( "-- 키정보 설정 실패 - %d -- \r\n", result );
  		return FALSE;
  	}

	strcpy( m_Id, AuthInfo.id );
	g_ErrorReport.Write("-- 키정보 id - %s -- ", AuthInfo.id);

	return TRUE;
}

bool GlobalPortalSystem::ResetAuthInfo()
{
	m_isAuthSet = FALSE;
	m_isAuthSet = SetAuthInfo();

	return m_isAuthSet;	
}

#endif //LDK_ADD_GLOBAL_PORTAL_SYSTEM

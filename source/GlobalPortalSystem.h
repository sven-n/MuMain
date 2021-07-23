// GlobalPortalSystem.h: interface for the GlobalPortalSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLOBALPORTALSYSTEM_H__CF0EA9D0_8E05_4094_9284_E3BAEC47BE40__INCLUDED_)
#define AFX_GLOBALPORTALSYSTEM_H__CF0EA9D0_8E05_4094_9284_E3BAEC47BE40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_MOD_LIBRARY_LINK_EACH_NATION
#include <ISharemem.h>
#else // KJH_MOD_LIBRARY_LINK_EACH_NATION
#include "ShareMemory/ISharemem.h"
#endif // KJH_MOD_LIBRARY_LINK_EACH_NATION

#ifdef LDK_ADD_GLOBAL_PORTAL_SYSTEM

class GlobalPortalSystem  
{
private:
	bool m_isAuthSet;

	// ISharemem.h
	// const DWORD WL_ID_LENGTH 	= 32;
	// const DWORD WL_KEY_LENGTH	= 260;
	char m_Id[WL_ID_LENGTH];

public:
	static GlobalPortalSystem& Instance();
	virtual ~GlobalPortalSystem();

	void Initialize();

	void SetPathToRegistry(HKEY* hKey);	// 레지스트리에 경로 남기기
	
	bool SetAuthInfo();			// 인증정보 획득
	bool ResetAuthInfo();		// 인증정보 갱신

	const char* GetAuthID() { return m_Id; }

	bool IsAuthSet() { return m_isAuthSet; }

private:
	GlobalPortalSystem() { Initialize(); }

};

#endif //LDK_ADD_GLOBAL_PORTAL_SYSTEM
#endif // !defined(AFX_GLOBALPORTALSYSTEM_H__CF0EA9D0_8E05_4094_9284_E3BAEC47BE40__INCLUDED_)

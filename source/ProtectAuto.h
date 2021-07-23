// ProtectAuto.h: interface for the CProtectAuto class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROTECTAUTO_H__29DC21E6_B3FB_4158_97EB_35D7F4CB00E5__INCLUDED_)
#define AFX_PROTECTAUTO_H__29DC21E6_B3FB_4158_97EB_35D7F4CB00E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef CSK_MOD_PROTECT_AUTO_V1

class CProtectAuto  
{
public:
	static CProtectAuto* ms_pProtectAuto;
	static CProtectAuto* Create();
	static CProtectAuto* Get();
	static void Destroy();

	CProtectAuto();
	virtual ~CProtectAuto();

	void SetNewVersion(bool bNew);
	bool IsNewVersion();

#ifdef YDG_MOD_CHECK_PROTECT_AUTO_FLAG
	bool CheckFlag(const std::string& strCommand);
#endif	// YDG_MOD_CHECK_PROTECT_AUTO_FLAG
	
#ifdef CSK_MOD_PROTECT_AUTO_V2
	// 오토프로그램이 상주되어 있는지 검사
	void CheckAuto();
	bool IsAutoProcess();

	// 오토프로그램을 작동시켰는지 검사
	void Check();
	void SetStartAuto(bool bStart);
	bool IsStartAuto();
#endif // CSK_MOD_PROTECT_AUTO_V2
#ifdef YDG_MOD_PROTECT_AUTO_V5
	BOOL FindAutoTool(char * pszProcessName);
#endif	// YDG_MOD_PROTECT_AUTO_V5
#ifdef YDG_MOD_PROTECT_AUTO_FIND_USB
	BOOL FindDevice();
#endif	// YDG_MOD_PROTECT_AUTO_FIND_USB
#ifdef YDG_MOD_PROTECT_AUTO_TEST
	BOOL BlockOpenProcess();
#endif	// YDG_MOD_PROTECT_AUTO_TEST

private:
#ifdef YDG_MOD_CHECK_PROTECT_AUTO_FLAG
	bool m_bNewVersion;
#endif	// YDG_MOD_CHECK_PROTECT_AUTO_FLAG

#ifdef CSK_MOD_PROTECT_AUTO_V2
	bool m_bAutoProcess;
	bool m_bStartAuto;
#endif // CSK_MOD_PROTECT_AUTO_V2

#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
public:
	BYTE * m_pbtKeyAddressBlock4;
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3
#ifdef KJH_MOD_PROTECT_AUTO_FIND_USB
	bool m_bFindRegisteredAutoUSB;		// 레지스터에 등록된 오토USB 검출
#endif // KJH_MOD_PROTECT_AUTO_FIND_USB
};

#define g_pProtectAuto CProtectAuto::Get()

#endif // CSK_MOD_PROTECT_AUTO_V1

#endif // !defined(AFX_PROTECTAUTO_H__29DC21E6_B3FB_4158_97EB_35D7F4CB00E5__INCLUDED_)

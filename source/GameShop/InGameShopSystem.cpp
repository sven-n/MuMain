// InGameShopSystem.cpp: implementation of the InGameShopSystem class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "InGameShopSystem.h"

#include "WSclientinline.h"
#include "ZzzInventory.h"
#include "MsgBoxIGSCommon.h"
 
#ifdef CONSOLE_DEBUG
	#include "./Utilities/Log/muConsoleDebug.h"
#endif // CONSOLE_DEBUG

#ifdef FOR_WORK
	#include "./Utilities/Log/DebugAngel.h"
#endif // FOR_WORK


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInGameShopSystem::CInGameShopSystem()
{
	m_pCategoryList = NULL;		// Category 리스트 Pointer
	m_pPackageList = NULL;		// Packege 리스트 Pointer
	m_pProductList = NULL;		// Product 리스트 Pointer

	m_pBannerList = NULL;		// Banner 리스트 Pointer

#ifdef KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG
	memset(&m_ScriptVerInfo, -1, sizeof(CListVersionInfo));
	memset(&m_BannerVerInfo, -1, sizeof(CListVersionInfo));
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	memset(&m_CurrentScriptVerInfo, -1, sizeof(CListVersionInfo));
	memset(&m_CurrentBannerVerInfo, -1, sizeof(CListVersionInfo));
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
#else // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG
	memset(&m_ScriptVerInfo, 0, sizeof(CListVersionInfo));
	memset(&m_BannerVerInfo, 0, sizeof(CListVersionInfo));
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	memset(&m_CurrentScriptVerInfo, 0, sizeof(CListVersionInfo));
	memset(&m_CurrentBannerVerInfo, 0, sizeof(CListVersionInfo));
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
#endif // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG

	m_bIsShopOpenLock = true; //louis
	m_bIsBanner	= false;

	m_bIsRequestEventPackage = false;

	m_plistSelectPackage = NULL;

#ifdef KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG
	m_bFirstScriptDownloaded = false;
	m_bFirstBannerDownloaded = false;
#endif // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG

#ifdef FOR_WORK
	remove("InGameShopStatue.Txt");			// 기존 로그 파일 지움
#endif // FOR_WORK
}

CInGameShopSystem::~CInGameShopSystem()
{
	Release();
}

CInGameShopSystem* CInGameShopSystem::GetInstance()
{
	static CInGameShopSystem s_InGameShopSystem;
	return &s_InGameShopSystem;
}

void CInGameShopSystem::Initalize()
{
	m_mapZoneSeqIndex.clear();
	m_listDisplayPackage.clear();
	m_listNormalPackage.clear();
	m_listEventPackage.clear();
	m_listZoneName.clear();
	m_listCategoryName.clear();
	m_plistSelectPackage = &m_listNormalPackage;
	m_dTotalCash	= 0;
	m_dTotalPoint	= 0;
	m_dTotalMileage	= 0;
	m_dCashCreditCard	= 0; 
	m_dCashPrepaid		= 0;	

#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT			// #ifndef
	m_iStorageItemCnt	= 0;
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
	m_iEventPackageCnt	= 0;
	m_iSelectedPage = 1;
	m_iTotalEventPackage = 0;
	m_iCntSelectEventZone = 0;
	m_bSelectEventCategory = false;
	m_bIsRequestShopOpenning = false;
	m_bAbleRequestEventPackage = true;
	InitZoneInfo();
}

void CInGameShopSystem::Release()
{
	m_mapZoneSeqIndex.clear();
	m_listDisplayPackage.clear();
	m_listNormalPackage.clear();
	m_listEventPackage.clear();
	m_listZoneName.clear();
	m_listCategoryName.clear();

	m_pCategoryList = NULL;
	m_pPackageList = NULL;
	m_pProductList = NULL;
}

void CInGameShopSystem::SetScriptVersion(int iSalesZone, int iYear, int iYearId)
{
	m_ScriptVerInfo.Zone = iSalesZone;
	m_ScriptVerInfo.year = iYear;
	m_ScriptVerInfo.yearId = iYearId;
}

void CInGameShopSystem::SetBannerVersion(int iSalesZone, int iYear, int iYearId)
{
	m_BannerVerInfo.Zone = iSalesZone;
	m_BannerVerInfo.year = iYear;
	m_BannerVerInfo.yearId = iYearId;
}

bool CInGameShopSystem::ScriptDownload()
{
	m_bFirstScriptDownloaded = true;
#ifndef KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG2			// #ifndef
	m_CurrentScriptVerInfo = m_ScriptVerInfo;
#endif // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG2

	::GetCurrentDirectory(256, m_szScriptLocalPath);
#ifdef KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION
	char szScriptRemotePathforDMZ[MAX_TEXT_LENGTH];
	sprintf(m_szScriptLocalPath, "%s%s", m_szScriptLocalPath, "\\data\\InGameShopScript");
	strcpy(m_szScriptIPAddress,			"image.webzen.com");
	strcpy(m_szScriptRemotePath,		"/Global/Payment/ProductTransfer");
	strcpy(szScriptRemotePathforDMZ,	"/Global/Payment/DevScriptGB/ProductTransfer");

#ifdef FOR_WORK
	HANDLE hFile; 
	hFile = CreateFile("dmz.ini",     // file to create
						GENERIC_READ,			// open for reading 
						0,						// do not share 
						NULL,                   // default security 
						OPEN_EXISTING,          // existing file only 
						FILE_ATTRIBUTE_NORMAL,  // normal file 
						NULL);                  // no template 
	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		strcpy(m_szScriptRemotePath,	szScriptRemotePathforDMZ);
	}
	CloseHandle(hFile);
#endif // FOR_WORK
	m_ShopManager.SetListManagerInfo(HTTP, m_szScriptIPAddress, 
									"", 
									"", 
									m_szScriptRemotePath, 
									m_szScriptLocalPath, 
									m_ScriptVerInfo,
									10000);
								
#else // KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION

	sprintf(m_szScriptLocalPath, "%s%s", m_szScriptLocalPath, "\\data\\InGameShopScript");
#ifdef KJH_MOD_SHOPSCRIPT_DOWNLOAD_DOMAIN
	strcpy(m_szIPAddress,			"image.webzen.co.kr");
#else // KJH_MOD_SHOPSCRIPT_DOWNLOAD_DOMAIN
	strcpy(m_szIPAddress,			"image.webzen.com");
#endif // KJH_MOD_SHOPSCRIPT_DOWNLOAD_DOMAIN

#ifdef FOR_WORK
	HANDLE hFile; 
	hFile = CreateFile("dmz.ini",     // file to create
						GENERIC_READ,			// open for reading 
						0,						// do not share 
						NULL,                   // default security 
						OPEN_EXISTING,          // existing file only 
						FILE_ATTRIBUTE_NORMAL,  // normal file 
						NULL);                  // no template 
	
	if (hFile == INVALID_HANDLE_VALUE)
	{
		strcpy(m_szScriptRemotePath,	"/ibs/Game/ProductTransfer");
	}
	else
	{
		strcpy(m_szScriptRemotePath,	"/ibs/Game/DevScript/ProductTransfer");
	}
	CloseHandle(hFile);
	
#else // FOR_WORK
	strcpy(m_szScriptRemotePath,	"/ibs/Game/ProductTransfer");
#endif // FOR_WORK

	// 스크립트 다운로드 Setting
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	m_ShopManager.SetListManagerInfo(HTTP, m_szIPAddress, 
										"", 
										"", 
										m_szScriptRemotePath, 
										m_szScriptLocalPath, 
										m_ScriptVerInfo,
#ifdef LJH_FIX_EXTENDING_OVER_MAX_TIME_4S_To_10S		
										10000);
#else	//LJH_FIX_EXTENDING_OVER_MAX_TIME_4S_To_10S										
										4000);
#endif	//LJH_FIX_EXTENDING_OVER_MAX_TIME_4S_To_10S										
										
#else // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	m_ShopManager.SetListManagerInfo(HTTP, m_szIPAddress, 
										"", 
										"", 
										m_szScriptRemotePath, 
										m_szScriptLocalPath, 
										m_ScriptVerInfo);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

#endif // KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION

	WZResult res = m_ShopManager.LoadScriptList(false);

	// DownLoad & Load 실패
	if(!res.IsSuccess())
	{
		m_pCategoryList = NULL;		// Category 리스트 Pointer
		m_pPackageList = NULL;		// Packege 리스트 Pointer
		m_pProductList = NULL;		// Product 리스트 Pointer

		ShopOpenLock();		// Shop OpenLock

		// MessageBox
		unicode::t_char szText[MAX_TEXT_LENGTH] = {'\0', };
		sprintf(szText, GlobalText[3029], m_ScriptVerInfo.Zone, m_ScriptVerInfo.year, m_ScriptVerInfo.yearId, res.GetErrorMessage());
		CMsgBoxIGSCommon* pMsgBox = NULL;
		CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
 		pMsgBox->Initialize(GlobalText[3028], szText);
				return false;
	}

#ifndef KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG			// #ifndef
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	m_CurrentScriptVerInfo = m_ScriptVerInfo;
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
#endif // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG


#ifdef KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG2
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	m_CurrentScriptVerInfo = m_ScriptVerInfo;
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
#endif // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG2

	

#ifdef FOR_WORK
	DebugAngel_Write("InGameShopStatue.Txt", "<IngameShop Script Download Success!!!>\r\n");
	DebugAngel_Write("InGameShopStatue.Txt", " - Ver %d.%d.%d\r\n", m_ScriptVerInfo.Zone, m_ScriptVerInfo.year, m_ScriptVerInfo.yearId);
#endif // FOR_WORK

	ShopOpenUnLock();

	CShopList* pShopList = m_ShopManager.GetListPtr();
	
	m_pCategoryList = pShopList->GetCategoryListPtr();
	m_pPackageList = pShopList->GetPackageListPtr();
	m_pProductList = pShopList->GetProductListPtr();

	return true;
}

bool CInGameShopSystem::BannerDownload()
{
#ifdef KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG
	m_bFirstBannerDownloaded = true;

#ifndef KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG2			// #ifndef
	// 현재 다운로드 받은 배너 버젼 동기화
	m_CurrentBannerVerInfo = m_BannerVerInfo;
#endif // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG2
#endif // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG

	::GetCurrentDirectory(256, m_szBannerLocalPath);

#ifdef KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION
	char szBannerRemotePathforDMZ[MAX_TEXT_LENGTH];
	sprintf(m_szBannerLocalPath, "%s%s", m_szBannerLocalPath, "\\data\\InGameShopBanner");

	strcpy(m_szBannerIPAddress,			"image.webzen.com");
	strcpy(m_szBannerRemotePath,		"/Global/Payment/BannerTransfer");
	strcpy(szBannerRemotePathforDMZ,	"/Global/Payment/DevScriptGB/BannerTransfer");

#ifdef FOR_WORK
	HANDLE hFile; 
	hFile = CreateFile("dmz.ini",     // file to create
						GENERIC_READ,			// open for reading 
						0,						// do not share 
						NULL,                   // default security 
						OPEN_EXISTING,          // existing file only 
						FILE_ATTRIBUTE_NORMAL,  // normal file 
						NULL);                  // no template 
	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		strcpy(m_szBannerRemotePath,	szBannerRemotePathforDMZ);
	}
	CloseHandle(hFile);
#endif // FOR_WORK
	
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	m_BannerManager.SetListManagerInfo(HTTP, m_szBannerIPAddress, 
										"", 
										"", 
										m_szBannerRemotePath, 
										m_szBannerLocalPath, 
										m_BannerVerInfo,
										4000);
#else // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	m_BannerManager.SetListManagerInfo(HTTP, m_szIPAddress, 
										"", 
										"", 
										m_szBannerRemotePath, 
										m_szBannerLocalPath, 
										m_BannerVerInfo);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD									

#else // KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION

	sprintf(m_szBannerLocalPath, "%s%s", m_szBannerLocalPath, "\\data\\InGameShopBanner");
#ifdef KJH_MOD_SHOPSCRIPT_DOWNLOAD_DOMAIN
	strcpy(m_szIPAddress,			"image.webzen.co.kr");
#else // KJH_MOD_SHOPSCRIPT_DOWNLOAD_DOMAIN
	strcpy(m_szIPAddress,			"image.webzen.com");
#endif // KJH_MOD_SHOPSCRIPT_DOWNLOAD_DOMAIN

#ifdef FOR_WORK
	HANDLE hFile; 
	hFile = CreateFile("dmz.ini",     // file to create
						GENERIC_READ,			// open for reading 
						0,						// do not share 
						NULL,                   // default security 
						OPEN_EXISTING,          // existing file only 
						FILE_ATTRIBUTE_NORMAL,  // normal file 
						NULL);                  // no template 

	if (hFile == INVALID_HANDLE_VALUE)
	{
		strcpy(m_szBannerRemotePath,	"/ibs/Game/BannerTransfer");
	}
	else
	{
		strcpy(m_szBannerRemotePath,	"/ibs/Game/DevScript/BannerTransfer");
	}
	CloseHandle(hFile);

#else // FOR_WORK
	strcpy(m_szBannerRemotePath,	"/ibs/Game/BannerTransfer");
#endif // FOR_WORK
	
	// 스크립트 다운로드 Setting
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	m_BannerManager.SetListManagerInfo(HTTP, m_szIPAddress, 
										"", 
										"", 
										m_szBannerRemotePath, 
										m_szBannerLocalPath, 
										m_BannerVerInfo,
										4000);
#else // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	m_BannerManager.SetListManagerInfo(HTTP, m_szIPAddress, 
										"", 
										"", 
										m_szBannerRemotePath, 
										m_szBannerLocalPath, 
										m_BannerVerInfo);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
#endif // KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION
	
	// DownLoad & Load
	WZResult res = m_BannerManager.LoadScriptList(false);
	
	// DownLoad & Load 실패
	if(!res.IsSuccess())
	{
		m_pBannerList = NULL;		// Banner 리스트 Pointer

		m_bIsBanner = false;

		// MessageBox
		unicode::t_char szText[MAX_TEXT_LENGTH] = {'\0', };
		// "배너 다운로드 실패. Version %d.%d.%d %s"
		sprintf(szText, GlobalText[3030], 
					m_BannerVerInfo.Zone, m_BannerVerInfo.year, m_BannerVerInfo.yearId, res.GetErrorMessage());
		CMsgBoxIGSCommon* pMsgBox = NULL;
		CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
		pMsgBox->Initialize(GlobalText[3028], szText);		// "에러"
		
		return false;
	}

#ifndef KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG		// #ifndef
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD		
	m_CurrentBannerVerInfo = m_BannerVerInfo;
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
#endif // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG

#ifdef KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG2
	#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD	
	m_CurrentBannerVerInfo = m_BannerVerInfo;
	#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
#endif // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG2

	m_pBannerList = m_BannerManager.GetListPtr();

	m_pBannerList->SetFirst();
	if( m_pBannerList->GetNext(m_BannerInfo) == false )
		return false;

	m_bIsBanner = true;

	return true;
}

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
bool CInGameShopSystem::IsScriptDownload()
{
#ifdef FOR_WORK
	DebugAngel_Write("InGameShopStatue.Txt", "CallStack - CInGameShopSystem::IsScriptDownload()\r\n");
	DebugAngel_Write("InGameShopStatue.Txt", " - Script Ver %d.%d.%d\r\n", m_ScriptVerInfo.Zone, m_ScriptVerInfo.year, m_ScriptVerInfo.yearId);
	DebugAngel_Write("InGameShopStatue.Txt", " - Current Ver %d.%d.%d\r\n", m_CurrentScriptVerInfo.Zone, m_CurrentScriptVerInfo.year, m_CurrentScriptVerInfo.yearId);
#endif // FOR_WORK
	if( ((m_ScriptVerInfo.year == m_CurrentScriptVerInfo.year)
		&& (m_ScriptVerInfo.yearId == m_CurrentScriptVerInfo.yearId)
		&& (m_ScriptVerInfo.Zone == m_CurrentScriptVerInfo.Zone))
		&& (m_bFirstScriptDownloaded == true)
		)
		
	{
#ifdef FOR_WORK
		DebugAngel_Write("InGameShopStatue.Txt", " Return - false\r\n");
#endif // FOR_WORK
		return false;
	}

#ifdef FOR_WORK
	DebugAngel_Write("InGameShopStatue.Txt", " Return - true\r\n");
#endif // FOR_WORK
	return true;
}

bool CInGameShopSystem::IsBannerDownload()
{
	if( ((m_BannerVerInfo.year == m_CurrentBannerVerInfo.year)
		&& (m_BannerVerInfo.yearId == m_CurrentBannerVerInfo.yearId)
		&& (m_BannerVerInfo.Zone == m_CurrentBannerVerInfo.Zone))
#ifdef KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG
		&& (m_bFirstBannerDownloaded == true)
#endif // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG
		)
	{
		return false;
	}

	return true;
}
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD


////////////////////////////////////////////////////////////////////
// Zone, Category Select
////////////////////////////////////////////////////////////////////

//--------------------------------------------
// SelectZone
bool CInGameShopSystem::SelectZone(int iIndex)
{
	int iZoneSeqIndex = GetZoneSeqIndexByIndex(iIndex);
	if( (INGAMESHOP_ERROR_ZERO_SIZE == iZoneSeqIndex) 
		|| (INGAMESHOP_ERROR_INVALID_INDEX == iZoneSeqIndex))
	{
		// error 처리
		return false;
	}

	if( m_pCategoryList->GetValueByKey(iZoneSeqIndex, m_SelectedZone) )
	{		
		SetCategoryName();

		return true;			// 성공
	}

	return false;
}

//--------------------------------------------
// SelectCategory
bool CInGameShopSystem::SelectCategory(int iIndex)
{
	m_listDisplayPackage.clear();

	// CategoryList Get
	int iCategorySeqIndex = GetCategorySeqIndexByIndex(iIndex);
	if( (INGAMESHOP_ERROR_ZERO_SIZE == iCategorySeqIndex) 
		|| (INGAMESHOP_ERROR_INVALID_INDEX == iCategorySeqIndex))
	{
		// error 처리
		return false;
	}

	if( m_pCategoryList->GetValueByKey(iCategorySeqIndex, m_SelectedCategory) )
	{
		// 이벤트존 이면 무조건 1번째 Catergory Select
		// 이벤트 조건 : Zone과 Zone에 속해있는 한개의 Category도 이벤트이어야 함.
		if( m_SelectedZone.EventFlag == 199 && m_SelectedCategory.EventFlag == 199)		// 199 : 이벤트, 200 : 일반
		{
			m_bSelectEventCategory = true;
			m_plistSelectPackage = &m_listEventPackage;

			SendRequestIGS_EventItemList(m_SelectedCategory.ProductDisplaySeq);
			m_bIsRequestEventPackage = true;
		}
		else
		{
			m_bSelectEventCategory = false;
			m_plistSelectPackage = &m_listNormalPackage;
			SetNormalPackage();
		}
	
		return true;			// 성공
	}

	return false;
}


////////////////////////////////////////////////////////////////////
// ItemList Page관리
////////////////////////////////////////////////////////////////////

//--------------------------------------------
// 처음페이지로 Setting
void CInGameShopSystem::BeginPage()
{
	m_iSelectedPage = 1;

	// Page당 Packege 정보를 갱신한다.
	InitPackagePerPage(m_iSelectedPage);
}

//--------------------------------------------
// 다음페이지로 Setting
void CInGameShopSystem::NextPage()
{
	if( GetTotalPages() > m_iSelectedPage )
	{
		m_iSelectedPage++;

		// Page당 Packege 정보를 갱신한다.
		InitPackagePerPage(m_iSelectedPage);
	}
}

//--------------------------------------------
// 이전페이지로 Setting
void CInGameShopSystem::PrePage()
{
	if( m_iSelectedPage > 1 )
	{
		m_iSelectedPage--;

		// Page당 Packege 정보를 갱신한다.	
		InitPackagePerPage(m_iSelectedPage);
	}
}

//--------------------------------------------
// 전체 페이지갯수를 가저온다.
int CInGameShopSystem::GetTotalPages()
{
	return (m_plistSelectPackage->size()/INGAMESHOP_DISPLAY_ITEMLIST_SIZE)+1;
}

//--------------------------------------------
// 전체 페이지번호를 가저온다.
int CInGameShopSystem::GetSelectPage()
{
	return m_iSelectedPage;
}

//--------------------------------------------
// 일반 패키지 셋팅
void CInGameShopSystem::SetNormalPackage()
{
	CShopPackage Package;
	int iPackageSeqIndex;

	m_listNormalPackage.clear();
	
	m_SelectedCategory.SetPackagSeqFirst();

	while(m_SelectedCategory.GetPackagSeqNext(iPackageSeqIndex))
	{
		if( !m_pPackageList->GetValueByKey(iPackageSeqIndex, Package))
			break;
		
		m_listNormalPackage.push_back(Package);
	}

	// 페이지 초기화
	BeginPage();
}


//--------------------------------------------
// 이벤트 패키지 셋팅
/* 사용안함
void CInGameShopSystem::SetEventPackage()
{
	type_listPackage::iterator iterEventPackage = m_listEventPackage.begin();

	int iNumDisplayPackage = min(m_listEventPackage.size(), INGAMESHOP_DISPLAY_ITEMLIST_SIZE);

	for( int i=0 ; i<iNumDisplayPackage ; i++)
	{
		m_listDisplayPackage.push_back((*iterEventPackage));
		iterEventPackage++;
	}

	// 페이지 초기화
	BeginPage();
}
*/

//--------------------------------------------
// 이벤트 패키지 초기화
void CInGameShopSystem::InitEventPackage(int iTotalEventPackage)
{
	m_listEventPackage.clear();
	m_listDisplayPackage.clear();
	m_iTotalEventPackage = iTotalEventPackage;
	m_iEventPackageCnt = 0;
#ifdef KJH_FIX_SHOP_EVENT_CATEGORY_PAGE
	m_iCurrentEventPackage = 0;
#else // KJH_FIX_SHOP_EVENT_CATEGORY_PAGE
#ifdef KJH_MOD_INGAMESHOP_PATCH_091028
	m_iTotalEventPackageFindFault = 0;
#endif // KJH_MOD_INGAMESHOP_PATCH_091028
#endif // KJH_FIX_SHOP_EVENT_CATEGORY_PAGE

	if( m_iTotalEventPackage < 1 )
	{
		m_bIsRequestEventPackage = false;
	}
}

//--------------------------------------------
// 이벤트 패키지 Insert
#ifdef KJH_FIX_SHOP_EVENT_CATEGORY_PAGE
void CInGameShopSystem::InsertEventPackage(int* pPackageSeq)
{
	m_SelectedCategory.SetPackagSeqFirst();
	
	CShopPackage Package;
	
	for( int i=0 ; i<INGAMESHOP_DISPLAY_ITEMLIST_SIZE ; i++)
	{
		if( m_pPackageList->GetValueByKey(pPackageSeq[i], Package))
		{
			m_listEventPackage.push_back(Package);			
		}
		
		m_iCurrentEventPackage++;

		// 패키지리스트를 다 받으면 페이지 초기화
		if( m_iTotalEventPackage == m_iCurrentEventPackage )
		{
			// 페이지 초기화
			BeginPage();
			m_bIsRequestEventPackage = false;
			return;
		}
	}
}
#else // KJH_FIX_SHOP_EVENT_CATEGORY_PAGE
void CInGameShopSystem::InsertEventPackage(int iPackageSeq)
{
	m_SelectedCategory.SetPackagSeqFirst();
	
	CShopPackage Package;

#ifdef KJH_MOD_INGAMESHOP_PATCH_091028
	if( m_pPackageList->GetValueByKey(iPackageSeq, Package))
	{
		m_listEventPackage.push_back(Package);			
	}
	else
	{
		m_iTotalEventPackageFindFault++;
	}
	
	// 패키지리스트를 다 받으면 페이지 초기화
	if( m_iTotalEventPackage == (int)m_listEventPackage.size() + m_iTotalEventPackageFindFault)
	{
		// 페이지 초기화
		BeginPage();
		m_bIsRequestEventPackage = false;
	}
#else // KJH_MOD_INGAMESHOP_PATCH_091028
	if( !m_pPackageList->GetValueByKey(iPackageSeq, Package))
		return;

	m_listEventPackage.push_back(Package);
	
	// 패키지리스트를 다 받으면 페이지 초기화
	if( m_iTotalEventPackage == (int)m_listEventPackage.size() )
	{
		// 페이지 초기화
		BeginPage();
		m_bIsRequestEventPackage = false;
	}
#endif // KJH_MOD_INGAMESHOP_PATCH_091028
}
#endif // KJH_FIX_SHOP_EVENT_CATEGORY_PAGE

////////////////////////////////////////////////////////////////////
// Zone, Category, Package 갯수 Get
////////////////////////////////////////////////////////////////////

//--------------------------------------------
// Zone의 갯수를 가져온다.
int CInGameShopSystem::GetSizeZones()
{
	return m_mapZoneSeqIndex.size();
}

//--------------------------------------------
// Category의 갯수를 가져온다. (현재 선택되어져있는 Zone)
int CInGameShopSystem::GetSizeCategoriesAsSelectedZone()
{
	return m_SelectedZone.CategoryList.size();
}

//--------------------------------------------
// Package의 갯수를 가져온다. (현재 선택되어져있는 Category)
int CInGameShopSystem::GetSizePackageAsSelectedCategory()
{
// 	if( m_bSelectEventCategory == true )
// 	{
// 		return m_iTotalEventPackage;
// 	}
// 
// 	return m_SelectedCategory.PackageList.size();
	return m_plistSelectPackage->size();
}

//--------------------------------------------
// 현재 Display되는 Package의 갯수를 가져온다.
int CInGameShopSystem::GetSizePackageAsDisplayPackage()
{
	return m_listDisplayPackage.size();
}

//--------------------------------------------
// Zone의 이름을 가져온다.
type_listName CInGameShopSystem::GetZoneName()
{
	return m_listZoneName;
}

//--------------------------------------------
// Category의 이름을 가져온다.
type_listName CInGameShopSystem::GetCategoryName()
{
	return m_listCategoryName;
}

////////////////////////////////////////////////////////////////////
// Package의 정보관리
////////////////////////////////////////////////////////////////////

//--------------------------------------------
// Package의 ItemCode을 Get
WORD CInGameShopSystem::GetPackageItemCode(int iIndex)
{
	type_listPackage::iterator iterPackage = m_listDisplayPackage.begin();

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	for(int i = 0; i < (int)m_listDisplayPackage.size(); i++)
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	for(int i=0 ; i<m_listDisplayPackage.size() ; i++ )
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	{
		if( iterPackage == m_listDisplayPackage.end() )
#ifdef KJH_MOD_INGAMESHOP_PATCH_091028
			return -1;
#else // KJH_MOD_INGAMESHOP_PATCH_091028
			return 0;
#endif // KJH_MOD_INGAMESHOP_PATCH_091028

		if( i == iIndex )
			break;

		iterPackage++;
	}

	return atoi((*iterPackage).InGamePackageID);
}

////////////////////////////////////////////////////////////////////
// Cash, Point, Mileage 관리
////////////////////////////////////////////////////////////////////

//--------------------------------------------
// Cash Set
void CInGameShopSystem::SetTotalCash(double dTotalCash)
{
	m_dTotalCash = dTotalCash;
}

//--------------------------------------------
// Point Set
void CInGameShopSystem::SetTotalPoint(double dTotalPoint)
{
	m_dTotalPoint = dTotalPoint;
}

//--------------------------------------------
// Mileage Set
void CInGameShopSystem::SetTotalMileage(double dTotalMileage)
{
	m_dTotalMileage = dTotalMileage;
}

#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
//--------------------------------------------
// CashCreditCard Set
void CInGameShopSystem::SetCashCreditCard(double dCashCreditCard)
{
	m_dCashCreditCard = dCashCreditCard;
}

//--------------------------------------------
// CashPrepaid Set
void CInGameShopSystem::SetCashPrepaid(double dCashPrepaid)
{
	m_dCashPrepaid = dCashPrepaid;
}
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL

//--------------------------------------------
// Cash Get
double CInGameShopSystem::GetTotalCash()
{
	return m_dTotalCash;
}

//--------------------------------------------
// Point Get
double CInGameShopSystem::GetTotalPoint()
{
	return m_dTotalPoint;
}

//--------------------------------------------
// Mileage Set
double CInGameShopSystem::GetTotalMileage()
{
	return m_dTotalMileage;
}


#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
//--------------------------------------------
// CashCredit Get
double CInGameShopSystem::GetCashCreditCard()
{
	return m_dCashCreditCard;
}

//--------------------------------------------
// CashPrepaid Get
double CInGameShopSystem::GetCashPrepaid()
{
	return m_dCashPrepaid;
}
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL

////////////////////////////////////////////////////////////////////
// Display Package관리
////////////////////////////////////////////////////////////////////

//--------------------------------------------
// Display되어있는 Packaged의 Pointer를 Get
CShopPackage* CInGameShopSystem::GetDisplayPackage(int iIndex)
{
	type_listPackage::iterator iterPackage = m_listDisplayPackage.begin();

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	for(int i = 0; i < (int)m_listDisplayPackage.size(); i++)
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	for(int i=0 ; i<m_listDisplayPackage.size() ; i++ )
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	{
		if( iterPackage == m_listDisplayPackage.end() )
			return NULL;
		
		if( i == iIndex )
			break;
		
		iterPackage++;
	}
	
	return &(*iterPackage);
}


////////////////////////////////////////////////////////////////////
// 샵을 Open했었는지 여부
////////////////////////////////////////////////////////////////////

//--------------------------------------------
// 샵을 Open했었는지 여부 Set
void CInGameShopSystem::SetIsRequestShopOpenning(bool IsRequestShopOpenning)
{
	m_bIsRequestShopOpenning = IsRequestShopOpenning;
}


//--------------------------------------------
// 샵을 Open했었는지 여부 Get
bool CInGameShopSystem::GetIsRequestShopOpenning()
{
	return m_bIsRequestShopOpenning;
}

#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT			// #ifndef
//--------------------------------------------
// SetStorageItemCnt - 보관함 전체 ItemCnt Set
void CInGameShopSystem::SetStorageItemCnt(int iItemCnt)
{
	m_iStorageItemCnt = iItemCnt;
}

//--------------------------------------------
// GetStorageItemCnt - 보관함 전체 ItemCnt Get
int CInGameShopSystem::GetStorageItemCnt()
{
	return m_iStorageItemCnt;
}
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT

//--------------------------------------------
// Package 정보 Get
bool CInGameShopSystem::GetPackageInfo(int iPackageSeq, int iPackageAttrType, OUT int& iValue, OUT unicode::t_char* pszText)
{
	CShopPackage Package;
	
	if( m_pPackageList->GetValueByKey(iPackageSeq, Package) == true )
	{
		switch(iPackageAttrType)
		{
		case IGS_PACKAGE_ATT_TYPE_NAME:
			{
				iValue = 0;
				strcpy(pszText, Package.PackageProductName);
				return true;
			}break;
		case IGS_PACKAGE_ATT_TYPE_DESCRIPTION:
			{
				iValue = 0;
				strcpy(pszText, Package.Description);
				return true;
			}break;
		case IGS_PACKAGE_ATT_TYPE_PRICE:
			{
				unicode::t_char szText[MAX_TEXT_LENGTH] = {'\0', };
				iValue = Package.Price;
				ConvertGold(iValue, szText);
				sprintf(pszText, "%s %s", szText, Package.PricUnitName);
				return true;
			}break;
		case IGS_PACKAGE_ATT_TYPE_ITEMCODE:
			{
				iValue = atoi(Package.InGamePackageID);
				pszText[0] = '\0';
				return true;
			}break;
		default:
			{
				iValue = 0;
				pszText[0] = '\0';
			}break;
		}
	}

	return false;
}
//--------------------------------------------
// 선택 상품일때 PriceSeq로 속성정보를 받아온다.
bool CInGameShopSystem::GetProductInfoFromPriceSeq(int iProductSeq, int iPriceSeq, int iAttrType, OUT int& iValue, OUT unicode::t_char* pszUnitName)
{
	CShopProduct Product;
	
	m_pProductList->SetPriceSeqFirst(iProductSeq, iPriceSeq);
	
	// 해당하는 속성을 가져온다.
	while(m_pProductList->GetPriceSeqNext(Product))
	{
		if( GetProductInfo(&Product, iAttrType, iValue, pszUnitName) == true )
		{
			// 성공
			return true;
		}
	}
	
	iValue = -1;
	pszUnitName[0] = '\0';	

	return false;
}

//--------------------------------------------
// 선택 상품일때 ProductSeq로 속성정보를 받아온다.
bool CInGameShopSystem::GetProductInfoFromProductSeq(int iProductSeq, int iAttrType, OUT int& iValue, OUT unicode::t_char* pszUnitName)
{
	CShopProduct Product;
	
	m_pProductList->SetProductSeqFirst(iProductSeq);
	
	// 해당하는 속성을 가져온다.
	while(m_pProductList->GetProductSeqNext(Product))
	{
		if( GetProductInfo(&Product, iAttrType, iValue, pszUnitName) == true )
		{
			// 성공
			return true;
		}
	}
	
	iValue = -1;
	pszUnitName[0] = '\0';	

	return false;
}

//--------------------------------------------
// 선택 상품일때 Product로 속성정보를 받아온다.
bool CInGameShopSystem::GetProductInfo(CShopProduct* pProduct, int iAttrType, OUT int& iValue, OUT unicode::t_char* pszUnitName)
{
	// 속성 타입에 따른 Output
	switch(iAttrType)
	{
	case IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD:		// 사용가능 기간
		{
			if( (pProduct->PropertySeq == 2) || (pProduct->PropertySeq == 28) || (pProduct->PropertySeq == 12)
				|| (pProduct->PropertySeq == 58) || (pProduct->PropertySeq == 10) )
			{
#ifdef KJH_MOD_INGAMESHOP_UNITTTYPE_FILED_OF_PRODUCT_SCRIPT
				iValue = atoi(pProduct->Value);
				switch( pProduct->UnitType)
				{
				case 386:		// 초
					{		
						if( iValue >= 86400 )
						{
							iValue /= 86400;
							strcpy(pszUnitName, GlobalText[2298]);		// 일
						}
						else if( iValue >= 3600 )
						{
							iValue /= 3600;
							strcpy(pszUnitName, GlobalText[2299]);		// 시간
						}
						else if( iValue >= 60)
						{
							iValue /= 60;
							strcpy(pszUnitName, GlobalText[2300]);		// 분
						}
						else 
						{
							strcpy(pszUnitName, GlobalText[2301]);		// 초
						}
					}break;		
				case 174:		// 분
					{
						if( iValue >= 1440 )
						{
							iValue /= 1440;
							strcpy(pszUnitName, GlobalText[2298]);		// 일
						}
						else if( iValue >= 60 )
						{
							iValue /= 60;
							strcpy(pszUnitName, GlobalText[2299]);		// 시간
						}
						else
						{
							strcpy(pszUnitName, GlobalText[2300]);		// 분
						}
					}break;
				case 172:		// 시간
					{
						if( iValue >= 24 )
						{
							iValue /= 24;
							strcpy(pszUnitName, GlobalText[2298]);		// 일
						}
						else
						{
							strcpy(pszUnitName, GlobalText[2299]);		// 시간
						}
					}break;
				default:		// 기타
					{
						strcpy(pszUnitName, pProduct->UnitName);		// 스크립트의 단위(UnitName) 저장
					}break;
				}		
#else // KJH_MOD_INGAMESHOP_UNITTTYPE_FILED_OF_PRODUCT_SCRIPT
				iValue = atoi(pProduct->Value)/60;		// 초단위를 분단위로 바꿈
				if( iValue >= 1440 )
				{
					iValue /= 1440;
					strcpy(pszUnitName, GlobalText[2298]);		// 일
				}
				else if( iValue >= 60 )
				{
					iValue /= 60;
					strcpy(pszUnitName, GlobalText[2299]);		// 시간
				}
				else
				{
					strcpy(pszUnitName, GlobalText[2300]);		// 분
				}
#endif // KJH_MOD_INGAMESHOP_UNITTTYPE_FILED_OF_PRODUCT_SCRIPT

				return true;
			}
		}break;
	case IGS_PRODUCT_ATT_TYPE_AVALIABLE_PERIOD:		// 유효기간
		{
			if( (pProduct->PropertySeq == 46) || (pProduct->PropertySeq == 49) || (pProduct->PropertySeq == 48)
				|| (pProduct->PropertySeq == 51) || (pProduct->PropertySeq == 52) || (pProduct->PropertySeq == 53) 
				|| (pProduct->PropertySeq == 50) || (pProduct->PropertySeq == 60) )
			{
				iValue = atoi(pProduct->Value);
				strcpy(pszUnitName, pProduct->UnitName);
				return true;
			}
		}break;
	case IGS_PRODUCT_ATT_TYPE_NUM:			// 갯수
		{
			if( (pProduct->PropertySeq == 30) || (pProduct->PropertySeq == 11) || (pProduct->PropertySeq == 7)
				|| (pProduct->PropertySeq == 8) || (pProduct->PropertySeq == 9) || (pProduct->PropertySeq == 31) )
			{
				iValue = atoi(pProduct->Value);
				strcpy(pszUnitName, pProduct->UnitName);
				return true;
			}
		}break;
	case IGS_PRODUCT_ATT_TYPE_PRICE:		// 가격
		{
			iValue = pProduct->Price;
			ConvertGold(pProduct->Price, pszUnitName);
			return true;
		}break;
	case IGS_PRODUCT_ATT_TYPE_ITEMCODE:		// 아이템 코드
		{
			iValue = atoi(pProduct->InGamePackageID);
			pszUnitName[0] = '\0';
			return true;
		}break;
	case IGS_PRODUCT_ATT_TYPE_ITEMNAME:
		{
			iValue = -1;
			strcpy(pszUnitName, pProduct->ProductName);
			return true;
		}break;
	case IGS_PRODUCT_ATT_TYPE_PRICE_SEQUENCE:	// PriceSeq
		{
			iValue = pProduct->PriceSeq;
			pszUnitName[0] = '\0';
			return true;
		}break;
	default:						// 해당하지 않는 속성 코드
		{
			iValue = -1;
			pszUnitName[0] = '\0';	
		}break;
	}

	return false;
}

//--------------------------------------------
// 이벤트 패키지를 요청중이면 버튼 클릭 false
bool CInGameShopSystem::IsRequestEventPackge()
{
	if( m_bIsRequestEventPackage == true )
		return false;

	return true;
}

void CInGameShopSystem::SetRequestEventPackge()
{
	m_bIsRequestEventPackage = false;
}

//--------------------------------------------
// IsShopOpen
bool CInGameShopSystem::IsShopOpen()
{
	return m_bIsShopOpenLock ? false : true;
}

//--------------------------------------------
// 배너 로딩 유무
bool CInGameShopSystem::IsBanner()
{
	return m_bIsBanner;
}

//--------------------------------------------
// 배너 파일이름 Get (FullPath)
unicode::t_char* CInGameShopSystem::GetBannerFileName()
{
	if( m_bIsBanner == false )
		return NULL;

	return m_BannerInfo.BannerImagePath;
}

//--------------------------------------------
// 배너 URL Get
unicode::t_char* CInGameShopSystem::GetBannerURL()
{
	if( m_bIsBanner == false )
		return NULL;
	
	return m_BannerInfo.BannerLinkURL;
}


////////////////////////////////////////////////////////////////////
// 내부함수들
////////////////////////////////////////////////////////////////////

//--------------------------------------------
// Zone 정보 Setting
void CInGameShopSystem::InitZoneInfo()
{
	m_mapZoneSeqIndex.clear();
	m_listZoneName.clear();

	m_pCategoryList->SetFirst();
	CShopCategory Zone;
	
	int i=0;
	while(m_pCategoryList->GetNext(Zone))
	{
		if(1 == Zone.Root)
		{
			m_mapZoneSeqIndex.insert(type_mapZoneSeq::value_type(i++, Zone.ProductDisplaySeq));
			m_listZoneName.push_back(Zone.CategroyName);
		}
	}
}

//--------------------------------------------
// 페이지당 Package Setting
void CInGameShopSystem::InitPackagePerPage(int iPageIndex)
{
	m_listDisplayPackage.clear();

	type_listPackage::iterator	iterlistPackage;


	iterlistPackage = m_plistSelectPackage->begin();

	// 패이지만큼 iter++
	int iBeginDisplayItemIndex = INGAMESHOP_DISPLAY_ITEMLIST_SIZE*(iPageIndex-1);
	for(int i=0 ; i<iBeginDisplayItemIndex ; i++)
	{
		iterlistPackage++;
	}
	
	for(int j=0 ; j<INGAMESHOP_DISPLAY_ITEMLIST_SIZE ; j++)
	{
		if( iterlistPackage == m_plistSelectPackage->end())
			break;

		m_listDisplayPackage.push_back(*iterlistPackage);
		iterlistPackage++;
	}
}

//--------------------------------------------
// Index로 Zone SeqIndex를 Get
int CInGameShopSystem::GetZoneSeqIndexByIndex(int iIndex)
{
	if( GetSizeZones() <= 0 )
		return INGAMESHOP_ERROR_ZERO_SIZE;

	type_mapZoneSeq::iterator iterZoneSeqIndex = m_mapZoneSeqIndex.find(iIndex);

	if( iterZoneSeqIndex == m_mapZoneSeqIndex.end() )
		return INGAMESHOP_ERROR_INVALID_INDEX;

	return (int)iterZoneSeqIndex->second;
}

//--------------------------------------------
// Index로 Category SeqIndex를 Get
int CInGameShopSystem::GetCategorySeqIndexByIndex(int iIndex)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int iCategorySeqIndex = 0;
	bool bRes = false;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int iCategorySeqIndex;
	bool bRes;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	if( GetSizeCategoriesAsSelectedZone() <= 0 )
		return INGAMESHOP_ERROR_ZERO_SIZE;

	m_SelectedZone.SetCategoryFirst();
	for(int i=0 ; i<=iIndex ; i++ )
	{
		bRes = m_SelectedZone.GetCategoryNext(iCategorySeqIndex);
	}

	if( bRes == false )
		return INGAMESHOP_ERROR_INVALID_INDEX;

	return iCategorySeqIndex;
}

//--------------------------------------------
// Category Name Set
void CInGameShopSystem::SetCategoryName()
{
	m_listCategoryName.clear();

	int iCategorySeqIndex;
	CShopCategory Category;
	m_SelectedZone.SetCategoryFirst();

	while( m_SelectedZone.GetCategoryNext(iCategorySeqIndex) )
	{
		m_pCategoryList->GetValueByKey(iCategorySeqIndex, Category);
		m_listCategoryName.push_back(Category.CategroyName);
	}
}

//--------------------------------------------
// SetShopOpenLock
void CInGameShopSystem::ShopOpenLock()
{
	m_bIsShopOpenLock = true;
}

//--------------------------------------------
// SetShopOpenUnLock
void CInGameShopSystem::ShopOpenUnLock()
{
	m_bIsShopOpenLock = false;
}

CListVersionInfo CInGameShopSystem::GetScriptVer()
{
	return m_ScriptVerInfo;
}

CListVersionInfo CInGameShopSystem::GetBannerVer()
{
	return m_BannerVerInfo;
}

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
CListVersionInfo CInGameShopSystem::GetCurrentScriptVer()
{
	return m_CurrentScriptVerInfo;
}

CListVersionInfo CInGameShopSystem::GetCurrentBannerVer()
{
	return m_CurrentBannerVerInfo;
}
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

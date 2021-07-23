// InGameShopSystem.h: interface for the InGameShopSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INGAMESHOPSYSTEM_H__2DF68839_DA28_44BC_B662_213BB22839CB__INCLUDED_)
#define AFX_INGAMESHOPSYSTEM_H__2DF68839_DA28_44BC_B662_213BB22839CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#ifdef KJH_MOD_LIBRARY_LINK_EACH_NATION
#include <ShopListManager.h>
#include <BannerListManager.h>
#else // KJH_MOD_LIBRARY_LINK_EACH_NATION
#include "./InGameShopSystem/ShopListManager/ShopListManager.h"				
#include "./InGameShopSystem/ShopListManager/BannerListManager.h"
#endif // KJH_MOD_LIBRARY_LINK_EACH_NATION


/* 
	// < Sequence 와 Index의 구분 >
	// Sequence : Sciprt상의 Category, Package, Product의 고유번호
	// Index : UI상의 넘버링
*/

#define INGAMESHOP_ERROR_ZERO_SIZE		(-1)
#define INGAMESHOP_ERROR_INVALID_INDEX	(-2)

//#define INGAMESHOP_DISPLAY_ITEMLIST_SIZE		(9)		_define.h 로 옮김

//typedef std::map<int, CShopPackage>		type_mapPackage;
typedef std::list<CShopPackage>			type_listPackage;
typedef std::map<int, int>				type_mapZoneSeq;
typedef std::list<unicode::t_string>	type_listName;

class CInGameShopSystem
{
public:
#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT				// #ifndef
	enum LISTBOX_INDEX
	{
		IGS_SAFEKEEPING_LISTBOX = 0,
		IGS_PRESENTBOX_LISTBOX,
		IGS_TOTAL_LISTBOX,
	};
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT

	// 패키지 상품 종류
	enum IGS_PACKAGE_GOODS_TYPE
	{
		IGS_GOODS_TYPE_FIXEDAMOUNT		= 135,		// 정액상품
		IGS_GOODS_TYPE_FIXEDQUANTITY	= 136,		// 정량상품
		IGS_GOODS_TYPE_PREMIUM			= 137,		// 프리미엄상품
		IGS_GOODS_TYPE_CONSUMPTION		= 138,		// 소모성아이템
		IGS_GOODS_TYPE_ETERNITY			= 139,		// 영구성아이템
		IGS_GOODS_TYPE_PERIOD			= 140,		// 기간제아이템
		IGS_GOODS_TYPE_PREMIUM_ITEM		= 406,		// 프리미엄아이템
		IGS_GOODS_TYPE_GOBLIN_POINT		= 515,		// 고블린포인트	
	};

	enum IGS_PACKAGE_ATTRIBUTE_TYPE
	{
		IGS_PACKAGE_ATT_TYPE_NONE	= 0,
		IGS_PACKAGE_ATT_TYPE_NAME,			// Name
		IGS_PACKAGE_ATT_TYPE_DESCRIPTION,	// Description (상세설명)
		IGS_PACKAGE_ATT_TYPE_PRICE,			// 가격
		IGS_PACKAGE_ATT_TYPE_ITEMCODE,			// ItemCode
	};

	enum IGS_PRODUCT_ATTRIBUTE_TYPE
	{
		IGS_PRODUCT_ATT_TYPE_NONE	= 0,
		IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD,	// 사용기간
		IGS_PRODUCT_ATT_TYPE_AVALIABLE_PERIOD,	// 유효기간
		IGS_PRODUCT_ATT_TYPE_NUM,				// 갯수
		IGS_PRODUCT_ATT_TYPE_PRICE,				// 아이템 가격
		IGS_PRODUCT_ATT_TYPE_ITEMCODE,			// 아이템코드
		IGS_PRODUCT_ATT_TYPE_ITEMNAME,			// 아이템 이름
		IGS_PRODUCT_ATT_TYPE_PRICE_SEQUENCE,	// Price Seq
	};

	enum IGS_ETC
	{
		IGS_LIMIT_REQUEST_EVENT_PACKAGE	= 20,	// 이벤트 패키지 요청을 제한 (이벤트존 20번 클릭)
	};

protected:
	CInGameShopSystem();
public:
	~CInGameShopSystem();

public:
	static CInGameShopSystem* GetInstance();

 	void Initalize();
// 	bool Update();
// 	bool Render();
 	void Release();

	void SetScriptVersion(int iSalesZone, int iYear, int iYearId);
	void SetBannerVersion(int iSalesZone, int iYear, int iYearId);
	bool ScriptDownload();		// 인게임샵 스크립트 다운로드
	bool BannerDownload();		// 베너 스크립트 다운로드

public:
	// 현재 Zone, Category Setting
	bool SelectZone(int iIndex);
	bool SelectCategory(int iIndex);

	// ItemList Page관리
	void BeginPage();
	void NextPage();
	void PrePage();
	int GetTotalPages();
	int GetSelectPage();

	// Zone, Category, Package 갯수 Get
	int GetSizeZones();
	int GetSizeCategoriesAsSelectedZone();
	int GetSizePackageAsSelectedCategory();
	int GetSizePackageAsDisplayPackage();

	// Package의 정보관리
	WORD GetPackageItemCode(int iIndex);

	// Zone, Category Name Get/Set
	type_listName GetZoneName();			// Zone Name Get
	type_listName GetCategoryName();		// Category Name Get

	// Cash, Point, Mileage
	void SetTotalCash(double dTotalCash);
	void SetTotalPoint(double dTotalPoint);
	void SetTotalMileage(double dTotalMileage);
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	void SetCashCreditCard(double dCashCreditCard);		// Global Credit Cash
	void SetCashPrepaid(double dCashPrepaid);			// Global Prepaid Cash
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	double GetTotalCash();
	double GetTotalPoint();
	double GetTotalMileage();
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	double GetCashCreditCard();							// Global Credit Cash
	double GetCashPrepaid();							// Global Prepaid Cash
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	
	// Display되어있는 Packaged의 Pointer를 Get
	CShopPackage* GetDisplayPackage(int iIndex);		

	// Product 정보관리
	//void SetProductSeqFirst(int iProductSeq);
	//bool GetProductSeqNext(CShopProduct& product);

	// 샵 Open 요청중인지 여부
	void SetIsRequestShopOpenning(bool bIsRequestShopOpenning);
	bool GetIsRequestShopOpenning();

	// 보관함 관리
	//void SetStorageItemCnt(int iStorageIndex, int iItemCnt);
	//int	GetStorageItemCnt(int iStorageIndex);
#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT			// # ifndef
	void SetStorageItemCnt(int iItemCnt);
	int GetStorageItemCnt();
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT

	// Package의 정보 OUT
	bool GetPackageInfo(int iPackageSeq, int iPackageAttrType, OUT int& iValue, OUT unicode::t_char* pszText);

	// 선택 상품일때 PriceSeq로 속성정보를 받아온다.
	bool GetProductInfoFromPriceSeq(int iProductSeq, int iPriceSeq, int iAttrType, 
									OUT int& iValue, OUT unicode::t_char* pszUnitName);
	bool GetProductInfoFromProductSeq(int iProductSeq, int iAttrType, OUT int& iValue, OUT unicode::t_char* pszUnitName);

	// Normal Package관련
	void SetNormalPackage();
	//void SetEventPackage();

	// Event Package관련
	void InitEventPackage(int iTotalEventPackage);
#ifdef KJH_FIX_SHOP_EVENT_CATEGORY_PAGE
	void InsertEventPackage(int* pPackageSeq);
#else // KJH_FIX_SHOP_EVENT_CATEGORY_PAGE
	void InsertEventPackage(int iPackageSeq);
#endif // KJH_FIX_SHOP_EVENT_CATEGORY_PAGE

	// Shop Open 관련
	bool IsShopOpen();
	bool IsRequestEventPackge();
	void SetRequestEventPackge();

	// Banner 관련
	bool IsBanner();
	unicode::t_char* GetBannerFileName();
	unicode::t_char* GetBannerURL();

	// 스크립트 버젼 관련
	CListVersionInfo GetScriptVer();
	CListVersionInfo GetBannerVer();
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	CListVersionInfo GetCurrentScriptVer();
	CListVersionInfo GetCurrentBannerVer();
	bool IsScriptDownload();			// 스크립트를 다운로드 할 수 있느냐
	bool IsBannerDownload();			// 배너를 다운로드 할 수 있느냐

	// Shop Open 관련
	void ShopOpenLock();
	void ShopOpenUnLock();
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

	// 내부함수들
protected:
	void InitZoneInfo();							// Zone정보 저장
	void InitPackagePerPage(int iPageIndex);		// 현재 패이지당 패키지 Init

	// Zone, Category Seq Get
	int GetZoneSeqIndexByIndex(int iIndex);		// Zone SeqIndex Get
	int GetCategorySeqIndexByIndex(int iIndex);	// Category SeqIndex Get

	// Category Name Set
	void SetCategoryName();

#ifndef KJH_MOD_SHOP_SCRIPT_DOWNLOAD			// #ifndef  - 정리할 때 지워야 하는 소스
	// Shop Open 관련
	void ShopOpenLock();
	void ShopOpenUnLock();
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

	bool GetProductInfo(CShopProduct* pProduct, int iAttrType, OUT int& iValue, OUT unicode::t_char* pszUnitName);

protected:
	CShopListManager		m_ShopManager;			// 인게임샵 매니져
	CBannerListManager		m_BannerManager;		// 배너 매니져

	CListVersionInfo		m_ScriptVerInfo;		// 다운받을 스크립트 버젼정보
	CListVersionInfo		m_BannerVerInfo;		// 다운받을 배너 버젼정보

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	CListVersionInfo		m_CurrentScriptVerInfo;		// 현재 스크립트 버젼정보
	CListVersionInfo		m_CurrentBannerVerInfo;		// 현재 배너 버젼정보
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	
#ifdef KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION
	char					m_szScriptIPAddress[20];			// 스크립트/배너 서버 IP주소
	char					m_szBannerIPAddress[20];	// 배너 서버 IP주소
#else // KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION
	char					m_szIPAddress[20];			// 스크립트/배너 서버 IP주소
#endif // KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION
	char					m_szScriptRemotePath[MAX_TEXT_LENGTH];	// 스크립트 파일 서버 Path 
	char					m_szScriptLocalPath[MAX_TEXT_LENGTH];	// 스크립트 파일 다운로드 Path
	char					m_szBannerRemotePath[MAX_TEXT_LENGTH];	// 배너 파일 서버 Path 
	char					m_szBannerLocalPath[MAX_TEXT_LENGTH];	// 배너 파일 다운로드 Path

	CShopCategory			m_SelectedZone;			// 현재 선택되어져있는 Zone - Category list가 있다.
 	CShopCategory			m_SelectedCategory;		// 현재 선택되어져있는 Category - 패키지들의 리스트가 있다.
	CBannerInfo				m_BannerInfo;			// 현재 배너 정보

 	int						m_iSelectedPage;		// 현재 페이지번호

	type_mapZoneSeq			m_mapZoneSeqIndex;		// Zone의 SeqIndex
	type_listPackage		m_listDisplayPackage;	// 현재 페이지의 패키지들
	type_listPackage		m_listNormalPackage;	// 일반 패키지들
	type_listPackage		m_listEventPackage;		// 이벤트 패키지들
	type_listPackage*		m_plistSelectPackage;		// 일반/이벤트 패키지
	type_listName			m_listZoneName;			// Zone의 Name
	type_listName			m_listCategoryName;		// 현재 선택되어져있는 Category의 Name
	
	CShopCategoryList		*m_pCategoryList;		// Category 리스트 Pointer
	CShopPackageList		*m_pPackageList;		// Packege 리스트 Pointer
	CShopProductList		*m_pProductList;		// Product 리스트 Pointer

	CBannerInfoList			*m_pBannerList;			// Banner 리스트 Pointer
	
	double					m_dTotalCash;			// 현재 캐시
	double					m_dTotalPoint;			// 현재 포인트  (사용안함)
	double					m_dTotalMileage;		// 현재 마일리지 (고블린 포인트)
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	double					m_dCashCreditCard;		// 현재 캐쉬:신용카드 글로벌 경우만
	double					m_dCashPrepaid;			// 현재 캐쉬:현금결제 글로벌 경우만
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL

	bool					m_bIsRequestEventPackage;		// 이벤트 패키지 정보를 요청중인지 여부

	bool					m_bIsRequestShopOpenning;		// 샵 Open 요청중인지 여부

	bool					m_bSelectEventCategory;		// 이벤트 카테고리를 선택했는지 여부
	bool					m_bAbleRequestEventPackage;	// 이벤트 패키지 요청 가능한지 여부
	int						m_iCntSelectEventZone;		// 이벤트 존 선택 카운트 (이벤트 패키지를 서버에 매번 요청하지 않기 위함)

#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT			// #ifndef
	int						m_iStorageItemCnt;		// 보관함의 Item의 갯수
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT

	int						m_iEventPackageCnt;		// 이벤트 Package의 갯수
	int						m_iTotalEventPackage;	// 서버로부터 다운받아야할 이벤트 Package의 갯수
#ifdef KJH_FIX_SHOP_EVENT_CATEGORY_PAGE
	int						m_iCurrentEventPackage;	// 서버로 부터 받은 Event Package
#else // KJH_FIX_SHOP_EVENT_CATEGORY_PAGE
#ifdef KJH_MOD_INGAMESHOP_PATCH_091028
	int						m_iTotalEventPackageFindFault;	// 서버로 부터 받은 Event Package Index가 스크립트에 없을때 갯수 누적
#endif // KJH_MOD_INGAMESHOP_PATCH_091028
#endif // KJH_FIX_SHOP_EVENT_CATEGORY_PAGE

	bool					m_bIsShopOpenLock;		// ShopOpen 잠그기(스크립트 로드 실패시)
	bool					m_bIsBanner;			// 배너 로딩 유무
#ifdef KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG
	bool					m_bFirstScriptDownloaded;	// 게임시작후 한번은 스크립트를 다운받아야 한다.
	bool					m_bFirstBannerDownloaded;
#endif // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG
};	

#define g_InGameShopSystem CInGameShopSystem::GetInstance()

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#endif // !defined(AFX_INGAMESHOPSYSTEM_H__2DF68839_DA28_44BC_B662_213BB22839CB__INCLUDED_)

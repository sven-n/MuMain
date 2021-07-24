//////////////////////////////////////////////////////////////////////
// InGameShopSystem.h: interface for the InGameShopSystem class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INGAMESHOPSYSTEM_H__2DF68839_DA28_44BC_B662_213BB22839CB__INCLUDED_)
#define AFX_INGAMESHOPSYSTEM_H__2DF68839_DA28_44BC_B662_213BB22839CB__INCLUDED_

#pragma once

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#ifdef KJH_MOD_LIBRARY_LINK_EACH_NATION
#include <ShopListManager.h>
#include <BannerListManager.h>
#else // KJH_MOD_LIBRARY_LINK_EACH_NATION
#include "./InGameShopSystem/ShopListManager/ShopListManager.h"				
#include "./InGameShopSystem/ShopListManager/BannerListManager.h"
#endif // KJH_MOD_LIBRARY_LINK_EACH_NATION


#define INGAMESHOP_ERROR_ZERO_SIZE		(-1)
#define INGAMESHOP_ERROR_INVALID_INDEX	(-2)

//#define INGAMESHOP_DISPLAY_ITEMLIST_SIZE		(9)		_define.h

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

	enum IGS_PACKAGE_GOODS_TYPE
	{
		IGS_GOODS_TYPE_FIXEDAMOUNT		= 135,
		IGS_GOODS_TYPE_FIXEDQUANTITY	= 136,
		IGS_GOODS_TYPE_PREMIUM			= 137,
		IGS_GOODS_TYPE_CONSUMPTION		= 138,
		IGS_GOODS_TYPE_ETERNITY			= 139,
		IGS_GOODS_TYPE_PERIOD			= 140,
		IGS_GOODS_TYPE_PREMIUM_ITEM		= 406,
		IGS_GOODS_TYPE_GOBLIN_POINT		= 515,	
	};

	enum IGS_PACKAGE_ATTRIBUTE_TYPE
	{
		IGS_PACKAGE_ATT_TYPE_NONE	= 0,
		IGS_PACKAGE_ATT_TYPE_NAME,			// Name
		IGS_PACKAGE_ATT_TYPE_DESCRIPTION,	// Description
		IGS_PACKAGE_ATT_TYPE_PRICE,	
		IGS_PACKAGE_ATT_TYPE_ITEMCODE,			// ItemCode
	};

	enum IGS_PRODUCT_ATTRIBUTE_TYPE
	{
		IGS_PRODUCT_ATT_TYPE_NONE	= 0,
		IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD,
		IGS_PRODUCT_ATT_TYPE_AVALIABLE_PERIOD,
		IGS_PRODUCT_ATT_TYPE_NUM,
		IGS_PRODUCT_ATT_TYPE_PRICE,
		IGS_PRODUCT_ATT_TYPE_ITEMCODE,
		IGS_PRODUCT_ATT_TYPE_ITEMNAME,
		IGS_PRODUCT_ATT_TYPE_PRICE_SEQUENCE,	// Price Seq
	};

	enum IGS_ETC
	{
		IGS_LIMIT_REQUEST_EVENT_PACKAGE	= 20,
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
	bool ScriptDownload();
	bool BannerDownload();

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

	// Zone, Category, Package
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
	
	// Display Package Pointer Get
	CShopPackage* GetDisplayPackage(int iIndex);		

	// Product
	//void SetProductSeqFirst(int iProductSeq);
	//bool GetProductSeqNext(CShopProduct& product);

	// Open
	void SetIsRequestShopOpenning(bool bIsRequestShopOpenning);
	bool GetIsRequestShopOpenning();

	//void SetStorageItemCnt(int iStorageIndex, int iItemCnt);
	//int	GetStorageItemCnt(int iStorageIndex);
#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT			// # ifndef
	void SetStorageItemCnt(int iItemCnt);
	int GetStorageItemCnt();
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT

	// Package의 정보 OUT
	bool GetPackageInfo(int iPackageSeq, int iPackageAttrType, OUT int& iValue, OUT unicode::t_char* pszText);

	bool GetProductInfoFromPriceSeq(int iProductSeq, int iPriceSeq, int iAttrType, 
									OUT int& iValue, OUT unicode::t_char* pszUnitName);
	bool GetProductInfoFromProductSeq(int iProductSeq, int iAttrType, OUT int& iValue, OUT unicode::t_char* pszUnitName);

	// Normal Package
	void SetNormalPackage();
	//void SetEventPackage();

	// Event Package
	void InitEventPackage(int iTotalEventPackage);
#ifdef KJH_FIX_SHOP_EVENT_CATEGORY_PAGE
	void InsertEventPackage(int* pPackageSeq);
#else // KJH_FIX_SHOP_EVENT_CATEGORY_PAGE
	void InsertEventPackage(int iPackageSeq);
#endif // KJH_FIX_SHOP_EVENT_CATEGORY_PAGE

	// Shop Open
	bool IsShopOpen();
	bool IsRequestEventPackge();
	void SetRequestEventPackge();

	// Banner
	bool IsBanner();
	unicode::t_char* GetBannerFileName();
	unicode::t_char* GetBannerURL();

	CListVersionInfo GetScriptVer();
	CListVersionInfo GetBannerVer();
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	CListVersionInfo GetCurrentScriptVer();
	CListVersionInfo GetCurrentBannerVer();
	bool IsScriptDownload();
	bool IsBannerDownload();

	// Shop Open
	void ShopOpenLock();
	void ShopOpenUnLock();
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

	// 내부함수들
protected:
	void InitZoneInfo();
	void InitPackagePerPage(int iPageIndex);

	// Zone, Category Seq Get
	int GetZoneSeqIndexByIndex(int iIndex);		// Zone SeqIndex Get
	int GetCategorySeqIndexByIndex(int iIndex);	// Category SeqIndex Get

	// Category Name Set
	void SetCategoryName();

#ifndef KJH_MOD_SHOP_SCRIPT_DOWNLOAD			// #ifndef
	// Shop Open
	void ShopOpenLock();
	void ShopOpenUnLock();
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

	bool GetProductInfo(CShopProduct* pProduct, int iAttrType, OUT int& iValue, OUT unicode::t_char* pszUnitName);

protected:
	CShopListManager		m_ShopManager;
	CBannerListManager		m_BannerManager;

	CListVersionInfo		m_ScriptVerInfo;
	CListVersionInfo		m_BannerVerInfo;

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	CListVersionInfo		m_CurrentScriptVerInfo;
	CListVersionInfo		m_CurrentBannerVerInfo;
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	
#ifdef KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION
	char					m_szScriptIPAddress[20];
	char					m_szBannerIPAddress[20];
#else // KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION
	char					m_szIPAddress[20];
#endif // KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION
	char					m_szScriptRemotePath[MAX_TEXT_LENGTH];
	char					m_szScriptLocalPath[MAX_TEXT_LENGTH];
	char					m_szBannerRemotePath[MAX_TEXT_LENGTH];
	char					m_szBannerLocalPath[MAX_TEXT_LENGTH];

	CShopCategory			m_SelectedZone;
 	CShopCategory			m_SelectedCategory;
	CBannerInfo				m_BannerInfo;

 	int						m_iSelectedPage;

	type_mapZoneSeq			m_mapZoneSeqIndex;
	type_listPackage		m_listDisplayPackage;
	type_listPackage		m_listNormalPackage;
	type_listPackage		m_listEventPackage;
	type_listPackage*		m_plistSelectPackage;
	type_listName			m_listZoneName;
	type_listName			m_listCategoryName;
	
	CShopCategoryList		*m_pCategoryList;
	CShopPackageList		*m_pPackageList;
	CShopProductList		*m_pProductList;

	CBannerInfoList			*m_pBannerList;
	
	double					m_dTotalCash;
	double					m_dTotalPoint;
	double					m_dTotalMileage;
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	double					m_dCashCreditCard;
	double					m_dCashPrepaid;
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL

	bool					m_bIsRequestEventPackage;

	bool					m_bIsRequestShopOpenning;

	bool					m_bSelectEventCategory;
	bool					m_bAbleRequestEventPackage;
	int						m_iCntSelectEventZone;

#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
	int						m_iStorageItemCnt;
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT

	int						m_iEventPackageCnt;
	int						m_iTotalEventPackage;
#ifdef KJH_FIX_SHOP_EVENT_CATEGORY_PAGE
	int						m_iCurrentEventPackage;
#else // KJH_FIX_SHOP_EVENT_CATEGORY_PAGE
#ifdef KJH_MOD_INGAMESHOP_PATCH_091028
	int						m_iTotalEventPackageFindFault;
#endif // KJH_MOD_INGAMESHOP_PATCH_091028
#endif // KJH_FIX_SHOP_EVENT_CATEGORY_PAGE

	bool					m_bIsShopOpenLock;
	bool					m_bIsBanner;
#ifdef KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG
	bool					m_bFirstScriptDownloaded;
	bool					m_bFirstBannerDownloaded;
#endif // KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG
};	

#define g_InGameShopSystem CInGameShopSystem::GetInstance()

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#endif // !defined(AFX_INGAMESHOPSYSTEM_H__2DF68839_DA28_44BC_B662_213BB22839CB__INCLUDED_)

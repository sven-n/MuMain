// InGameShopSystem.h: interface for the InGameShopSystem class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INGAMESHOPSYSTEM_H__2DF68839_DA28_44BC_B662_213BB22839CB__INCLUDED_)
#define AFX_INGAMESHOPSYSTEM_H__2DF68839_DA28_44BC_B662_213BB22839CB__INCLUDED_

#pragma once

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "./GameShop/ShopListManager/ShopListManager.h"
#include "./GameShop/ShopListManager/BannerListManager.h"

#define INGAMESHOP_ERROR_ZERO_SIZE		(-1)
#define INGAMESHOP_ERROR_INVALID_INDEX	(-2)

typedef std::list<CShopPackage>			type_listPackage;
typedef std::map<int, int>				type_mapZoneSeq;
typedef std::list<std::wstring>	type_listName;

class CInGameShopSystem
{
public:
    enum IGS_PACKAGE_GOODS_TYPE
    {
        IGS_GOODS_TYPE_FIXEDAMOUNT = 135,
        IGS_GOODS_TYPE_FIXEDQUANTITY = 136,
        IGS_GOODS_TYPE_PREMIUM = 137,
        IGS_GOODS_TYPE_CONSUMPTION = 138,
        IGS_GOODS_TYPE_ETERNITY = 139,
        IGS_GOODS_TYPE_PERIOD = 140,
        IGS_GOODS_TYPE_PREMIUM_ITEM = 406,
        IGS_GOODS_TYPE_GOBLIN_POINT = 515,
    };

    enum IGS_PACKAGE_ATTRIBUTE_TYPE
    {
        IGS_PACKAGE_ATT_TYPE_NONE = 0,
        IGS_PACKAGE_ATT_TYPE_NAME,
        IGS_PACKAGE_ATT_TYPE_DESCRIPTION,
        IGS_PACKAGE_ATT_TYPE_PRICE,
        IGS_PACKAGE_ATT_TYPE_ITEMCODE,
    };

    enum IGS_PRODUCT_ATTRIBUTE_TYPE
    {
        IGS_PRODUCT_ATT_TYPE_NONE = 0,
        IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD,
        IGS_PRODUCT_ATT_TYPE_AVALIABLE_PERIOD,
        IGS_PRODUCT_ATT_TYPE_NUM,
        IGS_PRODUCT_ATT_TYPE_PRICE,
        IGS_PRODUCT_ATT_TYPE_ITEMCODE,
        IGS_PRODUCT_ATT_TYPE_ITEMNAME,
        IGS_PRODUCT_ATT_TYPE_PRICE_SEQUENCE,
    };

    enum IGS_ETC
    {
        IGS_LIMIT_REQUEST_EVENT_PACKAGE = 20,
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
    bool SelectZone(int iIndex);
    bool SelectCategory(int iIndex);

    void BeginPage();
    void NextPage();
    void PrePage();
    int GetTotalPages();
    int GetSelectPage();

    int GetSizeZones();
    int GetSizeCategoriesAsSelectedZone();
    int GetSizePackageAsSelectedCategory();
    int GetSizePackageAsDisplayPackage();

    WORD GetPackageItemCode(int iIndex);

    type_listName& GetZoneName();
    type_listName& GetCategoryName();

    void SetTotalCash(double dTotalCash);
    void SetTotalPoint(double dTotalPoint);
    void SetTotalMileage(double dTotalMileage);
    void SetCashCreditCard(double dCashCreditCard);		// Global Credit Cash
    void SetCashPrepaid(double dCashPrepaid);			// Global Prepaid Cash
    double GetTotalCash();
    double GetTotalPoint();
    double GetTotalMileage();
    double GetCashCreditCard();							// Global Credit Cash
    double GetCashPrepaid();							// Global Prepaid Cash

    CShopPackage* GetDisplayPackage(int iIndex);

    void SetIsRequestShopOpenning(bool bIsRequestShopOpenning);
    bool GetIsRequestShopOpenning();

    bool GetPackageInfo(int iPackageSeq, int iPackageAttrType, OUT int& iValue, OUT wchar_t* pszText);

    bool GetProductInfoFromPriceSeq(int iProductSeq, int iPriceSeq, int iAttrType, OUT int& iValue, OUT wchar_t* pszUnitName);
    bool GetProductInfoFromProductSeq(int iProductSeq, int iAttrType, OUT int& iValue, OUT wchar_t* pszUnitName);

    void SetNormalPackage();
    void InitEventPackage(int iTotalEventPackage);
    void InsertEventPackage(int* pPackageSeq);

    bool IsShopOpen();
    bool IsRequestEventPackge();
    void SetRequestEventPackge();

    bool IsBanner();
    wchar_t* GetBannerFileName();
    wchar_t* GetBannerURL();

    CListVersionInfo GetScriptVer();
    CListVersionInfo GetBannerVer();
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
    CListVersionInfo GetCurrentScriptVer();
    CListVersionInfo GetCurrentBannerVer();
    bool IsScriptDownload();
    bool IsBannerDownload();

    void ShopOpenLock();
    void ShopOpenUnLock();
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

protected:
    void InitZoneInfo();
    void InitPackagePerPage(int iPageIndex);

    int GetZoneSeqIndexByIndex(int iIndex);
    int GetCategorySeqIndexByIndex(int iIndex);

    void SetCategoryName();

#ifndef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
    void ShopOpenLock();
    void ShopOpenUnLock();
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

    bool GetProductInfo(CShopProduct* pProduct, int iAttrType, OUT int& iValue, OUT wchar_t* pszUnitName);

protected:
    CShopListManager		m_ShopManager;
    CBannerListManager		m_BannerManager;

    CListVersionInfo		m_ScriptVerInfo;
    CListVersionInfo		m_BannerVerInfo;

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
    CListVersionInfo		m_CurrentScriptVerInfo;
    CListVersionInfo		m_CurrentBannerVerInfo;
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

    wchar_t					m_szScriptIPAddress[20];
    wchar_t					m_szBannerIPAddress[20];
    wchar_t					m_szScriptRemotePath[MAX_TEXT_LENGTH];
    wchar_t					m_szScriptLocalPath[MAX_TEXT_LENGTH];
    wchar_t					m_szBannerRemotePath[MAX_TEXT_LENGTH];
    wchar_t					m_szBannerLocalPath[MAX_TEXT_LENGTH];

    CShopCategory			m_SelectedZone;
    CShopCategory			m_SelectedCategory;
    CBannerInfo				m_BannerInfo;

    int						m_iSelectedPage;

    type_mapZoneSeq			m_mapZoneSeqIndex;
    type_listPackage		m_listDisplayPackage;
    type_listPackage		m_listNormalPackage;
    type_listPackage		m_listEventPackage;
    type_listPackage* m_plistSelectPackage;
    type_listName			m_listZoneName;
    type_listName			m_listCategoryName;

    CShopCategoryList* m_pCategoryList;
    CShopPackageList* m_pPackageList;
    CShopProductList* m_pProductList;

    CBannerInfoList* m_pBannerList;

    double					m_dTotalCash;
    double					m_dTotalPoint;
    double					m_dTotalMileage;
    double					m_dCashCreditCard;
    double					m_dCashPrepaid;

    bool					m_bIsRequestEventPackage;

    bool					m_bIsRequestShopOpenning;

    bool					m_bSelectEventCategory;
    bool					m_bAbleRequestEventPackage;
    int						m_iCntSelectEventZone;

    int						m_iEventPackageCnt;
    int						m_iTotalEventPackage;
    int						m_iCurrentEventPackage;

    bool					m_bIsShopOpenLock;
    bool					m_bIsBanner;
    bool					m_bFirstScriptDownloaded;
    bool					m_bFirstBannerDownloaded;
};

#define g_InGameShopSystem CInGameShopSystem::GetInstance()

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#endif // !defined(AFX_INGAMESHOPSYSTEM_H__2DF68839_DA28_44BC_B662_213BB22839CB__INCLUDED_)

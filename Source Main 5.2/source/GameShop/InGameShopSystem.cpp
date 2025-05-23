// InGameShopSystem.cpp: implementation of the InGameShopSystem class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "InGameShopSystem.h"
#include "ZzzInventory.h"
#include "MsgBoxIGSCommon.h"

#ifdef CONSOLE_DEBUG

#endif // CONSOLE_DEBUG

CInGameShopSystem::CInGameShopSystem()
{
    m_pCategoryList = NULL;
    m_pPackageList = NULL;
    m_pProductList = NULL;
    m_pBannerList = NULL;

    memset(&m_ScriptVerInfo, -1, sizeof(CListVersionInfo));
    memset(&m_BannerVerInfo, -1, sizeof(CListVersionInfo));
    memset(&m_CurrentScriptVerInfo, -1, sizeof(CListVersionInfo));
    memset(&m_CurrentBannerVerInfo, -1, sizeof(CListVersionInfo));

    m_bIsShopOpenLock = true; //louis
    m_bIsBanner = false;
    m_bIsRequestEventPackage = false;
    m_plistSelectPackage = NULL;
    m_bFirstScriptDownloaded = false;
    m_bFirstBannerDownloaded = false;
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
    m_dTotalCash = 0;
    m_dTotalPoint = 0;
    m_dTotalMileage = 0;
    m_dCashCreditCard = 0;
    m_dCashPrepaid = 0;
    m_iEventPackageCnt = 0;
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

    ::GetCurrentDirectory(255, m_szScriptLocalPath);

    wchar_t szScriptRemotePathforDMZ[MAX_TEXT_LENGTH];
    swprintf(m_szScriptLocalPath, L"%s%s", m_szScriptLocalPath, L"\\data\\InGameShopScript");
    wcscpy(m_szScriptIPAddress, L"image.webzen.com");
    wcscpy(m_szScriptRemotePath, L"/Global/Payment/ProductTransfer");
    wcscpy(szScriptRemotePathforDMZ, L"/Global/Payment/DevScriptGB/ProductTransfer");

#ifdef FOR_WORK
    HANDLE hFile;
    hFile = CreateFile(L"dmz.ini",     // file to create
        GENERIC_READ,			// open for reading
        0,						// do not share
        NULL,                   // default security
        OPEN_EXISTING,          // existing file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);                  // no template

    if (hFile != INVALID_HANDLE_VALUE)
    {
        wcscpy(m_szScriptRemotePath, szScriptRemotePathforDMZ);
    }
    CloseHandle(hFile);
#endif // FOR_WORK
    m_ShopManager.SetListManagerInfo(HTTP, m_szScriptIPAddress,
        L"",
        L"",
        m_szScriptRemotePath,
        m_szScriptLocalPath,
        m_ScriptVerInfo,
        10000);

    WZResult res = m_ShopManager.LoadScriptList(false);

    if (!res.IsSuccess())
    {
        m_pCategoryList = NULL;
        m_pPackageList = NULL;
        m_pProductList = NULL;

        ShopOpenLock();

        wchar_t szText[MAX_TEXT_LENGTH] = { '\0', };
        swprintf(szText, GlobalText[3029], m_ScriptVerInfo.Zone, m_ScriptVerInfo.year, m_ScriptVerInfo.yearId, res.GetErrorMessage());
        CMsgBoxIGSCommon* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[3028], szText);
        return false;
    }

    m_CurrentScriptVerInfo = m_ScriptVerInfo;

#ifdef CONSOLE_DEBUG
    g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt <IngameShop Script Download Success!!!>");
    g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt - Ver %d.%d.%d", m_ScriptVerInfo.Zone, m_ScriptVerInfo.year, m_ScriptVerInfo.yearId);
#endif
    ShopOpenUnLock();

    CShopList* pShopList = m_ShopManager.GetListPtr();

    m_pCategoryList = pShopList->GetCategoryListPtr();
    m_pPackageList = pShopList->GetPackageListPtr();
    m_pProductList = pShopList->GetProductListPtr();

    return true;
}

bool CInGameShopSystem::BannerDownload()
{
    m_bFirstBannerDownloaded = true;

    ::GetCurrentDirectory(255, m_szBannerLocalPath);

    wchar_t szBannerRemotePathforDMZ[MAX_TEXT_LENGTH];
    swprintf(m_szBannerLocalPath, L"%s%s", m_szBannerLocalPath, L"\\data\\InGameShopBanner");

    wcscpy(m_szBannerIPAddress, L"image.webzen.com");
    wcscpy(m_szBannerRemotePath, L"/Global/Payment/BannerTransfer");
    wcscpy(szBannerRemotePathforDMZ, L"/Global/Payment/DevScriptGB/BannerTransfer");

#ifdef FOR_WORK
    HANDLE hFile;
    hFile = CreateFile(L"dmz.ini",     // file to create
        GENERIC_READ,			// open for reading
        0,						// do not share
        NULL,                   // default security
        OPEN_EXISTING,          // existing file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);                  // no template

    if (hFile != INVALID_HANDLE_VALUE)
    {
        wcscpy(m_szBannerRemotePath, szBannerRemotePathforDMZ);
    }
    CloseHandle(hFile);
#endif // FOR_WORK

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
    m_BannerManager.SetListManagerInfo(HTTP, m_szBannerIPAddress,
        L"",
        L"",
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

    // DownLoad & Load
    WZResult res = m_BannerManager.LoadScriptList(false);

    // DownLoad & Load
    if (!res.IsSuccess())
    {
        m_pBannerList = NULL;
        m_bIsBanner = false;

        // MessageBox
        wchar_t szText[MAX_TEXT_LENGTH] = { '\0', };
        swprintf(szText, GlobalText[3030], m_BannerVerInfo.Zone, m_BannerVerInfo.year, m_BannerVerInfo.yearId, res.GetErrorMessage());
        CMsgBoxIGSCommon* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[3028], szText);

        return false;
    }

    m_CurrentBannerVerInfo = m_BannerVerInfo;
    m_pBannerList = m_BannerManager.GetListPtr();

    m_pBannerList->SetFirst();
    if (m_pBannerList->GetNext(m_BannerInfo) == false)
        return false;

    m_bIsBanner = true;
    return true;
}

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
bool CInGameShopSystem::IsScriptDownload()
{
#ifdef CONSOLE_DEBUG
    g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt CallStack - CInGameShopSystem::IsScriptDownload()");
    g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt - Script Ver %d.%d.%d", m_ScriptVerInfo.Zone, m_ScriptVerInfo.year, m_ScriptVerInfo.yearId);
    g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt - Current Ver %d.%d.%d", m_CurrentScriptVerInfo.Zone, m_CurrentScriptVerInfo.year, m_CurrentScriptVerInfo.yearId);
#endif
    if (((m_ScriptVerInfo.year == m_CurrentScriptVerInfo.year) && (m_ScriptVerInfo.yearId == m_CurrentScriptVerInfo.yearId) && (m_ScriptVerInfo.Zone == m_CurrentScriptVerInfo.Zone)) && (m_bFirstScriptDownloaded == true))
    {
#ifdef CONSOLE_DEBUG
        g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt Return - false");
#endif
        return false;
    }
#ifdef CONSOLE_DEBUG
    g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt Return - true");
#endif
    return true;
}

bool CInGameShopSystem::IsBannerDownload()
{
    if (((m_BannerVerInfo.year == m_CurrentBannerVerInfo.year) && (m_BannerVerInfo.yearId == m_CurrentBannerVerInfo.yearId) && (m_BannerVerInfo.Zone == m_CurrentBannerVerInfo.Zone)) && (m_bFirstBannerDownloaded == true))
    {
        return false;
    }

    return true;
}
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

bool CInGameShopSystem::SelectZone(int iIndex)
{
    int iZoneSeqIndex = GetZoneSeqIndexByIndex(iIndex);
    if ((INGAMESHOP_ERROR_ZERO_SIZE == iZoneSeqIndex) || (INGAMESHOP_ERROR_INVALID_INDEX == iZoneSeqIndex))
    {
        return false;
    }

    if (m_pCategoryList->GetValueByKey(iZoneSeqIndex, m_SelectedZone))
    {
        SetCategoryName();

        return true;
    }
    return false;
}

bool CInGameShopSystem::SelectCategory(int iIndex)
{
    m_listDisplayPackage.clear();

    int iCategorySeqIndex = GetCategorySeqIndexByIndex(iIndex);
    if ((INGAMESHOP_ERROR_ZERO_SIZE == iCategorySeqIndex) || (INGAMESHOP_ERROR_INVALID_INDEX == iCategorySeqIndex))
    {
        return false;
    }

    if (m_pCategoryList->GetValueByKey(iCategorySeqIndex, m_SelectedCategory))
    {
        if (m_SelectedZone.EventFlag == 199 && m_SelectedCategory.EventFlag == 199)
        {
            m_bSelectEventCategory = true;
            m_plistSelectPackage = &m_listEventPackage;
            SocketClient->ToGameServer()->SendCashShopEventItemListRequest(m_SelectedCategory.ProductDisplaySeq);
            m_bIsRequestEventPackage = true;
        }
        else
        {
            m_bSelectEventCategory = false;
            m_plistSelectPackage = &m_listNormalPackage;
            SetNormalPackage();
        }

        return true;
    }

    return false;
}

void CInGameShopSystem::BeginPage()
{
    m_iSelectedPage = 1;
    InitPackagePerPage(m_iSelectedPage);
}

void CInGameShopSystem::NextPage()
{
    if (GetTotalPages() > m_iSelectedPage)
    {
        m_iSelectedPage++;
        InitPackagePerPage(m_iSelectedPage);
    }
}

void CInGameShopSystem::PrePage()
{
    if (m_iSelectedPage > 1)
    {
        m_iSelectedPage--;
        InitPackagePerPage(m_iSelectedPage);
    }
}

int CInGameShopSystem::GetTotalPages()
{
    return (m_plistSelectPackage->size() / INGAMESHOP_DISPLAY_ITEMLIST_SIZE) + 1;
}

int CInGameShopSystem::GetSelectPage()
{
    return m_iSelectedPage;
}

void CInGameShopSystem::SetNormalPackage()
{
    CShopPackage Package;
    int iPackageSeqIndex;

    m_listNormalPackage.clear();

    m_SelectedCategory.SetPackagSeqFirst();

    while (m_SelectedCategory.GetPackagSeqNext(iPackageSeqIndex))
    {
        if (!m_pPackageList->GetValueByKey(iPackageSeqIndex, Package))
            break;

        m_listNormalPackage.push_back(Package);
    }
    BeginPage();
}

void CInGameShopSystem::InitEventPackage(int iTotalEventPackage)
{
    m_listEventPackage.clear();
    m_listDisplayPackage.clear();
    m_iTotalEventPackage = iTotalEventPackage;
    m_iEventPackageCnt = 0;
    m_iCurrentEventPackage = 0;

    if (m_iTotalEventPackage < 1)
    {
        m_bIsRequestEventPackage = false;
    }
}

void CInGameShopSystem::InsertEventPackage(int* pPackageSeq)
{
    m_SelectedCategory.SetPackagSeqFirst();

    CShopPackage Package;

    for (int i = 0; i < INGAMESHOP_DISPLAY_ITEMLIST_SIZE; i++)
    {
        if (m_pPackageList->GetValueByKey(pPackageSeq[i], Package))
        {
            m_listEventPackage.push_back(Package);
        }

        m_iCurrentEventPackage++;

        if (m_iTotalEventPackage == m_iCurrentEventPackage)
        {
            BeginPage();
            m_bIsRequestEventPackage = false;
            return;
        }
    }
}

int CInGameShopSystem::GetSizeZones()
{
    return m_mapZoneSeqIndex.size();
}

int CInGameShopSystem::GetSizeCategoriesAsSelectedZone()
{
    return m_SelectedZone.CategoryList.size();
}

int CInGameShopSystem::GetSizePackageAsSelectedCategory()
{
    return m_plistSelectPackage->size();
}

int CInGameShopSystem::GetSizePackageAsDisplayPackage()
{
    return m_listDisplayPackage.size();
}

type_listName& CInGameShopSystem::GetZoneName()
{
    return m_listZoneName;
}

type_listName& CInGameShopSystem::GetCategoryName()
{
    return m_listCategoryName;
}

WORD CInGameShopSystem::GetPackageItemCode(int iIndex)
{
    auto iterPackage = m_listDisplayPackage.begin();

    for (int i = 0; i < (int)m_listDisplayPackage.size(); i++)
    {
        if (iterPackage == m_listDisplayPackage.end())
            return -1;

        if (i == iIndex)
            break;

        iterPackage++;
    }

    return _wtoi((*iterPackage).InGamePackageID);
}

void CInGameShopSystem::SetTotalCash(double dTotalCash)
{
    m_dTotalCash = dTotalCash;
}

void CInGameShopSystem::SetTotalPoint(double dTotalPoint)
{
    m_dTotalPoint = dTotalPoint;
}

void CInGameShopSystem::SetTotalMileage(double dTotalMileage)
{
    m_dTotalMileage = dTotalMileage;
}

void CInGameShopSystem::SetCashCreditCard(double dCashCreditCard)
{
    m_dCashCreditCard = dCashCreditCard;
}

void CInGameShopSystem::SetCashPrepaid(double dCashPrepaid)
{
    m_dCashPrepaid = dCashPrepaid;
}

double CInGameShopSystem::GetTotalCash()
{
    return m_dTotalCash;
}

double CInGameShopSystem::GetTotalPoint()
{
    return m_dTotalPoint;
}

double CInGameShopSystem::GetTotalMileage()
{
    return m_dTotalMileage;
}

double CInGameShopSystem::GetCashCreditCard()
{
    return m_dCashCreditCard;
}

double CInGameShopSystem::GetCashPrepaid()
{
    return m_dCashPrepaid;
}

CShopPackage* CInGameShopSystem::GetDisplayPackage(int iIndex)
{
    auto iterPackage = m_listDisplayPackage.begin();

    for (int i = 0; i < (int)m_listDisplayPackage.size(); i++)
    {
        if (iterPackage == m_listDisplayPackage.end())
            return NULL;

        if (i == iIndex)
            break;

        iterPackage++;
    }

    return &(*iterPackage);
}

void CInGameShopSystem::SetIsRequestShopOpenning(bool IsRequestShopOpenning)
{
    m_bIsRequestShopOpenning = IsRequestShopOpenning;
}

bool CInGameShopSystem::GetIsRequestShopOpenning()
{
    return m_bIsRequestShopOpenning;
}

bool CInGameShopSystem::GetPackageInfo(int iPackageSeq, int iPackageAttrType, OUT int& iValue, OUT wchar_t* pszText)
{
    CShopPackage Package;

    if (m_pPackageList->GetValueByKey(iPackageSeq, Package) == true)
    {
        switch (iPackageAttrType)
        {
        case IGS_PACKAGE_ATT_TYPE_NAME:
        {
            iValue = 0;
            wcscpy(pszText, Package.PackageProductName);
            return true;
        }break;
        case IGS_PACKAGE_ATT_TYPE_DESCRIPTION:
        {
            iValue = 0;
            wcscpy(pszText, Package.Description);
            return true;
        }break;
        case IGS_PACKAGE_ATT_TYPE_PRICE:
        {
            wchar_t szText[MAX_TEXT_LENGTH] = { '\0', };
            iValue = Package.Price;
            ConvertGold(iValue, szText);
            swprintf(pszText, L"%s %s", szText, Package.PricUnitName);
            return true;
        }break;
        case IGS_PACKAGE_ATT_TYPE_ITEMCODE:
        {
            iValue = _wtoi(Package.InGamePackageID);
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

bool CInGameShopSystem::GetProductInfoFromPriceSeq(int iProductSeq, int iPriceSeq, int iAttrType, OUT int& iValue, OUT wchar_t* pszUnitName)
{
    CShopProduct Product;

    m_pProductList->SetPriceSeqFirst(iProductSeq, iPriceSeq);

    while (m_pProductList->GetPriceSeqNext(Product))
    {
        if (GetProductInfo(&Product, iAttrType, iValue, pszUnitName) == true)
        {
            return true;
        }
    }

    iValue = -1;
    pszUnitName[0] = '\0';

    return false;
}

bool CInGameShopSystem::GetProductInfoFromProductSeq(int iProductSeq, int iAttrType, OUT int& iValue, OUT wchar_t* pszUnitName)
{
    CShopProduct Product;

    m_pProductList->SetProductSeqFirst(iProductSeq);

    while (m_pProductList->GetProductSeqNext(Product))
    {
        if (GetProductInfo(&Product, iAttrType, iValue, pszUnitName) == true)
        {
            return true;
        }
    }

    iValue = -1;
    pszUnitName[0] = '\0';

    return false;
}

bool CInGameShopSystem::GetProductInfo(CShopProduct* pProduct, int iAttrType, OUT int& iValue, OUT wchar_t* pszUnitName)
{
    switch (iAttrType)
    {
    case IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD:
    {
        if ((pProduct->PropertySeq == 2) || (pProduct->PropertySeq == 28) || (pProduct->PropertySeq == 12)
            || (pProduct->PropertySeq == 58) || (pProduct->PropertySeq == 10))
        {
            iValue = _wtoi(pProduct->Value);
            switch (pProduct->UnitType)
            {
            case 386:
            {
                if (iValue >= 86400)
                {
                    iValue /= 86400;
                    wcscpy(pszUnitName, GlobalText[2298]);
                }
                else if (iValue >= 3600)
                {
                    iValue /= 3600;
                    wcscpy(pszUnitName, GlobalText[2299]);
                }
                else if (iValue >= 60)
                {
                    iValue /= 60;
                    wcscpy(pszUnitName, GlobalText[2300]);
                }
                else
                {
                    wcscpy(pszUnitName, GlobalText[2301]);
                }
            }break;
            case 174:
            {
                if (iValue >= 1440)
                {
                    iValue /= 1440;
                    wcscpy(pszUnitName, GlobalText[2298]);
                }
                else if (iValue >= 60)
                {
                    iValue /= 60;
                    wcscpy(pszUnitName, GlobalText[2299]);
                }
                else
                {
                    wcscpy(pszUnitName, GlobalText[2300]);
                }
            }break;
            case 172:
            {
                if (iValue >= 24)
                {
                    iValue /= 24;
                    wcscpy(pszUnitName, GlobalText[2298]);
                }
                else
                {
                    wcscpy(pszUnitName, GlobalText[2299]);
                }
            }break;
            default:
            {
                wcscpy(pszUnitName, pProduct->UnitName);
            }break;
            }
            return true;
        }
    }break;
    case IGS_PRODUCT_ATT_TYPE_AVALIABLE_PERIOD:
    {
        if ((pProduct->PropertySeq == 46) || (pProduct->PropertySeq == 49) || (pProduct->PropertySeq == 48) || (pProduct->PropertySeq == 51) || (pProduct->PropertySeq == 52) || (pProduct->PropertySeq == 53) || (pProduct->PropertySeq == 50) || (pProduct->PropertySeq == 60))
        {
            iValue = _wtoi(pProduct->Value);
            wcscpy(pszUnitName, pProduct->UnitName);
            return true;
        }
    }break;
    case IGS_PRODUCT_ATT_TYPE_NUM:
    {
        if ((pProduct->PropertySeq == 30) || (pProduct->PropertySeq == 11) || (pProduct->PropertySeq == 7) || (pProduct->PropertySeq == 8) || (pProduct->PropertySeq == 9) || (pProduct->PropertySeq == 31))
        {
            iValue = _wtoi(pProduct->Value);
            wcscpy(pszUnitName, pProduct->UnitName);
            return true;
        }
    }break;
    case IGS_PRODUCT_ATT_TYPE_PRICE:
    {
        iValue = pProduct->Price;
        ConvertGold(pProduct->Price, pszUnitName);
        return true;
    }break;
    case IGS_PRODUCT_ATT_TYPE_ITEMCODE:
    {
        iValue = _wtoi(pProduct->InGamePackageID);
        pszUnitName[0] = '\0';
        return true;
    }break;
    case IGS_PRODUCT_ATT_TYPE_ITEMNAME:
    {
        iValue = -1;
        wcscpy(pszUnitName, pProduct->ProductName);
        return true;
    }break;
    case IGS_PRODUCT_ATT_TYPE_PRICE_SEQUENCE:
    {
        iValue = pProduct->PriceSeq;
        pszUnitName[0] = '\0';
        return true;
    }break;
    default:
    {
        iValue = -1;
        pszUnitName[0] = '\0';
    }break;
    }

    return false;
}

bool CInGameShopSystem::IsRequestEventPackge()
{
    if (m_bIsRequestEventPackage == true)
        return false;

    return true;
}

void CInGameShopSystem::SetRequestEventPackge()
{
    m_bIsRequestEventPackage = false;
}

bool CInGameShopSystem::IsShopOpen()
{
    return m_bIsShopOpenLock ? false : true;
}

bool CInGameShopSystem::IsBanner()
{
    return m_bIsBanner;
}

wchar_t* CInGameShopSystem::GetBannerFileName()
{
    if (m_bIsBanner == false)
        return NULL;

    return m_BannerInfo.BannerImagePath;
}

wchar_t* CInGameShopSystem::GetBannerURL()
{
    if (m_bIsBanner == false)
        return NULL;

    return m_BannerInfo.BannerLinkURL;
}

void CInGameShopSystem::InitZoneInfo()
{
    m_mapZoneSeqIndex.clear();
    m_listZoneName.clear();

    m_pCategoryList->SetFirst();
    CShopCategory Zone;

    int i = 0;
    while (m_pCategoryList->GetNext(Zone))
    {
        if (1 == Zone.Root)
        {
            m_mapZoneSeqIndex.insert(type_mapZoneSeq::value_type(i++, Zone.ProductDisplaySeq));
            m_listZoneName.push_back(Zone.CategroyName);
        }
    }
}

void CInGameShopSystem::InitPackagePerPage(int iPageIndex)
{
    m_listDisplayPackage.clear();

    type_listPackage::iterator	iterlistPackage;

    iterlistPackage = m_plistSelectPackage->begin();

    int iBeginDisplayItemIndex = INGAMESHOP_DISPLAY_ITEMLIST_SIZE * (iPageIndex - 1);
    for (int i = 0; i < iBeginDisplayItemIndex; i++)
    {
        iterlistPackage++;
    }

    for (int j = 0; j < INGAMESHOP_DISPLAY_ITEMLIST_SIZE; j++)
    {
        if (iterlistPackage == m_plistSelectPackage->end())
            break;

        m_listDisplayPackage.push_back(*iterlistPackage);
        iterlistPackage++;
    }
}

int CInGameShopSystem::GetZoneSeqIndexByIndex(int iIndex)
{
    if (GetSizeZones() <= 0)
        return INGAMESHOP_ERROR_ZERO_SIZE;

    auto iterZoneSeqIndex = m_mapZoneSeqIndex.find(iIndex);

    if (iterZoneSeqIndex == m_mapZoneSeqIndex.end())
        return INGAMESHOP_ERROR_INVALID_INDEX;

    return (int)iterZoneSeqIndex->second;
}

int CInGameShopSystem::GetCategorySeqIndexByIndex(int iIndex)
{
    int iCategorySeqIndex = 0;
    bool bRes = false;

    if (GetSizeCategoriesAsSelectedZone() <= 0)
        return INGAMESHOP_ERROR_ZERO_SIZE;

    m_SelectedZone.SetCategoryFirst();
    for (int i = 0; i <= iIndex; i++)
    {
        bRes = m_SelectedZone.GetCategoryNext(iCategorySeqIndex);
    }

    if (bRes == false)
        return INGAMESHOP_ERROR_INVALID_INDEX;

    return iCategorySeqIndex;
}

void CInGameShopSystem::SetCategoryName()
{
    m_listCategoryName.clear();

    int iCategorySeqIndex;
    CShopCategory Category;
    m_SelectedZone.SetCategoryFirst();

    while (m_SelectedZone.GetCategoryNext(iCategorySeqIndex))
    {
        m_pCategoryList->GetValueByKey(iCategorySeqIndex, Category);
        m_listCategoryName.push_back(Category.CategroyName);
    }
}

void CInGameShopSystem::ShopOpenLock()
{
    m_bIsShopOpenLock = true;
}

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
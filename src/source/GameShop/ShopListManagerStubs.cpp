// Story 7.6.1: No-op stubs for ShopListManager classes on non-Win32.
//
// All ShopListManager/*.cpp files guard their implementations with #ifdef _WIN32,
// producing empty object files on macOS/Linux. InGameShopSystem.cpp (no such guard)
// references these class methods, causing linker failures.
//
// This file provides minimal no-op implementations so the game binary links.
// The in-game shop is non-functional on macOS — replaced by platform-agnostic
// shop API in EPIC-5.
// [VS0-QUAL-BUILDCOMP-MACOS]

#include "stdafx.h"

#ifndef _WIN32

#include "GameShop/ShopListManager/ShopCategoryList.h"
#include "GameShop/ShopListManager/ShopPackageList.h"
#include "GameShop/ShopListManager/ShopProductList.h"
#include "GameShop/ShopListManager/BannerInfoList.h"
#include "GameShop/ShopListManager/BannerListManager.h"
#include "GameShop/ShopListManager/ShopListManager.h"

// ============================================================
// WZResult
// ============================================================
WZResult::WZResult() : m_dwErrorCode(0), m_dwWindowErrorCode(0)
{
    memset(m_szErrorMessage, 0, sizeof(m_szErrorMessage));
}
WZResult::~WZResult() = default;
WZResult& WZResult::operator=(const WZResult& result)
{
    if (this != &result)
    {
        m_dwErrorCode = result.m_dwErrorCode;
        m_dwWindowErrorCode = result.m_dwWindowErrorCode;
        memcpy(m_szErrorMessage, result.m_szErrorMessage, sizeof(m_szErrorMessage));
    }
    return *this;
}
BOOL WZResult::IsSuccess()
{
    return TRUE;
}
TCHAR* WZResult::GetErrorMessage()
{
    return m_szErrorMessage;
}
DWORD WZResult::GetErrorCode()
{
    return m_dwErrorCode;
}
DWORD WZResult::GetWindowErrorCode()
{
    return m_dwWindowErrorCode;
}
void WZResult::SetSuccessResult()
{
    m_dwErrorCode = 0;
}
void WZResult::SetResult(DWORD /*dwErrorCode*/, DWORD /*dwWindowErrorCode*/, const TCHAR* /*szFormat*/, ...) {}
WZResult WZResult::BuildSuccessResult()
{
    WZResult r;
    r.SetSuccessResult();
    return r;
}
WZResult WZResult::BuildResult(DWORD /*dwErrorCode*/, DWORD /*dwWindowErrorCode*/, const TCHAR* /*szFormat*/, ...)
{
    return WZResult();
}

// ============================================================
// CShopCategory
// ============================================================
CShopCategory::CShopCategory()
    : ProductDisplaySeq(0), EventFlag(0), OpenFlag(0), ParentProductDisplaySeq(0), DisplayOrder(0), Root(0)
{
    memset(CategroyName, 0, sizeof(CategroyName));
}
CShopCategory::~CShopCategory() = default;
bool CShopCategory::SetCategory(std::wstring /*strdata*/)
{
    return false;
}
void CShopCategory::SetCategoryFirst() {}
bool CShopCategory::GetCategoryNext(int& /*CategorySeq*/)
{
    return false;
}
void CShopCategory::SetPackagSeqFirst() {}
bool CShopCategory::GetPackagSeqNext(int& /*PackagSeq*/)
{
    return false;
}
void CShopCategory::AddPackageSeq(int /*PackageSeq*/) {}
void CShopCategory::ClearPackageSeq() {}

// ============================================================
// CShopCategoryList
// ============================================================
CShopCategoryList::CShopCategoryList() = default;
CShopCategoryList::~CShopCategoryList() = default;
void CShopCategoryList::Clear()
{
    m_Categroys.clear();
}
int CShopCategoryList::GetSize()
{
    return static_cast<int>(m_Categroys.size());
}
void CShopCategoryList::Append(CShopCategory /*category*/) {}
void CShopCategoryList::SetFirst()
{
    m_Categoryiter = m_Categroys.begin();
}
bool CShopCategoryList::GetNext(CShopCategory& /*category*/)
{
    return false;
}
bool CShopCategoryList::GetValueByKey(int /*nKey*/, CShopCategory& /*category*/)
{
    return false;
}
bool CShopCategoryList::GetValueByIndex(int /*nIndex*/, CShopCategory& /*category*/)
{
    return false;
}
bool CShopCategoryList::InsertPackage(int /*Category*/, int /*Package*/)
{
    return false;
}
bool CShopCategoryList::RefreshPackageSeq(int /*Category*/, int /*PackageSeqs*/[], int /*PackageCount*/)
{
    return false;
}

// ============================================================
// CShopPackage
// ============================================================
CShopPackage::CShopPackage()
    : ProductDisplaySeq(0), ViewOrder(0), PackageProductSeq(0), PackageProductType(0), Price(0), SalesFlag(0),
      GiftFlag(0), CapsuleFlag(0), CapsuleCount(0), DeleteFlag(0), EventFlag(0), ProductAmount(0), ProductCashSeq(0),
      PriceCount(0), DeductMileageFlag(false), CashType(0), CashTypeFlag(0), LeftCount(0)
{
    memset(PackageProductName, 0, sizeof(PackageProductName));
    memset(Description, 0, sizeof(Description));
    memset(Caution, 0, sizeof(Caution));
    memset(&StartDate, 0, sizeof(StartDate));
    memset(&EndDate, 0, sizeof(EndDate));
    memset(ProductCashName, 0, sizeof(ProductCashName));
    memset(PricUnitName, 0, sizeof(PricUnitName));
    memset(InGamePackageID, 0, sizeof(InGamePackageID));
}
CShopPackage::~CShopPackage() = default;
bool CShopPackage::SetPackage(std::wstring /*strdata*/)
{
    return false;
}
void CShopPackage::SetLeftCount(int nCount)
{
    LeftCount = nCount;
}
int CShopPackage::GetProductCount()
{
    return 0;
}
void CShopPackage::SetProductSeqFirst() {}
bool CShopPackage::GetProductSeqFirst(int& /*ProductSeq*/)
{
    return false;
}
bool CShopPackage::GetProductSeqNext(int& /*ProductSeq*/)
{
    return false;
}
int CShopPackage::GetPriceCount()
{
    return 0;
}
void CShopPackage::SetPriceSeqFirst() {}
bool CShopPackage::GetPriceSeqFirst(int& /*PriceSeq*/)
{
    return false;
}
bool CShopPackage::GetPriceSeqNext(int& /*PriceSeq*/)
{
    return false;
}
void CShopPackage::SetProductSeqList(std::wstring /*strdata*/) {}
void CShopPackage::SetPriceSeqList(std::wstring /*strdata*/) {}

// ============================================================
// CShopPackageList
// ============================================================
CShopPackageList::CShopPackageList() = default;
CShopPackageList::~CShopPackageList() = default;
int CShopPackageList::GetSize()
{
    return 0;
}
void CShopPackageList::Clear()
{
    m_Packages.clear();
}
void CShopPackageList::Append(CShopPackage /*package*/) {}
void CShopPackageList::SetFirst()
{
    m_iter = m_Packages.begin();
}
bool CShopPackageList::GetNext(CShopPackage& /*package*/)
{
    return false;
}
bool CShopPackageList::GetValueByKey(int /*nKey*/, CShopPackage& /*package*/)
{
    return false;
}
bool CShopPackageList::GetValueByIndex(int /*nIndex*/, CShopPackage& /*package*/)
{
    return false;
}
bool CShopPackageList::SetPacketLeftCount(int /*PackageSeq*/, int /*nCount*/)
{
    return false;
}

// ============================================================
// CShopProduct
// ============================================================
CShopProduct::CShopProduct()
    : ProductSeq(0), Price(0), PriceSeq(0), PropertyType(0), MustFlag(0), vOrder(0), DeleteFlag(0), StorageGroup(0),
      ShareFlag(0), PropertySeq(0), ProductType(0), UnitType(0)
{
    memset(ProductName, 0, sizeof(ProductName));
    memset(PropertyName, 0, sizeof(PropertyName));
    memset(Value, 0, sizeof(Value));
    memset(UnitName, 0, sizeof(UnitName));
    memset(InGamePackageID, 0, sizeof(InGamePackageID));
}
CShopProduct::~CShopProduct() = default;
bool CShopProduct::SetProduct(std::wstring /*strdata*/)
{
    return false;
}

// ============================================================
// CShopProductList
// ============================================================
CShopProductList::CShopProductList() : PriceSeqKey(0) {}
CShopProductList::~CShopProductList() = default;
void CShopProductList::Clear()
{
    m_Products.clear();
}
int CShopProductList::GetSize()
{
    return static_cast<int>(m_Products.size());
}
void CShopProductList::Append(CShopProduct /*product*/) {}
void CShopProductList::SetFirst()
{
    m_ProductIter = m_Products.begin();
}
bool CShopProductList::GetNext(CShopProduct& /*product*/)
{
    return false;
}
void CShopProductList::SetProductSeqFirst(int /*ProductSeq*/) {}
bool CShopProductList::GetProductSeqNext(CShopProduct& /*product*/)
{
    return false;
}
void CShopProductList::SetPriceSeqFirst(int /*ProductSeq*/, int /*PriceSeq*/) {}
bool CShopProductList::GetPriceSeqNext(CShopProduct& /*product*/)
{
    return false;
}

// ============================================================
// CBannerInfo
// ============================================================
CBannerInfo::CBannerInfo() : BannerSeq(0), BannerOrder(0), BannerDirection(0)
{
    memset(BannerName, 0, sizeof(BannerName));
    memset(BannerImageURL, 0, sizeof(BannerImageURL));
    memset(&BannerStartDate, 0, sizeof(BannerStartDate));
    memset(&BannerEndDate, 0, sizeof(BannerEndDate));
    memset(BannerLinkURL, 0, sizeof(BannerLinkURL));
    memset(BannerImagePath, 0, sizeof(BannerImagePath));
}
CBannerInfo::~CBannerInfo() = default;
bool CBannerInfo::SetBanner(std::wstring /*strdata*/, std::wstring /*strDirPath*/, bool /*bDonwLoad*/)
{
    return false;
}

// ============================================================
// CBannerInfoList
// ============================================================
CBannerInfoList::CBannerInfoList() = default;
CBannerInfoList::~CBannerInfoList() = default;
WZResult CBannerInfoList::LoadBanner(std::wstring /*strDirPath*/, std::wstring /*strScriptFileName*/,
                                     bool /*bDonwLoad*/)
{
    return WZResult::BuildSuccessResult();
}
void CBannerInfoList::Clear()
{
    m_BannerInfos.clear();
}
int CBannerInfoList::GetSize()
{
    return 0;
}
void CBannerInfoList::Append(CBannerInfo /*banner*/) {}
void CBannerInfoList::SetFirst()
{
    m_BannerInfoIter = m_BannerInfos.begin();
}
bool CBannerInfoList::GetNext(CBannerInfo& /*banner*/)
{
    return false;
}

// ============================================================
// CFTPFileDownLoader
// ============================================================
CFTPFileDownLoader::CFTPFileDownLoader() : m_Break(FALSE), m_pFileDownloader(nullptr) {}
CFTPFileDownLoader::~CFTPFileDownLoader() = default;
WZResult CFTPFileDownLoader::DownLoadFiles(DownloaderType /*type*/, std::wstring /*strServerIP*/,
                                           unsigned short /*PortNum*/, std::wstring /*strUserName*/,
                                           std::wstring /*strPWD*/, std::wstring /*strRemotepath*/,
                                           std::wstring /*strlocalpath*/, bool /*bPassiveMode*/,
                                           CListVersionInfo /*Version*/, std::vector<std::wstring> /*vScriptFiles*/)
{
    return WZResult::BuildSuccessResult();
}
void CFTPFileDownLoader::Break()
{
    m_Break = TRUE;
}

// ============================================================
// CListManager
// ============================================================
CListManager::CListManager() : m_pFTPDownLoader(nullptr) {}
CListManager::~CListManager()
{
    delete m_pFTPDownLoader;
}
void CListManager::SetListManagerInfo(DownloaderType /*type*/, const wchar_t* /*ServerIP*/, const wchar_t* /*UserID*/,
                                      const wchar_t* /*Pwd*/, const wchar_t* /*RemotePath*/,
                                      const wchar_t* /*LocalPath*/, CListVersionInfo /*Version*/,
                                      DWORD /*dwDownloadMaxTime*/)
{
}
void CListManager::SetListManagerInfo(DownloaderType /*type*/, const wchar_t* /*ServerIP*/, unsigned short /*PortNum*/,
                                      const wchar_t* /*UserID*/, const wchar_t* /*Pwd*/, const wchar_t* /*RemotePath*/,
                                      const wchar_t* /*LocalPath*/, FTP_SERVICE_MODE /*ftpMode*/,
                                      CListVersionInfo /*Version*/, DWORD /*dwDownloadMaxTime*/)
{
}
WZResult CListManager::LoadScriptList(bool /*bDonwLoad*/)
{
    return WZResult::BuildSuccessResult();
}
bool CListManager::IsScriptFileExist()
{
    return false;
}
std::wstring CListManager::GetScriptPath()
{
    return L"";
}
void CListManager::DeleteScriptFiles() {}
WZResult CListManager::FileDownLoad()
{
    return WZResult::BuildSuccessResult();
}
WZResult CListManager::FileDownLoadImpl()
{
    return WZResult::BuildSuccessResult();
}
unsigned int CListManager::RunFileDownLoadThread(LPVOID /*pParam*/)
{
    return 0;
}

// ============================================================
// CShopListManager
// ============================================================
CShopListManager::CShopListManager() : m_ShopList(nullptr) {}
CShopListManager::~CShopListManager()
{
    delete m_ShopList;
}
WZResult CShopListManager::LoadScript(bool /*bDonwLoad*/)
{
    return WZResult::BuildSuccessResult();
}

// ============================================================
// CBannerListManager
// ============================================================
CBannerListManager::CBannerListManager() : m_BannerInfoList(nullptr) {}
CBannerListManager::~CBannerListManager()
{
    delete m_BannerInfoList;
}
WZResult CBannerListManager::LoadScript(bool /*bDonwLoad*/)
{
    return WZResult::BuildSuccessResult();
}

#endif // !_WIN32

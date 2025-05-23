#pragma once

#define STRSAFE_NO_DEPRECATE

#define SHOPLIST_SCRIPT_COUNT				3

#define SHOPLIST_SCRIPT_CATEGORY			L"IBSCategory.txt"
#define SHOPLIST_SCRIPT_PACKAGE				L"IBSPackage.txt"
#define SHOPLIST_SCRIPT_PRODUCT				L"IBSProduct.txt"
#define BANNER_SCRIPT_FILENAME				L"IBSBanner.txt"

#define SHOPLIST_LENGTH_CATEGORYNAME		128

#define SHOPLIST_LENGTH_PACKAGENAME			512
#define SHOPLIST_LENGTH_PACKAGEDESC			2048
#define SHOPLIST_LENGTH_PACKAGECAUTION		1024
#define SHOPLIST_LENGTH_PACKAGECASHNAME		256
#define SHOPLIST_LENGTH_PACKAGEPRICEUNIT	64

#define SHOPLIST_LENGTH_PRODUCTNAME			128
#define SHOPLIST_LENGTH_PRODUCTPROPERTYNAME 128
#define SHOPLIST_LENGTH_PRODUCTVALUE		512
#define SHOPLIST_LENGTH_PRODUCTUNITNAME		64
#define SHOPLIST_LENGTH_INGAMEPACKAGEID		20

#define BANNER_LENGTH_NAME					50

#define ERROR_TIMEOUT_BREAK					0x01
#define ERROR_FILE_SIZE_ZERO				0x02
#define ERROR_CATEGORY_OPEN_FAIL			0x03
#define ERROR_PACKAGE_OPEN_FAIL				0x04
#define ERROR_PRODUCT_OPEN_FAIL				0x05
#define ERROR_BANNER_OPEN_FAIL				0x06
#define ERROR_LOAD_SCRIPT					0x07
#define ERROR_THREAD						0x08

#include <Wininet.h>
#include <vector>
#include <string>
#include <tchar.h>
#include <crtdbg.h>
#include <strsafe.h>
#include "GameShop\ShopListManager\interface\WZResult\WZResult.h"
#include "GameShop\ShopListManager\interface\DownloadInfo.h"
#include "GameShop\ShopListManager\interface\FileDownloader.h"

#if !defined (INVALID_FILE_ATTRIBUTES)
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

//#ifdef _DEBUG
//	#pragma  comment(lib, L"FileDownloader.lib")
//#else
//	#pragma  comment(lib, L"FileDownloader.lib")
//#endif

enum FTP_SERVICE_MODE { FTP_MODE_ACTIVE, FTP_MODE_PASSIVE };
enum FILE_ENCODE
{
    FE_ANSI,
    FE_UTF8,
    FE_UNICODE
};

class CListVersionInfo
{
public:
    unsigned short Zone;
    unsigned short year;
    unsigned short yearId;
};

class CListManagerInfo
{
public:
    DownloaderType		m_DownloaderType;
    std::wstring			m_strServerIP;
    unsigned short		m_nPortNum;
    std::wstring			m_strUserID;
    std::wstring			m_strPWD;
    std::wstring			m_strRemotePath;
    FTP_SERVICE_MODE	m_ftpMode;
    std::wstring			m_strLocalPath;
    DWORD				m_dwDownloadMaxTime;

    CListVersionInfo	m_Version;
};

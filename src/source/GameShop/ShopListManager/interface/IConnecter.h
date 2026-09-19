/*******************************************************************************
 *	Connecter Interface — libcurl-based
 *	Migrated from WinINet (Story 7.6.6)
 *******************************************************************************/

#pragma once

#include <string>
#include <curl/curl.h>
#include "GameShop/ShopListManager/interface/WZResult/WZResult.h"
#include "GameShop/ShopListManager/interface/DownloadInfo.h"

class IConnecter
{
public:
    IConnecter(DownloadServerInfo* pServerInfo, DownloadFileInfo* pFileInfo)
        : m_pServerInfo(pServerInfo), m_pFileInfo(pFileInfo) {};
    virtual ~IConnecter() {};

    virtual std::string BuildURL() = 0;
    virtual WZResult ConfigureCurl(CURL* curl) = 0;

protected:
    WZResult m_Result;
    DownloadServerInfo* m_pServerInfo;
    DownloadFileInfo* m_pFileInfo;
};

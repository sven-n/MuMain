/*******************************************************************************
 *	HTTP Connecter — libcurl-based
 *	Migrated from WinINet (Story 7.6.6)
 *******************************************************************************/

#pragma once

#include "GameShop/ShopListManager/interface/IConnecter.h"

class HTTPConnecter : public IConnecter
{
public:
    HTTPConnecter(DownloadServerInfo* pServerInfo, DownloadFileInfo* pFileInfo);
    ~HTTPConnecter();

    std::string BuildURL() override;
    WZResult ConfigureCurl(CURL* curl) override;
};

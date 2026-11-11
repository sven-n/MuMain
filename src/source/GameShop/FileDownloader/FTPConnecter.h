/*******************************************************************************
 *	FTP Connecter — libcurl-based
 *	Migrated from WinINet (Story 7.6.6)
 *******************************************************************************/

#pragma once

#include "GameShop/ShopListManager/interface/IConnecter.h"

class FTPConnecter : public IConnecter
{
public:
    FTPConnecter(DownloadServerInfo* pServerInfo, DownloadFileInfo* pFileInfo);
    ~FTPConnecter();

    std::string BuildURL() override;
    WZResult ConfigureCurl(CURL* curl) override;
};

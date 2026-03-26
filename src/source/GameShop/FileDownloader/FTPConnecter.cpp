//************************************************************************
//
// FILE: FTPConnecter.cpp
// Migrated from WinINet to libcurl (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "FTPConnecter.h"

FTPConnecter::FTPConnecter(DownloadServerInfo* pServerInfo, DownloadFileInfo* pFileInfo)
    : IConnecter(pServerInfo, pFileInfo)
{
}

FTPConnecter::~FTPConnecter() {}

std::string FTPConnecter::BuildURL()
{
    // Convert wide strings to narrow for curl URL
    char serverURL[DL_MAX_URL_LENGTH] = {0};
    char remotePath[DL_MAX_URL_LENGTH] = {0};

    wcstombs(serverURL, this->m_pServerInfo->GetServerURL(), sizeof(serverURL) - 1);
    wcstombs(remotePath, this->m_pFileInfo->GetRemoteFilePath(), sizeof(remotePath) - 1);

    std::string url = "ftp://";
    url += serverURL;
    url += ":";
    url += std::to_string(this->m_pServerInfo->GetPort());
    url += "/";
    url += remotePath;

    return url;
}

WZResult FTPConnecter::ConfigureCurl(CURL* curl)
{
    // FTP-specific options
    if (this->m_pServerInfo->IsPassive())
    {
        // EPRT/EPSV disabled = passive mode
    }
    else
    {
        curl_easy_setopt(curl, CURLOPT_FTPPORT, "-"); // Active mode: use default interface
    }

    // FTP credentials
    char userID[DL_MAX_USER_NAME_LENGTH] = {0};
    char password[DL_MAX_PASSWORD_LENGTH] = {0};

    wcstombs(userID, this->m_pServerInfo->GetUserID(), sizeof(userID) - 1);
    wcstombs(password, this->m_pServerInfo->GetPassword(), sizeof(password) - 1);

    std::string userpwd = std::string(userID) + ":" + std::string(password);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str());

    this->m_Result.SetSuccessResult();
    return this->m_Result;
}
#endif

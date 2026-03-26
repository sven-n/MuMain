//************************************************************************
//
// FILE: HTTPConnecter.cpp
// Migrated from WinINet to libcurl (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "HTTPConnecter.h"

HTTPConnecter::HTTPConnecter(DownloadServerInfo* pServerInfo, DownloadFileInfo* pFileInfo)
    : IConnecter(pServerInfo, pFileInfo)
{
}

HTTPConnecter::~HTTPConnecter() {}

std::string HTTPConnecter::BuildURL()
{
    char serverURL[DL_MAX_URL_LENGTH] = {0};
    char remotePath[DL_MAX_URL_LENGTH] = {0};

    wcstombs(serverURL, this->m_pServerInfo->GetServerURL(), sizeof(serverURL) - 1);
    wcstombs(remotePath, this->m_pFileInfo->GetRemoteFilePath(), sizeof(remotePath) - 1);

    std::string url = "http://";
    url += serverURL;
    url += ":";
    url += std::to_string(this->m_pServerInfo->GetPort());
    url += "/";
    url += remotePath;

    return url;
}

WZResult HTTPConnecter::ConfigureCurl(CURL* curl)
{
    // HTTP-specific: follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // HTTP credentials if provided
    char userID[DL_MAX_USER_NAME_LENGTH] = {0};
    wcstombs(userID, this->m_pServerInfo->GetUserID(), sizeof(userID) - 1);

    if (userID[0] != '\0')
    {
        char password[DL_MAX_PASSWORD_LENGTH] = {0};
        wcstombs(password, this->m_pServerInfo->GetPassword(), sizeof(password) - 1);

        std::string userpwd = std::string(userID) + ":" + std::string(password);
        curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str());
    }

    this->m_Result.SetSuccessResult();
    return this->m_Result;
}
#endif

//************************************************************************
//
// FILE: BannerInfo.cpp
// Migrated from Urlmon/WinINet to libcurl (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "BannerInfo.h"
#include "StringToken.h"
#include "StringMethod.h"

#include <curl/curl.h>
#include <fstream>
#include <filesystem>

namespace
{
size_t BannerWriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    auto* stream = static_cast<std::ofstream*>(userdata);
    size_t bytes = size * nmemb;
    stream->write(static_cast<const char*>(ptr), static_cast<std::streamsize>(bytes));
    return stream->good() ? bytes : 0;
}
} // namespace

// cppcheck-suppress uninitMemberVar
CBannerInfo::CBannerInfo()
{
    memset(this->BannerName, 0, sizeof(this->BannerName));
    memset(this->BannerImageURL, 0, sizeof(this->BannerImageURL));
    memset(this->BannerImagePath, 0, sizeof(this->BannerImagePath));
    memset(this->BannerLinkURL, 0, sizeof(this->BannerLinkURL));
}
CBannerInfo::~CBannerInfo() {}

bool CBannerInfo::SetBanner(std::wstring strdata, std::wstring strDirPath, bool bDonwLoad)
{
    if (strdata.empty())
        return false;

    CStringToken Token(strdata, L"@");

    if (Token.hasMoreTokens() == 0)
        return false;

    this->BannerSeq = _wtoi(Token.nextToken().c_str());

    wcsncpy(this->BannerName, Token.nextToken().c_str(), BANNER_LENGTH_NAME - 1);
    this->BannerName[BANNER_LENGTH_NAME - 1] = L'\0';

    wcsncpy(this->BannerImageURL, Token.nextToken().c_str(), DL_MAX_URL_LENGTH - 1);
    this->BannerImageURL[DL_MAX_URL_LENGTH - 1] = L'\0';

    this->BannerOrder = _wtoi(Token.nextToken().c_str());
    this->BannerDirection = _wtoi(Token.nextToken().c_str());

    CStringMethod::ConvertStringToDateTime(this->BannerStartDate, Token.nextToken());
    CStringMethod::ConvertStringToDateTime(this->BannerEndDate, Token.nextToken());

    wcsncpy(this->BannerLinkURL, Token.nextToken().c_str(), DL_MAX_URL_LENGTH - 1);
    this->BannerLinkURL[DL_MAX_URL_LENGTH - 1] = L'\0';

    std::wstring url = this->BannerImageURL;
    std::size_t pos = url.rfind(L"/", std::wstring::npos);

    if (pos != std::wstring::npos)
    {
        std::wstring sub = url.substr(pos + 1, url.length() - pos - 1);

        swprintf(this->BannerImagePath, MAX_PATH, L"%ls%ls", strDirPath.c_str(), sub.c_str());

        std::error_code ec;
        bool fileExists = std::filesystem::exists(std::filesystem::path(this->BannerImagePath), ec);

        if (bDonwLoad || !fileExists)
        {
            // Download using libcurl instead of URLDownloadToFile
            char narrowUrl[DL_MAX_URL_LENGTH] = {0};
            char narrowPath[MAX_PATH * 4] = {0};
            wcstombs(narrowUrl, this->BannerImageURL, sizeof(narrowUrl) - 1);
            wcstombs(narrowPath, this->BannerImagePath, sizeof(narrowPath) - 1);

            CURL* curl = curl_easy_init();
            if (curl)
            {
                std::ofstream outFile(narrowPath, std::ios::binary);
                if (outFile.is_open())
                {
                    curl_easy_setopt(curl, CURLOPT_URL, narrowUrl);
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, BannerWriteCallback);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);
                    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
                    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
                    curl_easy_perform(curl);
                    outFile.close();
                }
                curl_easy_cleanup(curl);
            }
        }
    }

    return true;
}
#endif

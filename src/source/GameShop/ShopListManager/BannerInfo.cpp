//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: BannerInfo.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "BannerInfo.h"
#include "StringToken.h"
#include "StringMethod.h"

#include <atomic>
#include <filesystem>
#include <iterator>

#include "GameShop/FileDownloader/CurlFileDownloader.h"

CBannerInfo::CBannerInfo() // OK
{
    memset(this->BannerName, 0, sizeof(this->BannerName));
    memset(this->BannerImageURL, 0, sizeof(this->BannerImageURL));
    memset(this->BannerImagePath, 0, sizeof(this->BannerImagePath));
    memset(this->BannerLinkURL, 0, sizeof(this->BannerLinkURL));
}
CBannerInfo::~CBannerInfo() // OK
{
}

bool CBannerInfo::SetBanner(std::wstring strdata, std::wstring strDirPath, bool bDonwLoad) // OK
{
    if (strdata.empty())
        return 0;

    CStringToken Token(strdata, L"@");

    if (Token.hasMoreTokens() == 0)
        return 0;

    this->BannerSeq = _wtoi(Token.nextToken().c_str());

    StringCchCopy(this->BannerName, std::size(this->BannerName), Token.nextToken().c_str());

    StringCchCopy(this->BannerImageURL, std::size(this->BannerImageURL), Token.nextToken().c_str());

    this->BannerOrder = _wtoi(Token.nextToken().c_str());
    this->BannerDirection = _wtoi(Token.nextToken().c_str());

    CStringMethod::ConvertStringToDateTime(this->BannerStartDate, Token.nextToken());
    CStringMethod::ConvertStringToDateTime(this->BannerEndDate, Token.nextToken());

    StringCchCopy(this->BannerLinkURL, std::size(this->BannerLinkURL), Token.nextToken().c_str());

    std::wstring url = this->BannerImageURL;
    std::size_t pos = url.rfind(L"/", std::wstring::npos);

    if (pos != std::wstring::npos)
    {
        std::wstring sub = url.substr(pos + 1, url.length() - pos - 1);

        StringCchPrintf(this->BannerImagePath, std::size(this->BannerImagePath), L"%ls%ls", strDirPath.c_str(), sub.c_str());

        std::error_code error;
        if (bDonwLoad || !std::filesystem::exists(std::filesystem::path(this->BannerImagePath), error))
        {
            const std::atomic_bool cancel = false;
            CurlFileDownloader::DownloadFile(this->BannerImageURL, this->BannerImagePath, L"", L"", true, &cancel);
        }
    }

    return 1;
}
#endif

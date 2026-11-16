// Downloads one shop catalog, script, or banner file. Each target compiles one adapter from Adapters/: libcurl, or
// one that reports downloads as unavailable (DL_UNAVAILABLE).
#pragma once

#include <atomic>
#include <string>

#include "GameShop/ShopListManager/interface/WZResult/WZResult.h"

class ShopDownload
{
public:
    // Downloads url to localPath, creating parent directories as needed.
    // username/password are used for FTP authentication (ignored when empty).
    // passiveFtp selects passive vs. active FTP transfer mode. pBreak, when not
    // null, is polled during the transfer and aborts it once it becomes true.
    // Returns a WZResult using the same DL_/WZ_ codes as the WinINet
    // downloader so callers handle both paths identically.
    static WZResult DownloadFile(const std::wstring& url, const std::wstring& localPath, const std::wstring& username,
                                 const std::wstring& password, bool passiveFtp, const std::atomic_bool* pBreak);
};

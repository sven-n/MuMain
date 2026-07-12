// libcurl-backed shop file downloader (issue #462).
//
// The in-game shop's catalog/script downloader is a WinINet + urlmon stack on
// Windows (FileDownloader/FTPConnecter/HTTPConnecter). Those have no portable
// equivalent, so off Windows this class downloads a single remote file (FTP or
// HTTP) to a local path with libcurl instead. It is compiled only on
// non-Windows; the WinINet path stays in use on Windows unchanged.
#pragma once

#include <string>

#include "GameShop/ShopListManager/interface/WZResult/WZResult.h"

class CurlFileDownloader
{
public:
    // Downloads url to localPath, creating parent directories as needed.
    // username/password are used for FTP authentication (ignored when empty).
    // passiveFtp selects passive vs. active FTP transfer mode. pBreak, when not
    // null, is polled during the transfer and aborts it once it becomes
    // non-zero. Returns a WZResult using the same DL_/WZ_ codes as the WinINet
    // downloader so callers handle both paths identically.
    static WZResult DownloadFile(const std::wstring& url,
        const std::wstring& localPath,
        const std::wstring& username,
        const std::wstring& password,
        bool passiveFtp,
        const volatile int* pBreak);
};

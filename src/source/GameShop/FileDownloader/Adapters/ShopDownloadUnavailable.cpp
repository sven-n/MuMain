// For a target without shop downloads: every download fails with DL_UNAVAILABLE and writes nothing.
#include "GameShop/FileDownloader/ShopDownload.h"

WZResult ShopDownload::DownloadFile(const std::wstring& url, const std::wstring&, const std::wstring&,
                                    const std::wstring&, bool, const std::atomic_bool*)
{
    WZResult result;
    result.SetResult(DL_UNAVAILABLE, 0, L"[ShopDownload] Shop downloads are not available on this platform: %ls",
                     url.c_str());
    return result;
}

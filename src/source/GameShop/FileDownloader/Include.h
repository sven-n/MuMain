/*******************************************************************************
 *	Include Header — FileDownloader implementation layer
 *	Migrated from WinINet to libcurl (Story 7.6.6)
 *******************************************************************************/

#pragma once

#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <curl/curl.h>

#include "GameShop/ShopListManager/interface/WZResult/WZResult.h"
#include "GameShop/ShopListManager/interface/DownloadInfo.h"
#include "GameShop/ShopListManager/interface/IDownloaderStateEvent.h"

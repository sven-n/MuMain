/*******************************************************************************
*	�� �� �� : ������
*	�� �� �� : 2009.06.10
*	��    �� : Include Header
*******************************************************************************/

#pragma once

#pragma warning(disable : 4995)

#include <iostream>
#include "Core/Platform/WinInet.h"

#include "Core/Platform/CrtDbg.h"
#ifdef _WIN32
#include <tchar.h>
#endif
#include "Core/Platform/StrSafe.h"

#include "GameShop\ShopListManager\interface\WZResult/WZResult.h"
#include "GameShop\ShopListManager\interface\DownloadInfo.h"
#include "GameShop\ShopListManager\interface\IDownloaderStateEvent.h"

#pragma comment(lib, L"Wininet.lib")

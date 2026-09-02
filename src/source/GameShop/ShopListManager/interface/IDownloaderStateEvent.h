/*******************************************************************************
 *	Interface - Download State Event
 *	Portable types (Story 7.6.6)
 *******************************************************************************/

#pragma once

#include <cstdint>

class IDownloaderStateEvent
{
public:
    IDownloaderStateEvent() {};
    virtual ~IDownloaderStateEvent() {};

    virtual void OnStartedDownloadFile(wchar_t* szFileName, uint64_t uFileLength) = 0;
    virtual void OnProgressDownloadFile(wchar_t* szFileName, uint64_t uDownloadFileLength) = 0;
    virtual void OnCompletedDownloadFile(wchar_t* szFileName, WZResult wzResult) = 0;
};

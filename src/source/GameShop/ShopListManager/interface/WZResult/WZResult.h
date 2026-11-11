/*******************************************************************************
 *	WZResult - Processing result object
 *	Portable types (Story 7.6.6)
 *******************************************************************************/

#pragma once

#include "GameShop/ShopListManager/interface/WZResult/ErrorCodeDefine.h"
#include <cstdint>

#define MAX_ERROR_MESSAGE 1024

class WZResult
{
public:
    WZResult();
    ~WZResult();

    WZResult& operator=(const WZResult& result);
    bool IsSuccess();
    wchar_t* GetErrorMessage();
    uint32_t GetErrorCode();
    uint32_t GetWindowErrorCode();

    void SetSuccessResult();
    void SetResult(uint32_t dwErrorCode, uint32_t dwWindowErrorCode, const wchar_t* szFormat, ...);

    static WZResult BuildSuccessResult();
    static WZResult BuildResult(uint32_t dwErrorCode, uint32_t dwWindowErrorCode, const wchar_t* szFormat, ...);

private:
    uint32_t m_dwErrorCode;
    uint32_t m_dwWindowErrorCode;
    wchar_t m_szErrorMessage[MAX_ERROR_MESSAGE];
};

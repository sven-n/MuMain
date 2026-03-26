//************************************************************************
//
// FILE: WZResult.cpp
// Migrated to portable types (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "WZResult.h"

#include <cstdarg>
#include <cstring>
#include <cwchar>

WZResult::WZResult()
{
    this->SetSuccessResult();
}

WZResult::~WZResult() {}

bool WZResult::IsSuccess()
{
    return !this->m_dwErrorCode && !this->m_dwWindowErrorCode;
}

uint32_t WZResult::GetErrorCode()
{
    return this->m_dwErrorCode;
}

wchar_t* WZResult::GetErrorMessage()
{
    return this->m_szErrorMessage;
}

uint32_t WZResult::GetWindowErrorCode()
{
    return this->m_dwWindowErrorCode;
}

WZResult& WZResult::operator=(const WZResult& a2)
{
    this->m_dwErrorCode = a2.m_dwErrorCode;
    this->m_dwWindowErrorCode = a2.m_dwWindowErrorCode;
    wcsncpy(this->m_szErrorMessage, a2.m_szErrorMessage, MAX_ERROR_MESSAGE - 1);
    this->m_szErrorMessage[MAX_ERROR_MESSAGE - 1] = L'\0';
    return *this;
}

void WZResult::SetResult(uint32_t dwErrorCode, uint32_t dwWindowErrorCode, const wchar_t* szFormat, ...)
{
    va_list va;

    va_start(va, szFormat);
    this->m_dwErrorCode = dwErrorCode;
    this->m_dwWindowErrorCode = dwWindowErrorCode;
    vswprintf(this->m_szErrorMessage, MAX_ERROR_MESSAGE, szFormat, va);
    va_end(va);
}

void WZResult::SetSuccessResult()
{
    this->m_dwErrorCode = WZ_SUCCESS;
    this->m_dwWindowErrorCode = 0;
    wcsncpy(this->m_szErrorMessage, L"Success", MAX_ERROR_MESSAGE - 1);
    this->m_szErrorMessage[MAX_ERROR_MESSAGE - 1] = L'\0';
}

WZResult WZResult::BuildSuccessResult()
{
    WZResult result;

    result.SetSuccessResult();

    return result;
}

WZResult WZResult::BuildResult(uint32_t dwErrorCode, uint32_t dwWindowErrorCode, const wchar_t* szFormat, ...)
{
    WZResult result;
    wchar_t Buffer[MAX_ERROR_MESSAGE];
    va_list args;

    va_start(args, szFormat);

    memset(Buffer, 0, sizeof(Buffer));
    vswprintf(Buffer, MAX_ERROR_MESSAGE, szFormat, args);

    va_end(args);
    result.SetResult(dwErrorCode, dwWindowErrorCode, Buffer);

    return result;
}

#endif

//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: WZResult.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "WZResult.h"
#include <crtdbg.h>
#include <strsafe.h>

WZResult::WZResult() // OK
{
    this->SetSuccessResult();
}

WZResult::~WZResult() // OK
{
}

BOOL WZResult::IsSuccess() // OK
{
    return !this->m_dwErrorCode && !this->m_dwWindowErrorCode;
}

DWORD  WZResult::GetErrorCode() // OK
{
    return this->m_dwErrorCode;
}

wchar_t* WZResult::GetErrorMessage() // OK
{
    return this->m_szErrorMessage;
}

DWORD  WZResult::GetWindowErrorCode() // OK
{
    return this->m_dwWindowErrorCode;
}

WZResult& WZResult::operator=(const WZResult& a2) // OK
{
    this->m_dwErrorCode = a2.m_dwErrorCode;
    this->m_dwWindowErrorCode = a2.m_dwWindowErrorCode;
    StringCchCopy(this->m_szErrorMessage, sizeof(this->m_szErrorMessage), a2.m_szErrorMessage);
    return *this;
}

void WZResult::SetResult(DWORD dwErrorCode, DWORD dwWindowErrorCode, TCHAR* szFormat, ...) // OK
{
    va_list va;

    va_start(va, szFormat);
    this->m_dwErrorCode = dwErrorCode;
    this->m_dwWindowErrorCode = dwWindowErrorCode;
    StringCchVPrintf(this->m_szErrorMessage, sizeof(this->m_szErrorMessage), szFormat, va);
}

void WZResult::SetSuccessResult() // OK
{
    this->m_dwErrorCode = WZ_SUCCESS;
    this->m_dwWindowErrorCode = ERROR_SUCCESS;
    StringCchCopy(this->m_szErrorMessage, sizeof(this->m_szErrorMessage), L"Success");
}

WZResult WZResult::BuildSuccessResult() // OK
{
    WZResult result;

    result.SetSuccessResult();

    return result;
}

WZResult WZResult::BuildResult(DWORD dwErrorCode, DWORD dwWindowErrorCode, TCHAR* szFormat, ...) // OK
{
    WZResult result;
    wchar_t Buffer[MAX_ERROR_MESSAGE];
    va_list args;

    va_start(args, szFormat);

    memset(Buffer, 0, sizeof(Buffer));
    StringCchVPrintf(Buffer, sizeof(Buffer), szFormat, args);

    result.SetResult(dwErrorCode, dwWindowErrorCode, Buffer);

    return result;
}

#endif
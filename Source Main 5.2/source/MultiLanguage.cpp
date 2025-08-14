// MultiLanguage.cpp: implementation of the CMultiLanguage class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"


CMultiLanguage* CMultiLanguage::ms_Singleton = NULL;

CMultiLanguage::CMultiLanguage(std::wstring strSelectedML)
{
    ms_Singleton = this;

    if (wcsicmp(strSelectedML.c_str(), L"ENG") == 0)
    {
        byLanguage = 0;
    }
    else if (wcsicmp(strSelectedML.c_str(), L"POR") == 0)
    {
        byLanguage = 1;
    }
    else if (wcsicmp(strSelectedML.c_str(), L"SPN") == 0)
    {
        byLanguage = 2;
    }
    else
    {
        byLanguage = 0;
    }
}

BYTE CMultiLanguage::GetLanguage()
{
    return byLanguage;
}

int32_t CMultiLanguage::ConvertFromUtf8(wchar_t* target, char* source, int maxSourceLength)
{
    int count = MultiByteToWideChar(CP_UTF8, 0, source, maxSourceLength, NULL, 0);
    if (maxSourceLength > 0)
    {
        count = min(count, maxSourceLength);
    }

    MultiByteToWideChar(CP_UTF8, 0, source, -1, target, count);
    return count;
}

int32_t CMultiLanguage::ConvertToUtf8(char* target, wchar_t* source, int maxSourceLength)
{
    auto count = WideCharToMultiByte(0, 0, source, maxSourceLength, 0, 0, 0, 0);
    if (maxSourceLength > 0)
    {
        count = min(count, maxSourceLength);
    }

    WideCharToMultiByte(0, 0, source, -1, target, count, 0, 0);
    return count;
}


WPARAM CMultiLanguage::ConvertFulltoHalfWidthChar(DWORD wParam)
{
    auto Char = (wchar_t)(wParam);

    if (Char >= 0xFF01 && Char <= 0xFF5A)
        wParam -= 0xFEE0;
    else if (Char == 0x3000)
        wParam = 0x0020;

    return wParam;
}
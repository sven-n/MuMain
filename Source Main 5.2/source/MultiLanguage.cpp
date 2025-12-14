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
    if (target == nullptr || source == nullptr)
    {
        return 0;
    }

    const int sourceLength = (maxSourceLength < 0) ? -1 : maxSourceLength;
    int requiredChars = MultiByteToWideChar(CP_UTF8, 0, source, sourceLength, nullptr, 0);
    if (requiredChars <= 0)
    {
        target[0] = L'\0';
        return 0;
    }

    if (maxSourceLength > 0)
    {
        requiredChars = std::min(requiredChars, maxSourceLength - 1);
    }

    const int written = MultiByteToWideChar(CP_UTF8, 0, source, sourceLength, target, requiredChars + 1);
    target[written] = L'\0';
    return written;
}

int32_t CMultiLanguage::ConvertToUtf8(char* target, wchar_t* source, int maxSourceLength)
{
    if (target == nullptr || source == nullptr)
    {
        return 0;
    }

    const int sourceLength = (maxSourceLength < 0) ? -1 : maxSourceLength;
    int requiredBytes = WideCharToMultiByte(CP_UTF8, 0, source, sourceLength, nullptr, 0, nullptr, nullptr);
    if (requiredBytes <= 0)
    {
        target[0] = '\0';
        return 0;
    }

    if (maxSourceLength > 0)
    {
        requiredBytes = std::min(requiredBytes, maxSourceLength - 1);
    }

    const int written = WideCharToMultiByte(CP_UTF8, 0, source, sourceLength, target, requiredBytes + 1, nullptr, nullptr);
    target[written] = '\0';
    return written;
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
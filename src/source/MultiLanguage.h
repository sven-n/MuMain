// MultiLanguage.h: interface for the CMultiLanguage class.
//////////////////////////////////////////////////////////////////////

#pragma once

class CMultiLanguage
{
private:
    static CMultiLanguage* ms_Singleton;

    BYTE byLanguage;

    CMultiLanguage() { ms_Singleton = this; };

public:
    CMultiLanguage(std::wstring strSelectedML);
    ~CMultiLanguage() { ms_Singleton = 0; };

    BYTE GetLanguage();				// Getters
    
    WPARAM ConvertFulltoHalfWidthChar(DWORD wParam);

    static int32_t ConvertFromUtf8(wchar_t* target, char* source, int maxSourceLength = -1);
    static int32_t ConvertToUtf8(char* target, wchar_t* source, int maxSourceLength = -1);

    static CMultiLanguage* GetSingletonPtr() { return ms_Singleton; };
};

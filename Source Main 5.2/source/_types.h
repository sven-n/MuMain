#pragma once

namespace unicode
{
    typedef wchar_t			t_char;
    typedef std::wstring		t_string;
    inline wchar_t* _strcpy(wchar_t* strDestination, const wchar_t* strSource)
    {
        return wcscpy(strDestination, strSource);
    }
    inline wchar_t* _strncpy(wchar_t* strDest, const wchar_t* strSource, size_t count)
    {
        return wcsncpy(strDest, strSource, count);
    }
    inline wchar_t* _strtok(wchar_t* strToken, const wchar_t* strDelimit)
    {
        return _wcstok(strToken, strDelimit);
    }
    inline size_t  __cdecl _strlen(const wchar_t* szText)
    {
        return wcslen(szText);
    }
    inline int _strcmp(const wchar_t* string1, const wchar_t* string2)
    {
        return wcscmp(string1, string2);
    }
    inline wchar_t* _strstr(const wchar_t* string, const wchar_t* strCharSet)
    {
        return const_cast<wchar_t*>(wcsstr(string, strCharSet));
    }

    inline int _sprintf(wchar_t* buffer, const wchar_t* format, ...)
    {
        va_list	pArguments;
        va_start(pArguments, format);
        vswprintf(buffer, format, pArguments);
        va_end(pArguments);

        return wcslen(buffer);
    }

    inline BOOL _GetTextExtentPoint(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize)
    {
        return GetTextExtentPointA(hdc, lpString, cbString, lpSize);
    }
    inline BOOL _GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize)
    {
        return GetTextExtentPoint32A(hdc, lpString, cbString, lpSize);
    }
    inline BOOL _TextOut(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cbString)
    {
        return TextOutA(hdc, nXStart, nYStart, lpString, cbString);
    }
}

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec34_t[3][4];

template< typename T > inline
void InitVector(T* vect, int size) {
    for (int i = 0; i < size; ++i) {
        vect[i] = 0.0f;
    }
}

template< typename T > inline
void IdentityMatrix(T(*mat)[4]) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            mat[i][j] = 0;
        }
    }
}

template< typename T > inline
void IdentityVector2D(T* vect) {
    InitVector(vect, 2);
}

template< typename T > inline
void IdentityVector3D(T* vect) {
    InitVector(vect, 3);
}

typedef char* PCHAR;
typedef char			CHAR;
typedef std::wstring		STRING;

typedef wchar_t* PWCHAR;
typedef wchar_t			WCHAR;
typedef std::wstring	WSTRING;
typedef std::map<eBuffState, DWORD>    BuffStateMap;

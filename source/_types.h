#pragma once

#ifdef KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE
#include <string>
#include <list>
#include <vector>
#include <map>

#include "_enum.h"
#endif // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE

namespace unicode
{
#ifdef MU_UNICODE
	typedef wchar_t			t_char;
	typedef std::wstring	t_string;
	inline wchar_t* _strcpy(wchar_t* strDestination, const wchar_t* strSource)
	{ return wcscpy(strDestination, strSource); }
	inline wchar_t* _strncpy(wchar_t* strDest, const wchar_t* strSource, size_t count)
	{ return wcsncpy(strDest, strSource, count); }
	inline wchar_t* _strtok(wchar_t* strToken, const wchar_t* strDelimit)
	{ return wcstok(strToken, strDelimit); }
	inline size_t  __cdecl _strlen(const wchar_t* szText)
	{ return wcslen(szText); }
	inline int _strcmp(const wchar_t *string1, const wchar_t *string2)
	{ return wcscmp(string1, string2); }
	inline wchar_t* _strstr(const wchar_t* string, const wchar_t* strCharSet)
	{ return wcsstr(string, strCharSet); }

	inline int _sprintf(wchar_t* buffer, const wchar_t* format, ...)
	{
		va_list	pArguments;
		va_start(pArguments, format);
		vswprintf(buffer, format, pArguments);
		va_end(pArguments);	

		return wcslen(buffer);
	}
	

#else // MU_UNICODE
	typedef char			t_char;
	typedef std::string		t_string;
	inline char* _strcpy(char* strDestination, const char* strSource)
	{ return strcpy(strDestination, strSource); }
	inline char* _strncpy(char* strDest, const char* strSource, size_t count)
	{ return strncpy(strDest, strSource, count); }
	inline char* _strtok(char* strToken, const char* strDelimit)
	{ return strtok(strToken, strDelimit); }
	inline size_t  __cdecl _strlen(const char* szText)
	{ return strlen(szText); }
	inline int _strcmp(const char *string1, const char *string2)
	{ return strcmp(string1, string2); }
	inline char* _strstr(const char* string, const char* strCharSet)
#ifdef _VS2008PORTING
	{ return const_cast<char*>(strstr(string, strCharSet)); }
#else // _VS2008PORTING
	{ return strstr(string, strCharSet); }
#endif // _VS2008PORTING

	inline int _sprintf(char* buffer, const char* format, ...)
	{ 	
		va_list	pArguments;
		va_start(pArguments, format);
		vsprintf(buffer, format, pArguments);
		va_end(pArguments);

		return strlen(buffer);
	}
	
#endif // MU_UNICODE

//////////////////////////////////////////////////////////////////////////
// WIN32 API

#ifdef MU_UNICODE
	inline BOOL _GetTextExtentPoint(HDC hdc, LPCWSTR lpString, int cbString, LPSIZE lpSize)
	{ return GetTextExtentPointW(hdc, lpString, cbString, lpSize); }
	inline BOOL _GetTextExtentPoint32(HDC hdc, LPCWSTR lpString, int cbString, LPSIZE lpSize)
	{ return GetTextExtentPoint32W(hdc, lpString, cbString, lpSize); }
	inline BOOL _TextOut(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cbString)
	{ return TextOutW(hdc, nXStart, nYStart, lpString, cbString); }
#else // MU_UNICODE
	inline BOOL _GetTextExtentPoint(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize)
	{ return GetTextExtentPointA(hdc, lpString, cbString, lpSize); }
	inline BOOL _GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize)
	{ return GetTextExtentPoint32A(hdc, lpString, cbString, lpSize); }
	inline BOOL _TextOut(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cbString)
	{ return TextOutA(hdc, nXStart, nYStart, lpString, cbString); }
#endif // MU_UNICODE
}

#ifdef DOUBLEVEC_T
	typedef double vec_t;
#else //DOUBLEVEC_T
	typedef float vec_t;
#endif //DOUBLEVEC_T

typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];	
typedef vec_t vec4_t[4];	
typedef vec_t vec34_t[3][4];

template< typename T > inline
void InitVector( T* vect, int size ) {
	for( int i = 0; i < size; ++i ) {
#ifdef DOUBLEVEC_T
		vect[i] = 0; }
#else //DOUBLEVEC_T
		vect[i] = 0.0f; }
#endif //DOUBLEVEC_T
}

template< typename T > inline
void IdentityMatrix( T (*mat)[4] ) {
	for( int i = 0; i < 3; ++i ) {
		for( int j = 0; j < 4; ++j ) {
#ifdef DOUBLEVEC_T
			mat[i][j] = 0; } }
#else //DOUBLEVEC_T
			mat[i][j] = 0.0f; } }
#endif //DOUBLEVEC_T
}

template< typename T > inline
void IdentityVector2D( T* vect ) {
	InitVector( vect, 2 ); }

template< typename T > inline
void IdentityVector3D( T* vect ) {
	InitVector( vect, 3 ); }

#ifdef KWAK_ADD_TYPE_TEMPLATE_INBOUNDS
//===================================================================
// X가 범위 안에 있는지 판별해준다.
//===================================================================
template< class T > inline BOOL InBounds	( const T X, const T Min, const T Max )	{	return X<Min ? FALSE : X>Max ? FALSE : TRUE;		}
#endif // KWAK_ADD_TYPE_TEMPLATE_INBOUNDS

typedef char*			PCHAR;
typedef char			CHAR;
typedef std::string		STRING;

typedef wchar_t*		PWCHAR;
typedef wchar_t			WCHAR;
typedef std::wstring	WSTRING;
#ifdef KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE
	typedef std::map<eBuffState, DWORD>    BuffStateMap;
#else // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE
	typedef map<eBuffState, DWORD>    BuffStateMap;
#endif // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE


#pragma once

namespace unicode
{
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
	{ return const_cast<char*>(strstr(string, strCharSet)); }

	inline int _sprintf(char* buffer, const char* format, ...)
	{ 	
		va_list	pArguments;
		va_start(pArguments, format);
		vsprintf(buffer, format, pArguments);
		va_end(pArguments);

		return strlen(buffer);
	}

	inline BOOL _GetTextExtentPoint(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize)
	{ return GetTextExtentPointA(hdc, lpString, cbString, lpSize); }
	inline BOOL _GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize)
	{ return GetTextExtentPoint32A(hdc, lpString, cbString, lpSize); }
	inline BOOL _TextOut(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cbString)
	{ return TextOutA(hdc, nXStart, nYStart, lpString, cbString); }
}

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];	
typedef vec_t vec4_t[4];	
typedef vec_t vec34_t[3][4];

template< typename T > inline
void InitVector( T* vect, int size ) {
	for( int i = 0; i < size; ++i ) {
		vect[i] = 0.0f; }
}

template< typename T > inline
void IdentityMatrix( T (*mat)[4] ) {
	for( int i = 0; i < 3; ++i ) {
		for( int j = 0; j < 4; ++j ) {
			mat[i][j] = 0; } }
}

template< typename T > inline
void IdentityVector2D( T* vect ) {
	InitVector( vect, 2 ); }

template< typename T > inline
void IdentityVector3D( T* vect ) {
	InitVector( vect, 3 ); }

typedef char*			PCHAR;
typedef char			CHAR;
typedef std::string		STRING;

typedef wchar_t*		PWCHAR;
typedef wchar_t			WCHAR;
typedef std::wstring	WSTRING;
typedef std::map<eBuffState, DWORD>    BuffStateMap;


